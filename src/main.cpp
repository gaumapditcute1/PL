#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <WebServer.h>
#include "Fonts/Org_01.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "oled.h"
#include "wifi_lib.h"
#include "real_time.h"
#include "bitmap_anim.h"

#define SW_GIF_PIN GPIO_NUM_1
#define SW_CLOCL_PIN GPIO_NUM_2
#define SW_WIFI_PIN GPIO_NUM_3

const uint64_t DRIFT_SLEEP_SEC = 600;
const unsigned long CLOCK_TIMEOUT_MS = 30000;
const unsigned long AP_TIMEOUT_MS = 120000;

enum SystemState {
    STATE_RTC,
    STATE_CLOCK,
    STATE_GIF,
    STATE_WIFI_SYNC,
    STATE_AP_CONFIG
};

SystemState currentState = STATE_RTC;
unsigned long stateStartTime = 0;
unsigned long lastClockUpdate = 0;
int currentFrame = 0;
unsigned long lastFrameTime = 0;

void setup_sleep_weakups(){
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    esp_deep_sleep_enable_gpio_wakeup(1ULL << SW_GIF_PIN | 1ULL << SW_CLOCL_PIN | 1ULL << SW_WIFI_PIN, ESP_GPIO_WAKEUP_GPIO_HIGH);
    esp_sleep_enable_timer_wakeup(DRIFT_SLEEP_SEC * 1000000ULL);
    esp_deep_sleep_start();
}

void setup(){
    Serial.begin(115200);
    
    pinMode(SW_CLOCL_PIN, INPUT_PULLDOWN);
    pinMode(SW_GIF_PIN, INPUT_PULLDOWN);
    pinMode(SW_WIFI_PIN, INPUT_PULLDOWN);

    time_drift(DRIFT_SLEEP_SEC);

    oledInit(8, 9, 400000);

    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER && gpio_get_level(SW_CLOCL_PIN) == 0 && gpio_get_level(SW_GIF_PIN) == 0 && gpio_get_level(SW_WIFI_PIN) == 0){
        setup_sleep_weakups();
        return;
    }

    if (gpio_get_level(SW_CLOCL_PIN) == HIGH){
        display.ssd1306_command(SSD1306_DISPLAYON);
        while (gpio_get_level(SW_CLOCL_PIN) == HIGH) delay(10);
        currentState = STATE_CLOCK;
        stateStartTime = millis();

    }else if(gpio_get_level(SW_GIF_PIN) == HIGH){
        display.ssd1306_command(SSD1306_DISPLAYON);
        while (gpio_get_level(SW_GIF_PIN) == HIGH) delay(10);
        currentState = STATE_GIF;
        stateStartTime = millis();

    }else if(gpio_get_level(SW_WIFI_PIN) == HIGH){
        unsigned long pressStart = millis();
        while (gpio_get_level(SW_WIFI_PIN) == HIGH) {
            if (millis() - pressStart >= 5000){
                break;
            }
            delay(50);
        }
        if (millis() - pressStart >= 5000){
            display.ssd1306_command(SSD1306_DISPLAYON);
            currentState = STATE_AP_CONFIG;
            stateStartTime = millis();
            wifi_ap();
        }else{
            currentState = STATE_WIFI_SYNC;
        }
    }else{
        setup_sleep_weakups();
    }
}

void loop(){
    unsigned long now = millis();
    if (gpio_get_level(SW_CLOCL_PIN) == HIGH){
        while (gpio_get_level(SW_CLOCL_PIN) == HIGH) delay(10);

        if (currentState == STATE_CLOCK){
            setup_sleep_weakups();
            return;
        }else {
            currentState = STATE_CLOCK;
            stateStartTime = now;
            lastClockUpdate = 0;
        }
    }

    if (gpio_get_level(SW_GIF_PIN) == HIGH){
        while (gpio_get_level(SW_GIF_PIN) == HIGH) delay(10);

        if (currentState == STATE_GIF){
            setup_sleep_weakups();
            return;
        }else{
            currentState = STATE_GIF;
            currentFrame = 0;
            stateStartTime = now;
        }
    }

    switch (currentState){
        case STATE_CLOCK: {
            if (now - stateStartTime >= CLOCK_TIMEOUT_MS){
                setup_sleep_weakups();
            }

            if (now - lastClockUpdate >= 1000){
                lastClockUpdate = now;
                char timeStr[16];
                if (!time_sync(timeStr, sizeof(timeStr))){
                    strcpy(timeStr, "--:--");
                }
                display.clearDisplay();
                display.setTextColor(1);
                display.setTextSize(5);
                display.setTextWrap(false);
                display.setFont(&Org_01);
                display.setCursor(13, 40);
                display.print(timeStr);
                display.display();
            }
            break;
        }
        case STATE_GIF: {
            if (now - lastFrameTime >= bitmap_anim_delays[currentFrame]){
                lastFrameTime = now;
                display.clearDisplay();
                display.drawBitmap(0, 0, bitmap_anim_frames[currentFrame], BITMAP_ANIM_WIDTH, BITMAP_ANIM_HEIGHT, SSD1306_WHITE);
                display.display();
                currentFrame = (currentFrame + 1) % BITMAP_ANIM_FRAME_COUNT;
            }
            break;
        }
        case STATE_WIFI_SYNC: {
            if (wifi_sta(10)){
                sync_real_time();
                char test[16];
                uint8_t retry = 0;
                while (!time_sync(test, sizeof(test)) && retry < 15){
                    delay(200);
                    retry++;
                }
            }
            WiFi.disconnect(true);
            WiFi.mode(WIFI_OFF);
            setup_sleep_weakups();
            break;
        }
        case STATE_AP_CONFIG: {
            server.handleClient();
            if(wifi_config_save){
                currentState = STATE_WIFI_SYNC;
            }
            if(now - stateStartTime >= AP_TIMEOUT_MS){
                WiFi.softAPdisconnect(true);
                WiFi.mode(WIFI_OFF);
                setup_sleep_weakups();
            }
            break;
        }
        default: 
            setup_sleep_weakups();
            break;
    }
}
