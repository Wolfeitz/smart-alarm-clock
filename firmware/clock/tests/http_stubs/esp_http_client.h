#pragma once
#include <stdbool.h>
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_TIMEOUT -2
#define HTTP_EVENT_ON_DATA 1
#define HTTP_METHOD_POST 1
typedef struct {void *user_data;void *client;int event_id,data_len;void *data;} esp_http_client_event_t;
typedef struct {const char *url;void *crt_bundle_attach;int timeout_ms;esp_err_t (*event_handler)(esp_http_client_event_t *);void *user_data;bool disable_auto_redirect;int buffer_size;} esp_http_client_config_t;
typedef void *esp_http_client_handle_t;
esp_http_client_handle_t esp_http_client_init(const esp_http_client_config_t *config);
int esp_http_client_set_header(void *,const char *,const char *);
int esp_http_client_set_method(void *,int);
int esp_http_client_set_post_field(void *,const char *,int);
int esp_http_client_perform(void *);
int esp_http_client_get_status_code(void *);
int esp_http_client_cleanup(void *);

int esp_http_client_close(void *);
