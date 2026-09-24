#pragma once
#include <stddef.h>
typedef void *QueueHandle_t;
QueueHandle_t xQueueCreate(unsigned count,size_t size);
int xQueueSend(QueueHandle_t q,const void *value,unsigned timeout);
int xQueueReceive(QueueHandle_t q,void *value,unsigned timeout);
