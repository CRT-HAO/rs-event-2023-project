#include "StrikePad.h"

#include <Arduino.h>

StrikePad::StrikePad(uint8_t sensorPin, Adafruit_NeoPixel& neopixel) {
    this->sensorPin = sensorPin;
    this->neopixel = &neopixel;
    pinMode(this->sensorPin, INPUT);
}

StrikePad::StrikePad(uint8_t sensorPin, Adafruit_NeoPixel& neopixel, int32_t crosses_start_no, int32_t crosses_end_no, int32_t nought_start_no, int32_t nought_end_no) {
    this->sensorPin = sensorPin;
    this->neopixel = &neopixel;
    this->setCrossesLED(crosses_start_no, crosses_end_no);
    this->setNoughtLED(nought_start_no, nought_end_no);
    pinMode(this->sensorPin, INPUT);
}

int32_t StrikePad::readSensorRAW() {
    return analogRead(this->sensorPin);
}

void StrikePad::setCrossesLED(int32_t start_no, int32_t end_no) {
    this->crosses_start_no = start_no;
    this->crosses_end_no = end_no;
}

void StrikePad::setNoughtLED(int32_t start_no, int32_t end_no) {
    this->nought_start_no = start_no;
    this->nought_end_no = end_no;
}

void StrikePad::setCrossesLEDColor(uint32_t color) {
    this->crosses_led_color = color;
}

void StrikePad::setNoughtLEDColor(uint32_t color) {
    this->nought_led_color = color;
}

void StrikePad::setLEDState(LEDState state) {
    switch (state) {
        case LEDState::Off:
            this->clearLED();
            break;
        case LEDState::Crosses:
            this->clearLED();
            this->setMultiLEDColor(this->crosses_start_no, this->crosses_end_no, this->crosses_led_color);
            break;
        case LEDState::Nought:
            this->clearLED();
            this->setMultiLEDColor(this->nought_start_no, this->nought_end_no, this->nought_led_color);
            break;
        case LEDState::All:
            this->clearLED();
            this->setMultiLEDColor(this->crosses_start_no, this->crosses_end_no, this->crosses_led_color);
            this->setMultiLEDColor(this->nought_start_no, this->nought_end_no, this->nought_led_color);
            break;
        case LEDState::All_White:
            uint32_t white = Adafruit_NeoPixel::Color(255, 255, 255);
            this->clearLED();
            this->setMultiLEDColor(this->crosses_start_no, this->crosses_end_no, white);
            this->setMultiLEDColor(this->nought_start_no, this->nought_end_no, white);
            break;
    }
    this->updateLED();
    this->led_state = state;
}

void StrikePad::setMultiLEDColor(int32_t start_no, int32_t end_no, uint32_t color) {
    for (int32_t i = 0; i < abs(end_no - start_no) + 1; i++) {
        this->neopixel->setPixelColor(i + (start_no < end_no ? start_no : end_no), color);
    }
}

void StrikePad::clearLED() {
    uint32_t black = Adafruit_NeoPixel::Color(0, 0, 0);
    setMultiLEDColor(this->crosses_start_no, this->crosses_end_no, black);
    setMultiLEDColor(this->nought_start_no, this->nought_end_no, black);
}

void StrikePad::updateLED() {
    this->neopixel->show();
}