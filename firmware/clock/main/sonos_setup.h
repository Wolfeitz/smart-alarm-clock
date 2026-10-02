#pragma once
#include "sonos_client.h"
typedef struct {bool busy,ready;unsigned revision;char target[96],name[64],status[96];sonos_favorites_t favorites;} sonos_setup_snapshot_t;
void sonos_setup_init(void);
void sonos_setup_disable(void);
void sonos_setup_snapshot(sonos_setup_snapshot_t *out);
bool sonos_setup_lookup(const char *address);
bool sonos_setup_page(unsigned start);
void sonos_setup_poll(bool online);
