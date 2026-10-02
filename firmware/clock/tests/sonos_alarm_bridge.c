/* Deterministic clock/preferences and unavailable HA boundary. The alarm output,
 * remote coordinator, backend router and Sonos adapter are production modules. */
#include "media_ha.h"
#include "media_service.h"
#include <string.h>
#include <stdint.h>
static media_snapshot_t selection;
static int64_t microseconds;
int64_t esp_timer_get_time(void){return microseconds;}
void test_alarm_tick(uint32_t ms){microseconds=(int64_t)ms*1000;}
void test_alarm_setup(const char *target,const char *content,const char *type)
{memset(&selection,0,sizeof(selection));selection.configured=true;selection.remote_alarm=true;strcpy(selection.entity,target);strcpy(selection.content,content);strcpy(selection.content_type,type);}
void media_service_snapshot(media_snapshot_t *out){*out=selection;}
void diagnostics_printf(const char *format,...){(void)format;}
void media_ha_config(media_backend_config_t *out){memset(out,0,sizeof(*out));}
bool media_ha_identity_valid(const char *s){(void)s;return false;}
bool media_ha_target_valid(const char *s){(void)s;return false;}
int media_ha_read(const char *target,media_player_t *out){(void)target;(void)out;return MEDIA_BACKEND_ERROR;}
int media_ha_action(const char *target,media_action_t action,const media_player_t *current,const char *content,const char *type)
{(void)target;(void)action;(void)current;(void)content;(void)type;return MEDIA_BACKEND_ERROR;}
