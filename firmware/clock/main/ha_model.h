#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef enum {HA_UNKNOWN,HA_OFF,HA_ON,HA_UNAVAILABLE} ha_light_state_t;
typedef struct {ha_light_state_t state;char name[64];} ha_light_t;
bool ha_entity_valid(const char *entity);
bool ha_endpoint_valid(const char *endpoint);
bool ha_token_valid(const char *token);
bool ha_parse_light(const char *json,size_t length,const char *entity,ha_light_t *out);
