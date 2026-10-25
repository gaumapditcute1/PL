#include <Arduino.h>
#include "time.h"
#include <sys/time.h>
#include "esp_sleep.h"
#include "soc/rtc.h"
#include "esp32c3/rom/rtc.h"

const char* npt_sever = "pool.ntp.org";
RTC_DATA_ATTR long gmtOffset = 7 * 3600;
RTC_DATA_ATTR uint32_t prev_cal_val = 0;

void sync_real_time(void){
    configTime(0, 0, npt_sever);
}

bool time_sync(char *buffer, size_t max_len){
    time_t now;
    time(&now);

    if (now < 1600000000) {
        return false;
    }

    now += gmtOffset;
    
    struct tm time_info;
    gmtime_r(&now, &time_info);
    strftime(buffer, max_len, "%H:%M", &time_info);
    return true;
}

uint32_t calibrate_internal_rtc(void){
    uint32_t cal_val = rtc_clk_cal(RTC_CAL_RTC_MUX, 1024);
    return cal_val;
}

void time_drift(uint64_t time_sleep){
    uint32_t current_cal_val = calibrate_internal_rtc();
    if (prev_cal_val == 0){
        prev_cal_val = current_cal_val;
        return;
    }

    int64_t diff_call = (int64_t)current_cal_val - (int64_t)prev_cal_val;
    int64_t drift_us = ((int64_t)time_sleep * 1000000LL * diff_call) / (int64_t)prev_cal_val;

    if (abs(drift_us) >= 1000){
        struct timeval delta;
        delta.tv_sec = drift_us / 1000000LL;
        delta.tv_usec = drift_us % 1000000LL;
        adjtime(&delta, NULL);
    }

    prev_cal_val = current_cal_val;
}


