#pragma once
#include "media_model.h"
typedef struct {char entity[96],status[96],content[384],content_type[48];media_player_t player;bool configured,fresh,busy,remote_alarm,save_failed;uint32_t saved_ticket;} media_snapshot_t;
void media_service_init(void);
void media_service_disable(void);
void media_service_poll(bool online);
void media_service_snapshot(media_snapshot_t *out);
bool media_service_configure(const char *entity);
bool media_service_action(media_action_t action);
bool media_service_refresh(void);

bool media_service_configure_tagged(const char *entity,uint32_t tag);

bool media_service_select(const char *entity,const char *content,const char *type);

bool media_service_select_alarm(const char *entity,const char *content,const char *type,bool remote);

uint32_t media_service_select_alarm_tracked(const char *entity,const char *content,const char *type,bool remote);
