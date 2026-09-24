#!/usr/bin/env python3
"""Configure the clock over USB; tokens are entered privately, never as arguments."""
import argparse
import getpass
import json
import os
import re
import secrets
import sys
import time
import urllib.request
import urllib.parse
import warnings

class SetupError(Exception):
    pass


DEFAULT_PORT = '/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_10:BD:A3:E3:52:74-if00'


def entity_valid(entity, domain):
    return len(entity) <= 95 and re.fullmatch(re.escape(domain) + r'\.[a-z0-9_]+', entity) is not None


def endpoint_valid(url):
    parsed = urllib.parse.urlsplit(url)
    return (parsed.scheme in ('http', 'https') and bool(parsed.hostname)
            and parsed.username is None and parsed.password is None and not parsed.query
            and not parsed.fragment and not parsed.path and len(url) <= 191
            and all(ord(c) > 32 and ord(c) < 127 for c in url))


def wait_for(connection, prefix, timeout=45):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        line = connection.readline().decode('ascii', errors='replace').strip()
        # Unknown device output is deliberately never printed or retained.
        if line.startswith('SETUP_ERROR '):
            raise SetupError('The clock rejected the setup message')
        if line.startswith(prefix):
            return line[len(prefix):]
    raise SetupError('Timed out waiting for the clock; saved state is unconfirmed')


def send(connection, message):
    data = (message + '\n').encode('utf-8')
    if len(data) > 1900:
        raise SetupError('Setup message exceeds the supported size')
    for offset in range(0, len(data), 32):
        chunk = data[offset:offset + 32]
        if connection.write(chunk) != len(chunk):
            raise SetupError('Incomplete USB write; saved state is unconfirmed')
        time.sleep(.02)


def save_request(connection, command, prefix, tag):
    send(connection, command)
    # A tag binds the result to this request, not a previous UI save.
    deadline = time.monotonic() + 45
    accepted = saved = False
    while time.monotonic() < deadline:
        value = wait_for(connection, f'{prefix} tag={tag} ', max(.1, deadline - time.monotonic()))
        if value == 'accepted=0' or value == 'saved=0':
            raise SetupError('Clock setup was not saved; previous settings remain')
        accepted |= value == 'accepted=1'
        saved |= value == 'saved=1'
        if accepted and saved:
            return
    raise SetupError('Clock setup acknowledgment is incomplete')


class NoRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        return None  # Never forward a credential to a redirect target.


def discover(url, token):
    request = urllib.request.Request(url + '/api/states', headers={'Authorization': 'Bearer ' + token})
    opener = urllib.request.build_opener(urllib.request.ProxyHandler({}), NoRedirect())
    with opener.open(request, timeout=8) as response:
        data = response.read(2 * 1024 * 1024 + 1)
    if len(data) > 2 * 1024 * 1024:
        raise SetupError('Entity list is too large; supply --light and/or --player explicitly')
    states = json.loads(data)
    if not isinstance(states, list):
        raise SetupError('Unexpected Home Assistant response')
    for domain in ('light', 'media_player'):
        names = sorted({s.get('entity_id', '') for s in states if isinstance(s, dict)
                        and isinstance(s.get('entity_id'), str) and entity_valid(s['entity_id'], domain)})
        print(domain + ' entities:')
        for name in names[:60]:
            print('  ' + name)
        if len(names) > 60:
            print('  (first 60 shown)')
    return input('Light entity (blank to skip): ').strip(), input('Player entity (blank to skip): ').strip()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', default=DEFAULT_PORT)
    parser.add_argument('--url', default='http://192.168.1.232:8123')
    parser.add_argument('--light', default='')
    parser.add_argument('--player', default='')
    args, unknown = parser.parse_known_args()
    if unknown:
        parser.error('Unsupported options; enter tokens only at the hidden prompt')
    if not sys.stdin.isatty() or not sys.stderr.isatty():
        parser.error('Run interactively in a terminal; input echo must be disabled')
    url = args.url.rstrip('/')
    if not endpoint_valid(url):
        parser.error('Use a root http(s) server address, without credentials or a dashboard path')
    import serial
    connection = serial.Serial()
    connection.port = args.port
    connection.baudrate = 115200
    connection.timeout = .1
    connection.write_timeout = 2
    connection.dtr = connection.rts = False
    try:
        connection.open()
        time.sleep(3)  # This USB adapter can reset the board when opened.
        connection.reset_input_buffer()
        send(connection, 'SETUP?')
        if wait_for(connection, 'SETUP_READY ', 10) != 'version=1':
            raise SetupError('Unsupported clock setup protocol')
        with warnings.catch_warnings():
            warnings.simplefilter('error', getpass.GetPassWarning)
            token = getpass.getpass('Home Assistant token (hidden): ')
        if not token or len(token) > 511 or any(ord(c) < 33 or ord(c) > 126 for c in token):
            raise SetupError('Invalid token format')
        light, player = args.light, args.player
        if not light and not player:
            light, player = discover(url, token)
        if (light and not entity_valid(light, 'light')) or (player and not entity_valid(player, 'media_player')):
            raise SetupError('Invalid light or player entity ID')
        tag = secrets.randbelow(2**32 - 1) + 1
        payload = json.dumps({'tag': tag, 'url': url, 'token': token, 'light': light}, separators=(',', ':'))
        save_request(connection, 'HA_SETUP ' + payload, 'SETUP_HA', tag)
        token = payload = ''
        print('Home Assistant connection saved on the clock.')
        if player:
            tag = secrets.randbelow(2**32 - 1) + 1
            save_request(connection, f'MEDIA_SETUP {tag} {player}', 'SETUP_MEDIA', tag)
            print('Player selection saved on the clock.')
        print('Saved configuration is confirmed; live state appears on the clock. No playback was requested.')
    finally:
        if connection.is_open:
            os.close(connection.fileno())
            connection.is_open = False


if __name__ == '__main__':
    try:
        main()
    except SetupError as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(1)
    except (Exception, KeyboardInterrupt):
        # Never render request/response exceptions that might contain secrets.
        print('Setup did not finish. Check USB access, server/token and entity selection; no secrets were logged.', file=sys.stderr)
        raise SystemExit(1)
