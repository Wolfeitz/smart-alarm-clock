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
    fault=''; malformed=''; oversized=''; late=False; requests=[]; favorite_radio=False; bad_favorite=False; seek=0
state=State()
class Handler(http.server.BaseHTTPRequestHandler):
    def log_message(self,*args): pass
    def reply(self,body,status=200):
        data=body.encode();self.send_response(status);self.send_header('Content-Type','text/xml');self.send_header('Content-Length',str(len(data)));self.end_headers()
        try:self.wfile.write(data)
        except (BrokenPipeError,ConnectionResetError):pass
    def do_GET(self):
        state.requests.append(('GET',{}))
        assert self.path=='/xml/device_description.xml'
        self.reply(f'<root><device><deviceType>urn:schemas-upnp-org:device:ZonePlayer:1</deviceType><manufacturer>Sonos, Inc.</manufacturer><roomName>Simulator</roomName><UDN>uuid:{state.uuid}</UDN></device></root>')
    def do_POST(self):
        wire=self.rfile.read(int(self.headers['Content-Length']));envelope=ET.fromstring(wire)
        node=list(envelope.find('{'+SOAP+'}Body'))[0];service,action=self.headers['SOAPAction'].strip('"').split('#')
        assert node.tag=='{'+service+'}'+action
        expected='/ZoneGroupTopology/Control' if 'ZoneGroupTopology' in service else ('/MediaServer/' if 'ContentDirectory' in service else '/MediaRenderer/')+service.split(':')[-2]+'/Control'
        assert self.path==expected
        args={x.tag:x.text or '' for x in node};state.requests.append((action,args))
        if action not in ('GetZoneGroupState','Browse'):assert args['InstanceID']=='0'
        if action==state.fault:
            self.reply('<s:Envelope xmlns:s="'+SOAP+'"><s:Body><s:Fault><faultcode>s:Client</faultcode></s:Fault></s:Body></s:Envelope>',500);return
        values={}
        if action=='GetZoneGroupState':
            members=f'<ZoneGroupMember UUID="{UUID}"/>'
            if state.grouped:members+='<ZoneGroupMember UUID="RINCON_OTHER"/>'
            values={'ZoneGroupState':f'<ZoneGroups><ZoneGroup Coordinator="{UUID}">{members}</ZoneGroup></ZoneGroups>'}
        elif action=='Browse':
            assert args['Filter']=='*' and args['SortCriteria']==''
            didl=ET.Element('DIDL-Lite',{'xmlns':'urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/','xmlns:dc':'http://purl.org/dc/elements/1.1/','xmlns:r':'urn:schemas-rinconnetworks-com:metadata-1-0/'})
            metadata=args['BrowseFlag']=='BrowseMetadata'
            assert args['ObjectID']==('FV:2/1' if metadata else 'FV:2')
            assert args['RequestedCount']==('1' if metadata else '6')
            count=0 if int(args['StartingIndex'])>0 else (1 if metadata else 2)
            for index in range(count):
                item=ET.SubElement(didl,'item',{'id':f'FV:2/{index+1}'})
                ET.SubElement(item,'dc:title').text=['Morning & music','Radio'][index]
                if metadata:
                    ET.SubElement(item,'res',{'protocolInfo':'x-rincon-playlist:*:audio/x-sonos-playlist:*'}).text='x-sonosapi-stream:station?sid=1' if state.favorite_radio else 'x-rincon-cpcontainer:playlist&token=test'
                    if not state.bad_favorite:
                        ET.SubElement(item,'r:resMD').text='<DIDL-Lite xmlns="urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/"><item id="playlist"><desc id="cdudn">SA_RINCON_TEST</desc></item></DIDL-Lite>'
            values={'Result':ET.tostring(didl,encoding='unicode'),'NumberReturned':str(count),'TotalMatches':str(1 if metadata else 2)}
        elif action=='AddURIToQueue':
            assert args['DesiredFirstTrackNumberEnqueued']=='0' and args['EnqueueAsNext']=='0'
            md=ET.fromstring(args['EnqueuedURIMetaData']);ns='{urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/}'
            assert md.find(ns+'item/'+ns+'res').text==args['EnqueuedURI']
            assert md.find(ns+'item/'+ns+'desc').text=='SA_RINCON_TEST'
            values={'FirstTrackNumberEnqueued':'5'}
        elif action=='Seek':
            assert args['Unit']=='TRACK_NR' and args['Target']=='5';state.seek=5
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
class Favorite(C.Structure):
    _fields_=[('id',C.c_char*96),('title',C.c_char*96)]
class Favorites(C.Structure):
    _fields_=[('items',Favorite*6),('count',C.c_uint),('total',C.c_uint),('start',C.c_uint)]
class NoRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self,*args):return None
opener=urllib.request.build_opener(urllib.request.ProxyHandler({}),NoRedirect())
permit=True;cancel_on_uri=False;cancel_alarm=None
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
            if cancel_alarm and action and action.endswith(b'#SetAVTransportURI'):cancel_alarm()
            return response.status
    except (OSError,urllib.error.URLError):return -1

def main():
    global permit,cancel_on_uri,cancel_alarm
    with tempfile.TemporaryDirectory(prefix='esp-sonos-') as temp:
        library=str(Path(temp)/'sonos.so')
        # Host HTTP boundary only; production media adapter/controller/XML remain intact.
        bridge=Path(temp)/'network_bridge.c'
        bridge.write_text("""#include "sonos_network.h"
    static sonos_http_t transport;
    void test_transport(sonos_http_t http){transport=http;}
    void sonos_network_begin(sonos_client_t *c,sonos_network_t *n,bool (*allowed)(void *),void *context)
    {(void)n;c->http=transport;c->context=0;c->allowed=allowed;c->allowed_context=context;}
    """)
        subprocess.run(['cc','-shared','-fPIC','-std=c11','-Wall','-Wextra','-Werror','-I'+str(ROOT/'firmware/clock/main'),'-I'+str(ROOT/'firmware/clock/tests/ha_stubs'),
            *[str(ROOT/'firmware/clock/main'/f'{m}.c') for m in ('sonos_xml','sonos_client','media_sonos','media_model','media_backend','remote_alarm','alarm_output')],
            str(bridge),str(ROOT/'firmware/clock/tests/sonos_alarm_bridge.c'),'-lexpat','-o',library],check=True)
        lib=C.CDLL(library)
        lib.test_alarm_tick.argtypes=[C.c_uint32]
        lib.test_alarm_setup.argtypes=[C.c_char_p,C.c_char_p,C.c_char_p]
        lib.alarm_output_enable.argtypes=[C.c_bool]
        lib.alarm_output_local.argtypes=[C.c_uint8,C.c_uint32];lib.alarm_output_local.restype=C.c_bool
        lib.remote_alarm_poll.argtypes=[C.c_bool]
        lib.test_transport.argtypes=[HTTP];lib.test_transport(transport_http)
        lib.media_sonos_read.argtypes=[C.c_char_p,C.POINTER(Player)]
        lib.media_sonos_action.argtypes=[C.c_char_p,C.c_int,C.POINTER(Player),C.c_char_p,C.c_char_p]
        lib.media_sonos_action_guarded.argtypes=[C.c_char_p,C.c_int,C.POINTER(Player),C.c_char_p,C.c_char_p,ALLOWED,C.c_void_p]
        lib.media_sonos_stop.argtypes=[C.c_char_p]
        lib.sonos_probe.argtypes=[C.POINTER(Client),C.c_char_p,C.POINTER(Device)]
        lib.sonos_read.argtypes=[C.POINTER(Client),C.POINTER(Device),C.POINTER(Player)]
        lib.sonos_action.argtypes=[C.POINTER(Client),C.POINTER(Device),C.c_int,C.POINTER(Player),C.c_char_p,C.c_char_p]
        lib.sonos_favorites.argtypes=[C.POINTER(Client),C.POINTER(Device),C.c_uint,C.POINTER(Favorites)]
        lib.sonos_play_favorite.argtypes=[C.POINTER(Client),C.POINTER(Device),C.c_char_p]
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
            assert lib.media_sonos_read(target.value,C.byref(player))==0
            assert lib.media_sonos_action(target.value,6,C.byref(player),uri,b'playlist')!=0
            assert state.transport=='STOPPED'
            assert lib.media_sonos_action(target.value,6,C.byref(player),uri,b'uri')==0
            assert lib.media_sonos_read(target.value,C.byref(player))==0 and player.state==4 and player.content_id==uri
            assert lib.media_sonos_stop(target.value)==0 and state.transport=='STOPPED'
            cancel_on_uri=True
            assert lib.media_sonos_action_guarded(target.value,6,C.byref(player),uri,b'uri',allowed,None)!=0
            assert state.transport=='STOPPED'
            cancel_on_uri=False;permit=True
            state.uuid='RINCON_CHANGED'
            assert lib.media_sonos_read(target.value,C.byref(player))==3  # Backend NOT_FOUND
            state.uuid=UUID
            page=Favorites();assert lib.sonos_favorites(C.byref(client),C.byref(device),0,C.byref(page))==0
            assert page.count==2 and page.total==2 and page.items[0].title==b'Morning & music'
            assert lib.sonos_favorites(C.byref(client),C.byref(device),6,C.byref(page))==0 and page.count==0
            assert lib.media_sonos_action(target.value,6,C.byref(player),b'FV:2/1',b'sonos-favorite')==0
            assert state.seek==5 and state.transport=='PLAYING' and state.uri==f'x-rincon-queue:{UUID}#0'
            assert lib.media_sonos_stop(target.value)==0
            state.favorite_radio=True;before=len(state.requests)
            assert lib.sonos_play_favorite(C.byref(client),C.byref(device),b'FV:2/1')==0
            assert state.uri.startswith('x-sonosapi-stream:') and not any(x[0]=='AddURIToQueue' for x in state.requests[before:])
            assert lib.media_sonos_stop(target.value)==0
            state.bad_favorite=True;assert lib.sonos_play_favorite(C.byref(client),C.byref(device),b'FV:2/1')!=0 and state.transport=='STOPPED'
            state.bad_favorite=False;state.favorite_radio=False;cancel_on_uri=True
            assert lib.sonos_play_favorite(C.byref(client),C.byref(device),b'FV:2/1')==4 and state.transport=='STOPPED'
            cancel_on_uri=False;permit=True
            lib.alarm_output_enable(True)
            def start_alarm(ms,content=uri,kind=b'uri'):
                lib.test_alarm_setup(target.value,content,kind);lib.test_alarm_tick(ms)
                assert not lib.alarm_output_local(1,ms)
                lib.remote_alarm_poll(True)
            def stop_alarm(ms):
                lib.test_alarm_tick(ms);lib.alarm_output_local(0,ms);lib.remote_alarm_poll(True)
                assert state.transport=='STOPPED'
            start_alarm(100000);assert state.transport=='PLAYING'
            assert not lib.alarm_output_local(1,100001)  # Exact URI proof leases remote output.
            state.uri='different selection';lib.test_alarm_tick(102000);lib.remote_alarm_poll(True)
            assert lib.alarm_output_local(1,110000);stop_alarm(110000)
            start_alarm(120000,b'FV:2/1',b'sonos-favorite');assert state.transport=='PLAYING'
            assert lib.alarm_output_local(1,128000)  # Queue URI cannot prove the favorite.
            stop_alarm(128000)
            cancel_alarm=lambda:lib.alarm_output_local(0,140000)
            start_alarm(140000);assert state.transport=='STOPPED'
            cancel_alarm=None;lib.remote_alarm_poll(True)
            state.late=True;start_alarm(150000);time.sleep(.25);assert state.transport=='PLAYING'
            assert lib.alarm_output_local(1,158000);state.late=False;stop_alarm(158000)
            print('PASS production remote alarm through router/Sonos HTTP: exact selection lease, wrong-selection fallback, conservative favorite fallback, cancellation and timed-out late playback cleanup')
            print('PASS favorites: paged Browse, metadata/resource preservation, queue append and seek, radio direct URI, malformed selection and cancellation')
            print('PASS production Sonos media adapter: explicit URI, selection type rejection, readback, stop, in-operation cancellation and UUID refusal')
            print('PASS production C Sonos over HTTP: identity, transport, volume, escaping, group refusal, malformed/Fault/oversize, cancellation and late-playback stop')
        finally:server.shutdown();server.server_close();thread.join()

if __name__=="__main__":main()
