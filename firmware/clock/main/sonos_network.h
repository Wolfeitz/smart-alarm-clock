#pragma once
#include "sonos_client.h"
#include <stdint.h>
/* Caller owns both objects through the synchronous operation. Allocate the large
 * client in external RAM, never on the network task's stack. No UI calls. */
typedef struct {int64_t deadline;bool (*allowed)(void *);void *context;} sonos_network_t;
void sonos_network_begin(sonos_client_t *client,sonos_network_t *network,
    bool (*allowed)(void *),void *context);
