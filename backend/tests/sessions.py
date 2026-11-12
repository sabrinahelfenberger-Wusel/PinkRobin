"""Exercise session HTTP contracts through the running API, including isolation."""
import json
import urllib.error
import urllib.request
import sys
base = sys.argv[1] if len(sys.argv) > 1 else 'http://127.0.0.1:5080'
def request(path, method='GET', body=None, token=None, raw_body=None):
    headers = {'Content-Type': 'application/json'}
    if token:
        headers['X-Robin-Session'] = token

    data = raw_body
    if data is None and body is not None:
        data = json.dumps(body).encode('utf-8')

    req = urllib.request.Request(
        base + '/api/v1/' + path,
        method=method,
        data=data,
        headers=headers,
    )

    try:
        response = urllib.request.urlopen(req, timeout=5)
    except urllib.error.HTTPError as error:
        response = error

    with response:
        raw = response.read()
        content_type = response.headers.get('Content-Type', '')

        if not raw:
            result = None
        elif 'json' in content_type:
            result = json.loads(raw)
        else:
            result = raw.decode('utf-8', errors='replace')

        return response.status, result
code, a = request('sessions', 'POST', {'reducedMotion': True})
assert code == 201 and a['state']['mode'] == 'awake'
code, b = request('sessions', 'POST', {'reducedMotion': False})
assert code == 201 and a['token'] != b['token']
try:
    protected_requests = [
        ('session', 'GET', None),
        ('session/events', 'POST', {
            'sequence': 1,
            'event': 'heartTouched',
        }),
        ('session/outputs/next', 'GET', None),
        ('session/outputs/ack', 'POST', {
            'sequence': '1',
        }),
        ('session/weather', 'POST', {
            'postalCode': '9000',
        }),
    ]

    for path, method, body in protected_requests:
        for token in (None, 'invalid', 'g' * 64):
            status, _ = request(path, method, body, token)
            assert status == 401, (
                f'{method} {path}: expected 401, got {status}'
            )

        status, _ = request(path, method, body, '0' * 64)
        assert status == 404, (
            f'{method} {path}: expected 404, got {status}'
        )

    assert request('session')[0] == 401
    assert request('session', token='0' * 64)[0] == 404

    status, _ = request(
        'session/events',
        'POST',
        {
            'sequence': 1,
            'event': 'textReceived',
            'text': 'x' * 8193,
        },
        a['token'],
    )
    assert status == 413, (
        f'Oversized request: expected 413, got {status}'
    )

    status, _ = request(
        'session/events',
        'POST',
        token=a['token'],
        raw_body=b'{"sequence":',
    )
    assert status == 400, (
        f'Malformed JSON: expected 400, got {status}'
    )

    for text in ('', '   ', 'x' * 161, 'ä' * 81, '\x00'):
        status, _ = request(
            'session/events',
            'POST',
            {
                'sequence': 1,
                'event': 'textReceived',
                'text': text,
            },
            a['token'],
        )
        assert status == 400, (
            f'Invalid text: expected 400, got {status}'
        )

    assert request('session', token=a['token'])[1]['sequence'] == 0

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
for path, method, body in protected_requests:
    status, _ = request(path, method, body, a['token'])
    assert status == 404, (
        f'{method} {path}: deleted session returned {status}'
    )
print('HTTP session creation, isolation, validation, replay protection and deletion passed.')
