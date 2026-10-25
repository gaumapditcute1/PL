#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "oled.h"

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void oledInit(int sda, int scl, uint32_t freq){
    Wire.begin(sda, scl, freq);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false);
    display.setTextColor(WHITE);
    display.setTextSize(1);
}

void oledClear(){
    display.clearDisplay();
}

void oledDisplay(){
    display.display();
}



