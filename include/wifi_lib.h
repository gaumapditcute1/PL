#pragma once
#include <Arduino.h>

extern WebServer server;
extern String STA_SSID;
extern String STA_PASS;
extern bool wifi_config_save;

void save_wifi(String ssid, String pass, long tz_sec);
void wifi_ap(void);
bool wifi_sta(uint8_t timeout_sec);
