#include "json_memory.h"
#include "cJSON.h"
#include "esp_heap_caps.h"
#include <stddef.h>
/* Network/setup JSON is transient; preserve internal memory for alarm/UI tasks.
 * Small-board/no-PSRAM fallback retains existing cJSON allocation behavior. */
static void *json_allocate(size_t size)
{
    void *memory=heap_caps_malloc(size,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    if(!memory)memory=heap_caps_malloc(size,MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT);
    return memory;
}
void json_memory_init(void)
{
    cJSON_Hooks hooks={.malloc_fn=json_allocate,.free_fn=heap_caps_free};
    cJSON_InitHooks(&hooks);
}
