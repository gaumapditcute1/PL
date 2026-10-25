#pragma once

#include <Arduino.h>

extern long gmtOffset;

void sync_real_time(void);
bool time_sync(char *buffer, size_t max_len);
uint32_t calibrate_internal_rtc(void);
void time_drift(uint64_t time_sleep);


