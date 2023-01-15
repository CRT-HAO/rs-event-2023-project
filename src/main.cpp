#include <Arduino.h>
#include <ArduinoSTL.h>        // STL
#include <Arduino_FreeRTOS.h>  // FreeRTOS
#include <semphr.h>            // add the FreeRTOS functions for Semaphores (or Flags).

#include "StrikePad.h"
#include "config.h"
#include "pad.hpp"
#include "type.h"

using namespace std;

SemaphoreHandle_t xSerialSemaphore;

// 敲擊墊觸碰隊列
QueueHandle_t xStrikePadTouchQueue;
// LED燈渲染隊列
QueueHandle_t xLEDRenderQueue;

void TaskStartup(void *pvParameters);
void TaskSensor(void *pvParameters);
void TaskGame(void *pvParameters);
void TaskRender(void *pvParameters);
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
    xTaskCreate(TaskBlink, "Blink", 128, NULL, 0, NULL);
}

void loop() {
    // 所有程序都以任務形式執行，這裡不會有東西
}

void TaskStartup(void *pvParameters __attribute__((unused))) {
    xStrikePadTouchQueue = xQueueCreate(1, sizeof(BoardType::Pos));
    xLEDRenderQueue = xQueueCreate(1, sizeof(RenderType::LEDMsg));

    // 初始化敲擊墊
    init_strike_pads();

    // LED燈測試
    for (size_t m = 0; m < 3; m++) {
        for (size_t i = 0; i < pads.size(); i++) {
            for (size_t j = 0; j < pads[i].size(); j++) {
                StrikePad::LEDState state;
                switch (m) {
                    case 0:
                        state = StrikePad::LEDState::All_White;
                        break;
                    case 1:
                        state = StrikePad::LEDState::Crosses;
                        break;
                    case 2:
                        state = StrikePad::LEDState::Nought;
                        break;
                }
                pads[i][j].setLEDState(state);
                delay(100);
            }
        }
        delay(500);
        for (size_t i = 0; i < pads.size(); i++) {
            for (size_t j = 0; j < pads[i].size(); j++) {
                pads[i][j].setLEDState(StrikePad::LEDState::Off);
            }
        }
    }

    // 創建感測任務
    xTaskCreate(TaskSensor, "Sensor", 128, NULL, 1, NULL);
    // 創建遊戲任務
    xTaskCreate(TaskGame, "Game", 128, NULL, 2, NULL);
    // 創建渲染任務
    xTaskCreate(TaskRender, "Render", 128, NULL, 3, NULL);

    // 刪除本任務
    vTaskDelete(NULL);
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

/**
 * @brief 感測任務
 *
 * 用於偵測敲擊墊
 * @param __attribute__
 */
void TaskSensor(void *pvParameters __attribute__((unused))) {
    vector<vector<unsigned long>> last_touch_release(BOARD_SIZE, vector<unsigned long>(BOARD_SIZE, 0));
    for (;;) {
        // 掃描敲擊墊觸碰感應
        for (size_t i = 0; i < pads.size(); i++) {
            for (size_t j = 0; j < pads[i].size(); j++) {
                // 如果敲擊墊未被觸碰則刷新時間
                if (pads[i][j].readSensorRAW() < STRIKEPADS_TOUCH_THRESHOLD) {
                    last_touch_release[i][j] = millis();
                }
                // 如果敲擊墊超過域值時間未被刷新則表示被觸碰
                if (millis() - last_touch_release[i][j] >= STRIKEPADS_TOUCH_MS_THRESHOLD) {
                    BoardType::Pos pos;
                    pos.x = i;
                    pos.y = j;
                    // 向隊列發送觸碰位置
                    xQueueSendToBack(xStrikePadTouchQueue, &pos, portMAX_DELAY);
                    last_touch_release[i][j] = millis();
                }
                // if (pads[i][j].readSensorRAW() > STRIKEPADS_TOUCH_THRESHOLD) {
                //     BoardType::Pos pos;
                //     pos.x = i;
                //     pos.y = j;
                //     // if (xSemaphoreTake(xSerialSemaphore, (TickType_t)5) == pdTRUE) {
                //     //     cout << "x: " << i << ", y:" << j << ", val: " << val << endl;
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
    // 盤面狀態
    vector<vector<BoardType::Status>> board(BOARD_SIZE, vector<BoardType::Status>(BOARD_SIZE, BoardType::Status::None));
    // 當前玩家
    PlayerType::Player player = PlayerType::Player::Crosses;
    BoardType::Pos touch_pos;
    for (;;) {
        if (xSemaphoreTake(xSerialSemaphore, (TickType_t)5) == pdTRUE) {
            cout << "Now player is " << player << endl;
            xSemaphoreGive(xSerialSemaphore);
        }
        xQueueReceive(xStrikePadTouchQueue, &touch_pos, portMAX_DELAY);
        if (GAME_ALLOW_OVERRIDE || board[touch_pos.x][touch_pos.y] == BoardType::Status::None) {
            if (board[touch_pos.x][touch_pos.y] != (BoardType::Status)player) {
                RenderType::LEDMsg led_render_msg;
                led_render_msg.mode = RenderType::LEDMode::SetSinglePad;
                led_render_msg.pos = touch_pos;
                led_render_msg.led_state =
                    (player == PlayerType::Player::Crosses) ? StrikePad::LEDState::Crosses : StrikePad::LEDState::Nought;
                // 向隊列發送LED渲染
                xQueueSendToBack(xLEDRenderQueue, &led_render_msg, portMAX_DELAY);
                // 刷新盤面狀態
                board[touch_pos.x][touch_pos.y] = (BoardType::Status)player;
                // 交換玩家
                player = (player == PlayerType::Player::Crosses) ? PlayerType::Player::Nought : PlayerType::Player::Crosses;
                if (xSemaphoreTake(xSerialSemaphore, (TickType_t)5) == pdTRUE) {
                    cout << "Shot! x: " << touch_pos.x << ", y:" << touch_pos.y << ", player: " << player << endl;
                    xSemaphoreGive(xSerialSemaphore);
                }
            }
        }

        vTaskDelay(1);
    }
}

void TaskRender(void *pvParameters __attribute__((unused))) {
    RenderType::LEDMsg led_render_msg;
    for (;;) {
        xQueueReceive(xLEDRenderQueue, &led_render_msg, portMAX_DELAY);
        switch (led_render_msg.mode) {
            case RenderType::LEDMode::SetSinglePad:
                for (size_t i = 0; i < STRIKEPADS_TOUCH_BLINK_TIMES * 2; i++) {
                    pads[led_render_msg.pos.x][led_render_msg.pos.y].setLEDState(
                        i % 2 == 0 ? led_render_msg.led_state : StrikePad::LEDState::Off);
                    delay(STRIKEPADS_TOUCH_BLINK_MS);
                }
                pads[led_render_msg.pos.x][led_render_msg.pos.y].setLEDState(led_render_msg.led_state);
                break;
            default:
                break;
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