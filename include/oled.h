#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 display;

void oledInit(int sda, int scl, uint32_t freq);
void oledClear();
template <typename T>
void oledPrint(int x, int y, const T& data){
    display.setCursor(x,y);
    display.print(data);
}
void oledDisplay();