#include "remote_alarm.h"
#include "alarm_output.h"
#include "media_service.h"
#include "media_backend.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static int64_t now;static unsigned starts,pauses;static bool playing,mismatch,cancel_during_start,muted;
static bool selection_changed;
static bool volume_known=true;static double volume=.4;
static int start_result=MEDIA_BACKEND_OK,stop_result=MEDIA_BACKEND_OK;
int64_t esp_timer_get_time(void){return now*1000;}
void diagnostics_printf(const char *format,...){(void)format;}
void media_service_snapshot(media_snapshot_t *s)
{*s=(media_snapshot_t){.configured=true,.remote_alarm=true,.entity="speaker/bedroom",.content="stream:morning",.content_type="music"};if(selection_changed)strcpy(s->entity,"speaker/other");}
void media_backend_config_for(const char *target,media_backend_config_t *c){*c=(media_backend_config_t){.configured=true,.identity="test:local"};if(!strcmp(target,"speaker/other"))strcpy(c->identity,"test:other");}
int media_backend_read(const char *target,media_player_t *p)
{assert(!strcmp(target,"speaker/bedroom"));*p=(media_player_t){.state=playing?MEDIA_PLAYING:MEDIA_PAUSED,.volume=volume,.volume_known=volume_known,.muted_known=true,.muted=muted,.capabilities=(1u<<MEDIA_PAUSE)|(1u<<MEDIA_START_SAVED)};strcpy(p->content_id,mismatch?"stream:other":"stream:morning");return MEDIA_BACKEND_OK;}
int media_backend_action(const char *target,media_action_t action,const media_player_t *p,const char *id,const char *type)
{
    assert(!strcmp(target,"speaker/bedroom")&&media_action_supported(p,action));
    if(action==MEDIA_START_SAVED){assert(!strcmp(id,"stream:morning")&&!strcmp(type,"music"));starts++;playing=true;if(cancel_during_start)alarm_output_local(0,(uint32_t)now);return start_result;}
    assert(action==MEDIA_PAUSE);pauses++;if(stop_result==MEDIA_BACKEND_OK)playing=false;return stop_result;
}
int media_backend_action_guarded(const char *target,media_action_t action,const media_player_t *p,
    const char *id,const char *type,bool (*allowed)(void *),void *context)
{if(allowed&&!allowed(context))return MEDIA_BACKEND_ERROR;return media_backend_action(target,action,p,id,type);}
int media_backend_stop(const char *target,const media_player_t *p)
{return media_backend_action(target,MEDIA_PAUSE,p,"","");}
static void stop(void){alarm_output_local(0,(uint32_t)now);remote_alarm_poll(true);assert(!playing);now+=11000;}
int main(void)
{
    alarm_output_enable(true);assert(!alarm_output_local(1,0));remote_alarm_poll(true);assert(starts==1);
    now=1;assert(!alarm_output_local(1,1));now=3001;assert(alarm_output_local(1,3001));stop();assert(pauses==1);
    mismatch=true;assert(!alarm_output_local(1,(uint32_t)now));remote_alarm_poll(true);assert(starts==2);
    now+=8000;assert(alarm_output_local(1,(uint32_t)now));stop();mismatch=false;
    cancel_during_start=true;alarm_output_local(1,(uint32_t)now);remote_alarm_poll(true);assert(!alarm_output_session());
    remote_alarm_poll(true);assert(!playing&&pauses==3);cancel_during_start=false;now+=11000;
    alarm_output_local(1,(uint32_t)now);unsigned before=starts;remote_alarm_poll(false);
    now+=8000;assert(alarm_output_local(1,(uint32_t)now));remote_alarm_poll(true);assert(starts==before);stop();
    start_result=MEDIA_BACKEND_ERROR;alarm_output_local(1,(uint32_t)now);remote_alarm_poll(true);stop();assert(!playing);
    start_result=MEDIA_BACKEND_OK;muted=true;alarm_output_local(1,(uint32_t)now);remote_alarm_poll(true);
    now+=8000;assert(alarm_output_local(1,(uint32_t)now));stop();muted=false;
    for(unsigned i=0;i<2;i++){
        volume=i?.4:0;volume_known=!i;alarm_output_local(1,(uint32_t)now);remote_alarm_poll(true);
        now+=8000;assert(alarm_output_local(1,(uint32_t)now));stop();
    }
    volume=.4;volume_known=true;
    alarm_output_local(1,(uint32_t)now);remote_alarm_poll(true);stop_result=MEDIA_BACKEND_ERROR;
    alarm_output_local(0,(uint32_t)now);unsigned stop_before=pauses;
    for(unsigned i=0;i<5;i++){remote_alarm_poll(true);now+=1000;}
    assert(pauses==stop_before+3&&playing&&!alarm_output_session());
    stop_result=MEDIA_BACKEND_OK;playing=false;now+=11000;
    alarm_output_local(1,(uint32_t)now);remote_alarm_poll(true);assert(playing);
    selection_changed=true;stop();assert(!playing);selection_changed=false;
    puts("PASS remote worker: exact selection, local lease expiry, mismatched media, in-flight cancellation, offline no late start, ambiguous-start cleanup");
}
