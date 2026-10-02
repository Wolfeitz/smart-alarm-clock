#pragma once
/* Install once before any task can parse JSON. Never mutate hooks at runtime. */
void json_memory_init(void);
