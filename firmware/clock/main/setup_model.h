#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef struct {uint32_t tag;char endpoint[192],token[512],entity[96];} setup_request_t;
bool setup_parse(const char *json,size_t size,setup_request_t *out);
