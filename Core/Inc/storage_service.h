#ifndef STORAGE_SERVICE_H
#define STORAGE_SERVICE_H

#include "app_types.h"
#include <stdbool.h>

bool StorageService_Init(void);
bool StorageService_Request(const StorageRequest_t *request);
uint32_t StorageService_GetRecordCount(void);

#endif
