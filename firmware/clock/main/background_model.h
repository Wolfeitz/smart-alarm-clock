#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define BACKGROUND_MAX_IMAGES 8
#define BACKGROUND_URL_SIZE 384
#define BACKGROUND_QUERY_SIZE 96
#define BACKGROUND_JSON_LIMIT 32768
/* Local file identifiers are resolved by the storage adapter, never as URLs. */
typedef enum {BACKGROUND_LOCAL, BACKGROUND_SELECTED, BACKGROUND_WALLHAVEN} background_source_t;
typedef struct {
    uint8_t version,categories,purity,sorting,ascending,top_range,position;
    uint8_t retries,notify_refresh,notify_error,exact_resolution;
    uint16_t retry_seconds;
    char resolution[64],ratios[48],color[7],seed[7];
} background_options_t;
background_options_t background_options_default(void);
bool background_options_valid(const background_options_t *options);
bool background_api_key_valid(const char *key);
typedef struct {
    background_source_t source;
    uint32_t interval_seconds; /* zero pins current selection */
    unsigned count;
    char images[BACKGROUND_MAX_IMAGES][BACKGROUND_URL_SIZE];
    char query[BACKGROUND_QUERY_SIZE];
    background_options_t options;
} background_config_t;
typedef struct {char id[7];char url[BACKGROUND_URL_SIZE];} background_candidate_t;
bool background_https_url(const char *url);
bool background_config_valid(const background_config_t *config);
/* Converts a Wallhaven page URL to its JSON endpoint; preserves direct URLs. */
bool background_selected_url(const char *input,char *output,size_t capacity);
bool background_search_url(const char *query,char *output,size_t capacity);
bool background_search_url_options(const background_config_t *config,char *output,size_t capacity);
/* Provider JPEG thumbnail candidates; filters follow user selection. */
unsigned background_parse_search_filtered(const char *json,size_t size,background_candidate_t *out,unsigned capacity,unsigned purity);
unsigned background_parse_search(const char *json,size_t size,background_candidate_t *out,unsigned capacity);
