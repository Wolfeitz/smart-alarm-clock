#!/usr/bin/env python3
"""Read-only Sonos setup acceptance against a temporary LAN simulator. No media save/play."""
import argparse,importlib.util,os,re,threading,time
from pathlib import Path
import serial
parser=argparse.ArgumentParser();parser.add_argument('--bind',required=True);parser.add_argument('--port',required=True);parser.add_argument('--http-port',type=int,default=0);args=parser.parse_args()
root=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('sonos_fixture',root/'scripts/test-sonos-protocol.py');fixture=importlib.util.module_from_spec(spec);spec.loader.exec_module(fixture)
server=fixture.http.server.ThreadingHTTPServer((args.bind,args.http_port),fixture.Handler)
thread=threading.Thread(target=server.serve_forever,daemon=True);thread.start()
s=serial.Serial();s.port=args.port;s.baudrate=115200;s.timeout=.1;s.dtr=False;s.rts=False
logs=[]
def read(seconds):
    end=time.monotonic()+seconds;raw=b''
    while time.monotonic()<end:raw+=s.read(8192)
    text=raw.decode(errors='replace');logs.append(text)
    assert not any(v in text for v in ('Guru Meditation','Stack canary','assert failed','CORRUPT HEAP'))
    return text
def command(text,seconds=1):s.write((text+'\n').encode());return read(seconds)
def lookup(expected):
    text=command(f'SONOS_LOOKUP {args.bind}:{server.server_port}')
    assert 'SONOS_LOOKUP accepted=1' in text,text
    end=time.monotonic()+35
    while time.monotonic()<end:
        text=command('SONOS_STATE',2)
        rows=re.findall(r'^SONOS_STATE .*$',text,re.M)
        if rows and 'busy=0' in rows[-1]:
            assert expected in rows[-1],rows[-1]
            print(rows[-1],flush=True);return
    raise AssertionError('lookup completion timed out')
try:
    s.open();read(30)
    before=command('STATE',2);alarms=re.findall(r'^ALARM_SLOT .*$',before,re.M);assert len(alarms)==8
    lookup('ready=1');assert any(action=='Browse' for action,_ in fixture.state.requests)
    fixture.state.grouped=True;lookup('Choose an ungrouped speaker');fixture.state.grouped=False
    fixture.state.malformed='Browse';lookup('Speaker unavailable or favorites invalid');fixture.state.malformed=''
    lookup('ready=1')
    after=command('STATE',2);assert alarms==re.findall(r'^ALARM_SLOT .*$',after,re.M)
    assert not any(action in ('Play','SetAVTransportURI','AddURIToQueue','SetVolume') for action,_ in fixture.state.requests)
    print('PASS actual ESP32 HTTP/SOAP lookup, favorites, group/malformed refusal, recovery, no playback or alarm changes',flush=True)
finally:
    print('Simulator requests: '+','.join(action for action,_ in fixture.state.requests),flush=True)
    (root/'local-config/clock/sonos-device-simulator.log').write_text(''.join(logs))
    if s.is_open:os.close(s.fileno());s.is_open=False
    server.shutdown();server.server_close();thread.join()
