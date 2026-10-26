"""HTTP contract tests against a running backend (no third-party packages)."""
import argparse
import json
import urllib.error
import urllib.request

parser = argparse.ArgumentParser()
parser.add_argument('base', nargs='?', default='http://127.0.0.1:5080')
parser.add_argument('--development', action='store_true')
parser.add_argument('--without-core', action='store_true')
args = parser.parse_args()


def request(path, origin=None, method='GET'):
    headers = {'Origin': origin} if origin else {}
    req = urllib.request.Request(args.base + path, headers=headers, method=method)
    try:
        response = urllib.request.urlopen(req, timeout=5)
    except urllib.error.HTTPError as error:
        response = error
    with response:
        return response.status, response.headers, response.read()


code, headers, body = request('/health/live')
assert code == 200 and json.loads(body)['status'] == 'alive'
assert headers['Cache-Control'] == 'no-store'
code, _, body = request('/health/ready')
if args.without_core:
    assert code == 503 and json.loads(body)['title'] == 'Core not connected'
else:
    assert code == 200 and json.loads(body)['status'] == 'ready'
code, _, body = request('/api/v1/status')
status = json.loads(body)
assert code == 200 and status['service'] == 'PinkRobin.Api'
assert status['apiVersion'] == '1.0' and status['stage'] == 'preparation'
assert status['core'] == ({'connected': False, 'implementation': 'not-connected'} if args.without_core
    else {'connected': True, 'implementation': 'native-cabi-v1'})
assert status['sessionsAvailable'] == (not args.without_core)
assert status['commandsAvailable'] == (not args.without_core)
assert status['realtimeAvailable'] is False
assert request('/api/v1/status', method='POST')[0] == 405
assert request('/api/v1/commands')[0] == 404
assert request('/hubs/robin/negotiate', method='POST')[0] == 404
_, headers, _ = request('/api/v1/status', origin='https://untrusted.example')
assert 'Access-Control-Allow-Origin' not in headers
_, headers, _ = request('/api/v1/status', origin='http://localhost:4200')
if args.development:
    assert headers['Access-Control-Allow-Origin'] == 'http://localhost:4200'
else:
    assert 'Access-Control-Allow-Origin' not in headers
print('Liveness, readiness, status, unavailable controls and CORS passed.')
