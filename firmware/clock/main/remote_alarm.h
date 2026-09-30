#pragma once
#include <stdbool.h>
/* Existing network worker only. Local scheduling/audio never calls this. */
void remote_alarm_poll(bool online);
