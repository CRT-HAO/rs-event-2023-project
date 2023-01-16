#pragma once
#include <Adafruit_NeoPixel.h>

class StrikePad {
   public:
    /**
     * @brief LED燈狀態
     *
     */
    enum LEDState {
        Off,       // 關閉
        Crosses,   // 圈圈
        Nought,    // 叉叉
        All,       // 全亮
        All_White  // 全白
    };

    /**
     * @brief StrikePad 敲擊墊
     *
     * @param sensorPin 感測器腳位
     * @param neopixel Neopixel Reference
     */
    StrikePad(uint8_t sensorPin, Adafruit_NeoPixel& neopixel);

    /**
     * @brief StrikePad 敲擊墊
     *
     * @param sensorPin 感測器腳位
     * @param neopixel Neopixel Reference
     * @param crosses_start_no 圈圈燈開始燈珠編號
     * @param crosses_end_no 圈圈燈結束燈珠編號
     * @param nought_start_no 叉叉燈開始燈珠編號
     * @param nought_end_no 叉叉燈結束燈珠編號
     */
    StrikePad(uint8_t sensorPin, Adafruit_NeoPixel& neopixel, int32_t crosses_start_no, int32_t crosses_end_no, int32_t nought_start_no, int32_t nought_end_no);

    /**
     * @brief 讀取感測器原始數值
     *
     * @return int32_t 數值
     */
    int32_t readSensorRAW();

    /**
     * @brief 設置圈圈燈燈珠編號
     *
     * @param start_no 開始燈珠編號
     * @param end_no 結束燈珠編號
     */
    void setCrossesLED(int32_t start_no, int32_t end_no);

    /**
     * @brief 設置叉叉燈燈珠編號
     *
     * @param start_no 開始燈珠編號
     * @param end_no 結束燈珠編號
     */
    void setNoughtLED(int32_t start_no, int32_t end_no);

    /**
     * @brief 設置圈圈燈顏色
     *
     * @param color 顏色
     */
    void setCrossesLEDColor(uint32_t color);

    /**
     * @brief 設置叉叉燈顏色
     *
     * @param color 顏色
     */
    void setNoughtLEDColor(uint32_t color);

    /**
     * @brief 設置LED燈狀態
     *
     * @param state 狀態
     */
    void setLEDState(LEDState state);

    /**
     * @brief 獲取LED燈狀態
     *
     * @return LEDState 狀態
     */
    LEDState getLEDState() {
        return led_state;
    }

   private:
    uint8_t sensorPin;
    Adafruit_NeoPixel* neopixel;
    int32_t crosses_start_no;
    int32_t crosses_end_no;
    int32_t nought_start_no;
    int32_t nought_end_no;
    uint32_t crosses_led_color = Adafruit_NeoPixel::Color(0, 255, 0);
    uint32_t nought_led_color = Adafruit_NeoPixel::Color(255, 0, 0);
    LEDState led_state = LEDState::Off;

    /**
     * @brief 一次設置多個燈珠的顏色
     *
     * @param start_no 開始燈珠
     * @param end_no 結束燈珠
     * @param color 顏色
     */
    void setMultiLEDColor(int32_t start_no, int32_t end_no, uint32_t color);

    /**
     * @brief 清空LED燈顏色
     *
     */
    void clearLED();

    /**
     * @brief 刷新LED燈
     *
     * 設置完色彩後須刷新才會顯示
     */
    void updateLED();
};