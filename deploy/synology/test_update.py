"""Test deployment decisions against a fake Docker CLI, without touching containers."""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

SCRIPT = Path(__file__).with_name('update-web.sh').resolve()
FAKE = r'''
import json, os, sys
from pathlib import Path
p = Path(os.environ['FAKE_DOCKER_STATE'])
s = json.loads(p.read_text())
a = sys.argv[1:]
s['calls'].append(a)
def finish(code=0, output=''):
    if s.get('signal_after') == cmd and not s.get('signal_sent'):
        s['signal_sent'] = True
        p.write_text(json.dumps(s))
        os.kill(os.getppid(), 15)
    p.write_text(json.dumps(s))
    if output: print(output)
    sys.exit(code)
cmd = a[0]
if cmd == 'inspect':
    name = a[-1]
    if name not in s['containers']: finish(1)
    c = s['containers'][name]
    if '--format' not in a: finish()
    fmt = a[a.index('--format') + 1]
    if 'Labels' in fmt: finish(output='true' if c['managed'] else '<no value>')
    if 'Health' in fmt: finish(output=c['health'])
    if '.State.Running' in fmt: finish(output='true' if c['running'] else 'false')
    if '.Image' in fmt: finish(output=c['image'])
if cmd == 'pull': finish(1 if s.get('fail_pull') else 0)
if cmd == 'image': finish(output=s['target'])
if cmd == 'run':
    name = a[a.index('--name')+1]
    if name == 'pinkrobin-web' and s.get('fail_start'): finish(1)
    health = 'unhealthy' if s.get('fail_candidate') and name.endswith('candidate') else 'healthy'
    if name == 'pinkrobin-web' and s.get('fail_health'): health='unhealthy'
    s['containers'][name] = {'image':a[-1], 'health':health, 'managed':'--label' in a, 'running':True}
    finish(output='container-id')
if cmd == 'exec': finish()
if cmd == 'stop': s['containers'][a[1]]['running']=False; finish()
if cmd == 'start':
    if s.get('fail_rollback'): finish(1)
    s['containers'][a[1]]['running']=True; finish()
if cmd == 'rename': s['containers'][a[2]]=s['containers'].pop(a[1]); finish()
if cmd == 'rm':
    for name in a[1:]:
        if not name.startswith('-'): s['containers'].pop(name, None)
    finish()
finish(2, 'Unexpected Docker invocation')
'''


class DeploymentTests(unittest.TestCase):
    def run_case(self, preexisting_lock=False, **changes):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            state = {
                'target': 'new-image', 'calls': [],
                'containers': {'pinkrobin-web': {
                    'image': 'old-image', 'managed': True,
                    'health': 'healthy', 'running': True,
                }},
            }
            state.update(changes)
            file = root / 'state.json'
            file.write_text(json.dumps(state))
            docker = root / 'docker'
            docker.write_text(f'#!{sys.executable}\n' + FAKE)
            docker.chmod(0o755)
            env = {**os.environ, 'PATH': f"{root}:{os.environ['PATH']}",
                   'FAKE_DOCKER_STATE': str(file), 'PINKROBIN_LOCK': str(root / 'lock')}
            if preexisting_lock: (root / 'lock').mkdir()
            result = subprocess.run(['sh', str(SCRIPT)], env=env, capture_output=True, text=True)
            self.assertEqual((root / 'lock').exists(), preexisting_lock)
            state = json.loads(file.read_text())
            self.assertNotIn('pinkrobin-web-candidate', state['containers'])
            return result, state

    def test_success_keeps_previous(self):
        result, state = self.run_case()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(state['containers']['pinkrobin-web']['image'], 'new-image')
        self.assertEqual(state['containers']['pinkrobin-web-previous']['image'], 'old-image')

    def test_same_image_does_not_restart(self):
        result, state = self.run_case(target='old-image')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertFalse(any(call[0] == 'stop' for call in state['calls']))

    def test_candidate_or_pull_failure_preserves_running_site(self):
        for failure in ['fail_candidate', 'fail_pull']:
            with self.subTest(failure=failure):
                result, state = self.run_case(**{failure: True})
                self.assertNotEqual(result.returncode, 0)
                self.assertEqual(state['containers']['pinkrobin-web']['image'], 'old-image')
                self.assertTrue(state['containers']['pinkrobin-web']['running'])

    def test_start_and_health_failures_restore_previous(self):
        for failure in ['fail_start', 'fail_health']:
            with self.subTest(failure=failure):
                result, state = self.run_case(**{failure: True})
                self.assertNotEqual(result.returncode, 0)
                self.assertEqual(state['containers']['pinkrobin-web']['image'], 'old-image')
                self.assertTrue(state['containers']['pinkrobin-web']['running'])

    def test_legacy_container_requires_migration(self):
        legacy = {'pinkrobin-web': {'image': 'legacy', 'managed': False,
                                   'health': 'healthy', 'running': True}}
        result, state = self.run_case(containers=legacy)
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(state['containers'], legacy)
        self.assertFalse(any(call[0] == 'pull' for call in state['calls']))

    def test_interruption_during_stop_or_rename_restores_site(self):
        for operation in ['stop', 'rename']:
            with self.subTest(operation=operation):
                result, state = self.run_case(signal_after=operation)
                self.assertNotEqual(result.returncode, 0)
                self.assertEqual(state['containers']['pinkrobin-web']['image'], 'old-image')
                self.assertTrue(state['containers']['pinkrobin-web']['running'])

    def test_existing_lock_reports_failure_without_touching_docker(self):
        result, state = self.run_case(preexisting_lock=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('lock', result.stderr)
        self.assertEqual(state['calls'], [])

    def test_failed_rollback_releases_lock_and_reports_manual_recovery(self):
        result, state = self.run_case(fail_start=True, fail_rollback=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Rollback failed', result.stderr)
        self.assertEqual(state['containers']['pinkrobin-web']['image'], 'old-image')

    def test_first_install(self):
        result, state = self.run_case(containers={})
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(state['containers']['pinkrobin-web']['image'], 'new-image')


if __name__ == '__main__':
    unittest.main()
