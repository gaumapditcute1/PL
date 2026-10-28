#include <Arduino.h>
#include "time.h"
#include <sys/time.h>
#include <Preferences.h>
#include "esp_sleep.h"
#include "esp_sntp.h"
#include "soc/rtc.h"
#include "esp32c3/rom/rtc.h"
#include "real_time.h"


RTC_DATA_ATTR long gmtOffset = 7 * 3600;
RTC_DATA_ATTR bool tz_initialized = false;
RTC_DATA_ATTR uint32_t prev_cal_val = 0;


void load_timezone_from_flash(void) {
    if (!tz_initialized) {
        Preferences pre;
        pre.begin("wifi", true);
        gmtOffset = pre.getLong("tz", 7 * 3600);
        pre.end();
        tz_initialized = true;
    }
}

bool wait_for_ntp_sync(uint8_t timeout_sec) {
    sntp_set_sync_status(SNTP_SYNC_STATUS_RESET);

    configTime(0, 0, "time.google.com", "vn.pool.ntp.org", "pool.ntp.org");

    unsigned long start = millis();
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) {
        if (millis() - start >= (unsigned long)timeout_sec * 1000UL) {
            return false; 
        }
        delay(200);
    }
    return true;
}

bool time_sync(char *buffer, size_t max_len) {
    load_timezone_from_flash();

    time_t now;
    time(&now);

    if (now < 1704067200L) {
        return false;
    }

    now += gmtOffset;

    struct tm time_info;
    gmtime_r(&now, &time_info);
    strftime(buffer, max_len, "%H:%M", &time_info);
    return true;
}

uint32_t calibrate_internal_rtc(void) {
    uint32_t cal_val = rtc_clk_cal(RTC_CAL_RTC_MUX, 1024);
    return cal_val;
}

void time_drift(uint64_t time_sleep) {
    uint32_t current_cal_val = calibrate_internal_rtc();
    if (prev_cal_val == 0) {
        prev_cal_val = current_cal_val;
        return;
    }

    int64_t diff_call = (int64_t)current_cal_val - (int64_t)prev_cal_val;
    int64_t drift_us = ((int64_t)time_sleep * 1000000LL * diff_call) / (int64_t)prev_cal_val;

    if (abs(drift_us) >= 1000) {
        struct timeval delta;
        delta.tv_sec = drift_us / 1000000LL;
        delta.tv_usec = drift_us % 1000000LL;
        adjtime(&delta, NULL);
    }

    prev_cal_val = current_cal_val;
}