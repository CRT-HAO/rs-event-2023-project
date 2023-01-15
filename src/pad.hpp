#pragma once
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <ArduinoSTL.h>

#include "StrikePad.h"
#include "config.h"

// 定義3組燈條
Adafruit_NeoPixel strip1(60, 3, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(60, 4, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3(60, 5, NEO_GRB + NEO_KHZ800);

// 存放所有敲擊墊的2維陣列
std::vector<std::vector<StrikePad>> pads(BOARD_SIZE, std::vector<StrikePad>());

/**
 * @brief 初始化敲擊墊
 *
 */
void init_strike_pads() {
    strip1.begin();
    strip2.begin();
    strip3.begin();

    pads[0].push_back(StrikePad(31, strip1, 0, 0, 1, 1));
    pads[0].push_back(StrikePad(33, strip1, 2, 2, 3, 3));
    pads[0].push_back(StrikePad(35, strip1, 4, 4, 5, 5));

    pads[1].push_back(StrikePad(37, strip2, 0, 0, 1, 1));
    pads[1].push_back(StrikePad(39, strip2, 2, 2, 3, 3));
    pads[1].push_back(StrikePad(41, strip2, 4, 4, 5, 5));

    pads[2].push_back(StrikePad(43, strip3, 0, 0, 1, 1));
    pads[2].push_back(StrikePad(45, strip3, 2, 2, 3, 3));
    pads[2].push_back(StrikePad(47, strip3, 4, 4, 5, 5));
}