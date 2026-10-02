#!/usr/bin/env python3
"""Production C controller/XML over actual localhost HTTP; no Sonos/HA required."""
from pathlib import Path
import ctypes as C
import http.server
import threading
import tempfile
import subprocess
import urllib.request
import urllib.error
import xml.etree.ElementTree as ET
import time

ROOT=Path(__file__).resolve().parents[1]
SOAP='http://schemas.xmlsoap.org/soap/envelope/'
UUID='RINCON_00000000000101400'
class State:
    uuid=UUID; grouped=False; volume=35; muted=0; transport='STOPPED'; uri=''
    fault=''; malformed=''; oversized=''; late=False; requests=[]
state=State()
class Handler(http.server.BaseHTTPRequestHandler):
    def log_message(self,*args): pass
    def reply(self,body,status=200):
        data=body.encode();self.send_response(status);self.send_header('Content-Type','text/xml');self.send_header('Content-Length',str(len(data)));self.end_headers()
        try:self.wfile.write(data)
        except (BrokenPipeError,ConnectionResetError):pass
    def do_GET(self):
        assert self.path=='/xml/device_description.xml'
        self.reply(f'<root><device><deviceType>urn:schemas-upnp-org:device:ZonePlayer:1</deviceType><manufacturer>Sonos, Inc.</manufacturer><roomName>Simulator</roomName><UDN>uuid:{state.uuid}</UDN></device></root>')
    def do_POST(self):
        wire=self.rfile.read(int(self.headers['Content-Length']));envelope=ET.fromstring(wire)
        node=list(envelope.find('{'+SOAP+'}Body'))[0];service,action=self.headers['SOAPAction'].strip('"').split('#')
        assert node.tag=='{'+service+'}'+action
        expected='/ZoneGroupTopology/Control' if 'ZoneGroupTopology' in service else '/MediaRenderer/'+service.split(':')[-2]+'/Control'
        assert self.path==expected
        args={x.tag:x.text or '' for x in node};state.requests.append((action,args))
        if action!='GetZoneGroupState':assert args['InstanceID']=='0'
        if action==state.fault:
            self.reply('<s:Envelope xmlns:s="'+SOAP+'"><s:Body><s:Fault><faultcode>s:Client</faultcode></s:Fault></s:Body></s:Envelope>',500);return
        values={}
        if action=='GetZoneGroupState':
            members=f'<ZoneGroupMember UUID="{UUID}"/>'
            if state.grouped:members+='<ZoneGroupMember UUID="RINCON_OTHER"/>'
            values={'ZoneGroupState':f'<ZoneGroups><ZoneGroup Coordinator="{UUID}">{members}</ZoneGroup></ZoneGroups>'}
        elif action=='GetTransportInfo':values={'CurrentTransportState':state.transport}
        elif action=='GetVolume':assert args['Channel']=='Master';values={'CurrentVolume':str(state.volume)}
        elif action=='GetMute':assert args['Channel']=='Master';values={'CurrentMute':str(state.muted)}
        elif action=='GetMediaInfo':values={'CurrentURI':state.uri}
        elif action=='SetVolume':state.volume=int(args['DesiredVolume']);assert 0<=state.volume<=100
        elif action=='SetAVTransportURI':state.uri=args['CurrentURI']
        elif action=='Play':
            assert args['Speed']=='1'
            if state.late:time.sleep(.2)
            state.transport='PLAYING'
        elif action=='Pause':state.transport='PAUSED_PLAYBACK'
        elif action=='Stop':state.transport='STOPPED'
        elif action in ('Next','Previous'):pass
        else:raise AssertionError(action)
        root=ET.Element('{'+SOAP+'}Envelope');body=ET.SubElement(root,'{'+SOAP+'}Body');response=ET.SubElement(body,'{'+service+'}'+action+'Response')
        for k,v in values.items():ET.SubElement(response,k).text=v
        xml=ET.tostring(root,encoding='unicode')
        if action==state.malformed:xml=xml[:-10]
        if action==state.oversized:xml='x'*40000
        self.reply(xml)

HTTP=C.CFUNCTYPE(C.c_int,C.c_void_p,C.c_char_p,C.c_char_p,C.c_char_p,C.c_char_p,C.c_void_p,C.c_size_t,C.POINTER(C.c_size_t))
ALLOWED=C.CFUNCTYPE(C.c_bool,C.c_void_p)
class Client(C.Structure):
    _fields_=[('http',HTTP),('context',C.c_void_p),('allowed',ALLOWED),('allowed_context',C.c_void_p),('response',C.c_char*32769),('body',C.c_char*24576),('response_size',C.c_size_t)]
class Device(C.Structure):
    _fields_=[('endpoint',C.c_char*48),('uuid',C.c_char*48),('name',C.c_char*64)]
class Player(C.Structure):
    _fields_=[('state',C.c_int),('name',C.c_char*64),('title',C.c_char*96),('artist',C.c_char*64),('content_id',C.c_char*384),('capabilities',C.c_uint32),('volume',C.c_double),('volume_known',C.c_bool),('relative_volume',C.c_bool),('muted_known',C.c_bool),('muted',C.c_bool)]
class NoRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self,*args):return None
opener=urllib.request.build_opener(urllib.request.ProxyHandler({}),NoRedirect())
permit=True;cancel_on_uri=False
@ALLOWED
def allowed(ctx):return permit
@HTTP
def transport_http(ctx,endpoint,path,action,body,dest,capacity,size):
    global permit
    request=urllib.request.Request('http://'+endpoint.decode()+path.decode(),data=body,headers={'Content-Type':'text/xml; charset="utf-8"',**({'SOAPAction':'"'+action.decode()+'"'} if action else {})})
    try:
        with opener.open(request,timeout=.08 if state.late else 1) as response:
            data=response.read(capacity)
            if len(data)>=capacity:return -1
            C.memmove(dest,data,len(data));size[0]=len(data)
            if cancel_on_uri and action and action.endswith(b'#SetAVTransportURI'):permit=False
            return response.status
    except (OSError,urllib.error.URLError):return -1

with tempfile.TemporaryDirectory(prefix='esp-sonos-') as temp:
    library=str(Path(temp)/'sonos.so')
    subprocess.run(['cc','-shared','-fPIC','-std=c11','-Wall','-Wextra','-Werror','-I'+str(ROOT/'firmware/clock/main'),str(ROOT/'firmware/clock/main/sonos_xml.c'),str(ROOT/'firmware/clock/main/sonos_client.c'),'-lexpat','-o',library],check=True)
    lib=C.CDLL(library)
    lib.sonos_probe.argtypes=[C.POINTER(Client),C.c_char_p,C.POINTER(Device)]
    lib.sonos_read.argtypes=[C.POINTER(Client),C.POINTER(Device),C.POINTER(Player)]
    lib.sonos_action.argtypes=[C.POINTER(Client),C.POINTER(Device),C.c_int,C.POINTER(Player),C.c_char_p,C.c_char_p]
    lib.sonos_stop.argtypes=[C.POINTER(Client),C.POINTER(Device)]
    lib.sonos_target_format.argtypes=[C.POINTER(Device),C.c_void_p];lib.sonos_target_format.restype=C.c_bool
    lib.sonos_target_parse.argtypes=[C.c_char_p,C.POINTER(Device)];lib.sonos_target_parse.restype=C.c_bool
    server=http.server.ThreadingHTTPServer(('127.0.0.1',0),Handler)
    thread=threading.Thread(target=server.serve_forever,daemon=True);thread.start()
    try:
        client=Client(http=transport_http,allowed=allowed);device=Device();player=Player()
        endpoint=f'127.0.0.1:{server.server_port}'.encode()
        assert lib.sonos_probe(C.byref(client),endpoint,C.byref(device))==0 and device.uuid==UUID.encode()
        target=C.create_string_buffer(96);assert lib.sonos_target_format(C.byref(device),target)
        copy=Device();assert lib.sonos_target_parse(target.value,C.byref(copy)) and copy.uuid==device.uuid
        assert not lib.sonos_target_parse(b'sonos:127.0.0.1:1400/bad',C.byref(copy))
        assert lib.sonos_read(C.byref(client),C.byref(device),C.byref(player))==0 and player.volume==.35 and player.muted_known and not player.muted
        uri=b'https://example.test/music?a=1&b=2'
        assert lib.sonos_action(C.byref(client),C.byref(device),6,C.byref(player),uri,b'<DIDL-Lite/>')==0
        assert state.uri==uri.decode() and state.transport=='PLAYING'
        assert lib.sonos_read(C.byref(client),C.byref(device),C.byref(player))==0 and player.content_id==uri and player.state==4
        assert lib.sonos_action(C.byref(client),C.byref(device),2,C.byref(player),b'',b'')==0 and state.transport=='PAUSED_PLAYBACK'
        assert lib.sonos_action(C.byref(client),C.byref(device),5,C.byref(player),b'',b'')==0 and state.volume==40
        assert lib.sonos_stop(C.byref(client),C.byref(device))==0 and state.transport=='STOPPED'
        for attribute,expected in [('grouped',3),('uuid',2)]:
            old=getattr(state,attribute);setattr(state,attribute,True if attribute=='grouped' else 'RINCON_CHANGED')
            before=len([x for x in state.requests if x[0]=='Play'])
            assert lib.sonos_action(C.byref(client),C.byref(device),1,C.byref(player),b'',b'')==expected
            assert len([x for x in state.requests if x[0]=='Play'])==before;setattr(state,attribute,old)
        for attribute in ('fault','malformed','oversized'):
            setattr(state,attribute,'GetVolume');assert lib.sonos_read(C.byref(client),C.byref(device),C.byref(player))!=0;setattr(state,attribute,'')
        cancel_on_uri=True
        assert lib.sonos_action(C.byref(client),C.byref(device),6,C.byref(player),uri,b'')==4 and state.transport=='STOPPED'
        cancel_on_uri=False;permit=True;state.late=True
        assert lib.sonos_action(C.byref(client),C.byref(device),1,C.byref(player),b'',b'')!=0
        time.sleep(.25);assert state.transport=='PLAYING';state.late=False
        assert lib.sonos_stop(C.byref(client),C.byref(device))==0 and state.transport=='STOPPED'
        print('PASS production C Sonos over HTTP: identity, transport, volume, escaping, group refusal, malformed/Fault/oversize, cancellation and late-playback stop')
    finally:server.shutdown();server.server_close();thread.join()
