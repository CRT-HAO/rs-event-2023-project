#include <Arduino.h>
#include <ArduinoSTL.h>        // STL
#include <Arduino_FreeRTOS.h>  // FreeRTOS
#include <semphr.h>            // add the FreeRTOS functions for Semaphores (or Flags).

#include "StrikePad.h"
#include "config.h"
#include "pad.h"
#include "type.h"

SemaphoreHandle_t xSerialSemaphore;

// 敲擊墊觸碰隊列
QueueHandle_t xStrikePadTouchQueue;

void TaskStartup(void *pvParameters);
void TaskSensor(void *pvParameters);
void TaskGame(void *pvParameters);
void TaskBlink(void *pvParameters);

void setup() {
    Serial.begin(9600);

    if (xSerialSemaphore == NULL) {
        xSerialSemaphore = xSemaphoreCreateMutex();
        if ((xSerialSemaphore) != NULL)
            xSemaphoreGive((xSerialSemaphore));
    }

    // 創建啟動任務
    xTaskCreate(TaskStartup, "Startup", 128, NULL, 0, NULL);
    // 創建閃爍任務
    xTaskCreate(TaskBlink, "Blink", 128, NULL, 4, NULL);
}

void loop() {
    // 所有程序都以任務形式執行，這裡不會有東西
}

void TaskStartup(void *pvParameters __attribute__((unused))) {
    xStrikePadTouchQueue = xQueueCreate(1, sizeof(BoardPos));

    // 初始化敲擊墊
    init_strike_pads();

    // LED燈測試
    // for (size_t m = 0; m < 3; m++) {
    //     for (size_t i = 0; i < pads.size(); i++) {
    //         for (size_t j = 0; j < pads[i].size(); j++) {
    //             StrikePad::LEDState state;
    //             switch (m) {
    //                 case 0:
    //                     state = StrikePad::LEDState::All_White;
    //                     break;
    //                 case 1:
    //                     state = StrikePad::LEDState::Crosses;
    //                     break;
    //                 case 2:
    //                     state = StrikePad::LEDState::Nought;
    //                     break;
    //             }
    //             pads[i][j].setLEDState(state);
    //             delay(100);
    //         }
    //     }
    //     delay(500);
    //     for (size_t i = 0; i < pads.size(); i++) {
    //         for (size_t j = 0; j < pads[i].size(); j++) {
    //             pads[i][j].setLEDState(StrikePad::LEDState::Off);
    //         }
    //     }
    // }

    // 創建感測任務
    xTaskCreate(TaskSensor, "Sensor", 128, NULL, 0, NULL);
    // 創建遊戲任務
    xTaskCreate(TaskGame, "Game", 128, NULL, 1, NULL);

    // 刪除本任務
    vTaskDelete(NULL);
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

/**
 * @brief 感測任務
 *
 * 用於處理感測器數據
 * @param __attribute__
 */
void TaskSensor(void *pvParameters __attribute__((unused))) {
    std::vector<std::vector<unsigned long>> last_touch_release(BOARD_ROWS, std::vector<unsigned long>(BOARD_COLS, 0));
    for (;;) {
        // 掃描敲擊墊觸碰感應
        for (size_t i = 0; i < pads.size(); i++) {
            for (size_t j = 0; j < pads[i].size(); j++) {
                if (pads[i][j].readSensorRAW() < STRIKEPADS_TOUCH_THRESHOLD) {
                    last_touch_release[i][j] = millis();
                }
                if (millis() - last_touch_release[i][j] >= STRIKEPADS_TOUCH_MS_THRESHOLD) {
                    BoardPos pos;
                    pos.x = i;
                    pos.y = j;
                    xQueueSendToBack(xStrikePadTouchQueue, &pos, portMAX_DELAY);
                    last_touch_release[i][j] = millis();
                }
                // if (pads[i][j].readSensorRAW() > STRIKEPADS_TOUCH_THRESHOLD) {
                //     BoardPos pos;
                //     pos.x = i;
                //     pos.y = j;
                //     // if (xSemaphoreTake(xSerialSemaphore, (TickType_t)5) == pdTRUE) {
                //     //     std::cout << "x: " << i << ", y:" << j << ", val: " << val << std::endl;
                //     //     xSemaphoreGive(xSerialSemaphore);
                //     // }
                //     xQueueSendToBack(xStrikePadTouchQueue, &pos, portMAX_DELAY);
                // }
            }
        }
        vTaskDelay(1);
    }
}

void TaskGame(void *pvParameters __attribute__((unused))) {
    BoardPos touch_pos;
    for (;;) {
        xQueueReceive(xStrikePadTouchQueue, &touch_pos, portMAX_DELAY);
        if (xSemaphoreTake(xSerialSemaphore, (TickType_t)5) == pdTRUE) {
            std::cout << "Touch! x: " << touch_pos.x << ", y:" << touch_pos.y << std::endl;
            xSemaphoreGive(xSerialSemaphore);
        }
        vTaskDelay(1);
    }
}

void TaskBlink(void *pvParameters __attribute__((unused))) {
    pinMode(LED_BUILTIN, OUTPUT);
    bool status = false;
    for (;;) {
        digitalWrite(LED_BUILTIN, status = !status);
        delay(500);
    }
}