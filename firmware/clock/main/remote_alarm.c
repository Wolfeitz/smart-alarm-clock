#include "remote_alarm.h"
#include "alarm_output.h"
#include "media_service.h"
#include "media_backend.h"
#include "esp_timer.h"
#include "diagnostics.h"
#include <string.h>
static uint32_t attempted,active;
static char target[96],content[384],type[48],identity[192];
static media_player_t last_player;
static int64_t poll_at,stop_until;
static unsigned stop_attempts;
static bool may_be_playing;
void remote_alarm_poll(bool online)
{
    uint32_t desired=alarm_output_session();int64_t now=esp_timer_get_time();
    media_backend_config_t config;media_backend_config(&config);
    if(may_be_playing&&desired!=active){
        if(!stop_until){stop_until=now+10000000;poll_at=0;}
        if(!config.configured||strcmp(identity,config.identity)||now>=stop_until||stop_attempts>=3){
            diagnostics_printf("REMOTE_ALARM stop=unconfirmed\n");may_be_playing=false;
        }else if(online&&now>=poll_at){
            stop_attempts++;int result=media_backend_action(target,MEDIA_PAUSE,&last_player,"","");
            if(result==MEDIA_BACKEND_OK){
                media_player_t observed;
                if(media_backend_read(target,&observed)==MEDIA_BACKEND_OK&&observed.state==MEDIA_PAUSED){
                    may_be_playing=false;diagnostics_printf("REMOTE_ALARM stop=paused\n");
                }
            }
            poll_at=esp_timer_get_time()+1000000;
        }
        if(may_be_playing)return;
    }
    if(!desired)return;
    if(desired!=attempted){
        attempted=desired;active=desired;stop_until=0;stop_attempts=0;poll_at=0;
        media_snapshot_t selected;media_service_snapshot(&selected);
        if(!online||!selected.remote_alarm||!selected.configured||!selected.content[0]||!config.configured)return;
        strcpy(target,selected.entity);strcpy(content,selected.content);strcpy(type,selected.content_type);strcpy(identity,config.identity);
        if(media_backend_read(target,&last_player)!=MEDIA_BACKEND_OK||
           !media_action_supported(&last_player,MEDIA_START_SAVED)||!media_action_supported(&last_player,MEDIA_PAUSE))return;
        if(alarm_output_session()!=active)return;
        /* A timeout may still have delivered the command: always attempt cleanup. */
        may_be_playing=true;
        int result=media_backend_action(target,MEDIA_START_SAVED,&last_player,content,type);
        diagnostics_printf("REMOTE_ALARM start=%s\n",result==MEDIA_BACKEND_OK?"accepted":"unconfirmed");
        if(alarm_output_session()!=active)return;
    }
    if(!may_be_playing||!online||now<poll_at||!config.configured||strcmp(identity,config.identity))return;
    media_player_t observed;
    if(media_backend_read(target,&observed)==MEDIA_BACKEND_OK){
        last_player=observed;
        if(observed.state==MEDIA_PLAYING&&observed.volume_known&&observed.volume>0&&
           observed.muted_known&&!observed.muted&&!strcmp(observed.content_id,content))
            alarm_output_confirm(active,(uint32_t)(esp_timer_get_time()/1000));
    }
    poll_at=esp_timer_get_time()+1000000;
}
