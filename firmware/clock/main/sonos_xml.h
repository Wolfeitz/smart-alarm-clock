#pragma once
#include <stdbool.h>
#include <stddef.h>
typedef struct {const char *path,*attribute;char *value;size_t capacity;bool found;} sonos_xml_field_t;
/* Strict complete XML, bounded depth/size, no DTD/entity declarations. */
bool sonos_xml_fields(const char *xml,size_t size,sonos_xml_field_t *fields,unsigned count);
bool sonos_xml_escape(const char *text,char *out,size_t capacity);
bool sonos_xml_standalone(const char *xml,size_t size,const char *uuid);

/* Select one repeated DIDL item while preserving duplicate-field rejection. */
bool sonos_xml_item_fields(const char *xml,size_t size,unsigned index,sonos_xml_field_t *fields,unsigned count,unsigned *items);
bool sonos_xml_add_resource(const char *xml,const char *uri,const char *protocol,char *out,size_t capacity);
