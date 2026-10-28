#pragma once
#include <Arduino.h>

extern long gmtOffset;

void load_timezone_from_flash(void);
bool wait_for_ntp_sync(uint8_t timeout_sec);
bool time_sync(char *buffer, size_t max_len);
uint32_t calibrate_internal_rtc(void);
void time_drift(uint64_t time_sleep);