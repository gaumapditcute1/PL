#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <WebServer.h>
#include "web.h"
#include "oled.h"
#include "wifi_lib.h"
#include "real_time.h"

Preferences pre;
WebServer server(80);
bool wifi_config_save = false;

uint8_t timeout = 15;
const char* AP_SSID = "test";
const char* AP_PASS = "123456789";
String STA_SSID = "";
String STA_PASS = "";

void save_wifi(String ssid, String pass, long tz_sec){
    pre.begin("wifi", false);
    pre.putString("ssid", ssid);
    pre.putString("pass", pass);
    pre.putLong("tz", tz_sec); 
    pre.end();
    gmtOffset = tz_sec;
}

void wifi_ap(void){
    wifi_config_save = false;

    WiFi.disconnect(true);
    WiFi.setAutoReconnect(false);
    delay(100);

    WiFi.mode(WIFI_AP_STA);
    WiFi.setTxPower(WIFI_POWER_8_5dBm);

    WiFi.softAP(AP_SSID, AP_PASS, 1, 0, 4);

    display.clearDisplay();
    display.setTextColor(1);
    display.setFont(NULL);
    display.setTextSize(1);

    display.setCursor(9, 11);
    display.printf("ssid: %s", AP_SSID);
    display.setCursor(9, 26);
    display.printf("pass: %s", AP_PASS);
    display.setCursor(9, 41);
    display.print("Web: ");
    display.print(WiFi.softAPIP());
    
    display.display();
    server.on("/", HTTP_GET, [](){
        server.send_P(200, "text/html", index_html);
    });
    
    server.on("/api/scan", HTTP_GET, [](){
        int n = WiFi.scanNetworks();

        String json = "[";
        for (int i = 0; i < n; i++){
            if (i > 0) json += ",";
            json += "{";
            json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
            json += "\"rssi\":" + String(WiFi.RSSI(i));
            json += "}";
        }
        json += "]";
        WiFi.scanDelete();
        server.send(200, "application/json", json);
    });

    server.on("/api/save", HTTP_POST, []() {
        String ssid = server.arg("ssid");
        String pass = server.arg("pass");
        

        long tz_sec = 7 * 3600;
        if (server.hasArg("tz")) {
            tz_sec = (long)(server.arg("tz").toFloat() * 3600);
        }

        save_wifi(ssid, pass, tz_sec);
        server.send(200, "text/plain", "Đã lưu WiFi & Múi giờ | Đang kết nối...");
        delay(500);
        server.stop();
        WiFi.softAPdisconnect(true);
        delay(200);
        wifi_config_save = true;
    });
    server.begin();
}

bool wifi_sta(uint8_t timeout_sec){
    pre.begin("wifi", true);
    STA_SSID = pre.getString("ssid", "");
    STA_PASS = pre.getString("pass", "");
    gmtOffset = pre.getLong("tz", 7 * 3600);
    pre.end();

    if (STA_SSID.length() == 0) return false;

    WiFi.mode(WIFI_STA);
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
    WiFi.begin(STA_SSID.c_str(), STA_PASS.c_str());

    unsigned long start_time = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start_time < (unsigned long)timeout_sec * 1000)) {
        delay(100);
    }

    if(WiFi.status() == WL_CONNECTED){
        return true;
    } else {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        return false;
    }
}