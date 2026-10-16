"""Exercise session HTTP contracts through the running API, including isolation."""
import json
import urllib.error
import urllib.request
import sys
base = sys.argv[1] if len(sys.argv) > 1 else 'http://127.0.0.1:5080'
def request(path, method='GET', body=None, token=None):
    headers = {'Content-Type': 'application/json'}
    if token: headers['X-Robin-Session'] = token
    req = urllib.request.Request(base + '/api/v1/' + path, method=method,
        data=json.dumps(body).encode() if body is not None else None, headers=headers)
    try: response = urllib.request.urlopen(req, timeout=5)
    except urllib.error.HTTPError as error: response = error
    with response:
        raw = response.read()
        return response.status, json.loads(raw) if raw else None
code, a = request('sessions', 'POST', {'reducedMotion': True})
assert code == 201 and a['state']['mode'] == 'awake'
code, b = request('sessions', 'POST', {'reducedMotion': False})
assert code == 201 and a['token'] != b['token']
try:
    assert request('session')[0] == 401
    assert request('session', token='0' * 64)[0] == 404
    code, state = request('session/events', 'POST', {'sequence': 1, 'event': 'headStarted'}, a['token'])
    assert code == 200 and state['headContact'] and state['expression'] == 'happy'
    assert request('session', token=b['token'])[1]['headContact'] is False
    assert request('session/events', 'POST', {'sequence': 1, 'event': 'headStarted'}, a['token'])[1]['revision'] == state['revision']
    assert request('session/events', 'POST', {'sequence': 1, 'event': 'restart'}, a['token'])[0] == 409
    assert request('session/events', 'POST', {'sequence': 3, 'event': 'restart'}, a['token'])[0] == 409
    assert request('session/events', 'POST', {'sequence': 2, 'event': 'invented'}, a['token'])[0] == 400
    assert request('session/events', 'POST', {'sequence': 2, 'event': 'headEnded'}, a['token'])[1]['headContact'] is False
    assert request('session/events', 'POST', {'sequence': 3, 'event': 'heartTouched'}, a['token'])[1]['mood'] == 'happy'
    assert request('session/events', 'POST', {'sequence': 4, 'event': 'restart'}, a['token'])[1]['mode'] == 'awake'
finally:
    for session in (a, b): assert request('session', 'DELETE', token=session['token'])[0] == 204
assert request('session', token=a['token'])[0] == 404
print('HTTP session creation, isolation, validation, replay protection and deletion passed.')
