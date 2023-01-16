#include <Arduino.h>
#include <ArduinoSTL.h>        // STL
#include <Arduino_FreeRTOS.h>  // FreeRTOS
#include <semphr.h>            // add the FreeRTOS functions for Semaphores (or Flags).

#include "config.h"
#include "pad.hpp"
#include "strike_pad.h"
#include "tictactoe.h"
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
    Serial.begin(115200);

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
    xStrikePadTouchQueue = xQueueCreate(1, sizeof(TicTacToe::ChessPos));
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
    xTaskCreate(TaskGame, "Game", 512, NULL, 2, NULL);
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
                    TicTacToe::ChessPos pos;
                    pos.y = i;
                    pos.x = j;
                    // 向隊列發送觸碰位置
                    xQueueSendToBack(xStrikePadTouchQueue, &pos, portMAX_DELAY);
                    delay(STRIKEPADS_TOUCH_DELAY_MS);
                    last_touch_release[i][j] = millis();
                }
            }
        }

        vTaskDelay(1);
    }
}

/**
 * @brief 遊戲任務
 *
 * 用於處理遊戲進度及判斷
 * @param __attribute__
 */
void TaskGame(void *pvParameters __attribute__((unused))) {
    TicTacToe game(BOARD_SIZE, GAME_ALLOW_OVERRIDE);
    TicTacToe::ChessPos touch_pos;
    RenderType::LEDMsg led_render_msg;
    TicTacToe::ChessType winner;
    vector<TicTacToe::ChessPos> winner_chess_pos;
    for (;;) {
        if (xSemaphoreTake(xSerialSemaphore, portMAX_DELAY) == pdTRUE) {
            cout << "Waiting player " << TicTacToe::toInt(game.getCurrentPlayer()) << " move..." << endl;
            xSemaphoreGive(xSerialSemaphore);
        }
        // 接收敲擊墊觸摸隊列
        xQueueReceive(xStrikePadTouchQueue, &touch_pos, portMAX_DELAY);
        if (game.setChess(touch_pos, false)) {
            if (xSemaphoreTake(xSerialSemaphore, portMAX_DELAY) == pdTRUE) {
                cout << "Player " << TicTacToe::toInt(game.getCurrentPlayer()) << " moved. Pos x: " << touch_pos.x << ", y: " << touch_pos.y << endl;
                xSemaphoreGive(xSerialSemaphore);
            }
            // 獲取遊戲結果
            bool game_over = game.getGameResult(winner, winner_chess_pos);
            if (!game_over || (game_over && winner == TicTacToe::ChessType::None)) {  // 如果遊戲未結束，或結束了，但是平局
                // 閃爍出棋的位置
                led_render_msg.type = RenderType::LEDSetType::Pads;
                led_render_msg.mode = RenderType::LEDSetMode::Blink;
                led_render_msg.pads_pos = vector<TicTacToe::ChessPos>{touch_pos};
                led_render_msg.led_state =
                    (game.getCurrentPlayer() == TicTacToe::Player::Crosses) ? StrikePad::LEDState::Crosses : StrikePad::LEDState::Nought;
                // 向隊列發送LED渲染
                xQueueSendToBack(xLEDRenderQueue, &led_render_msg, portMAX_DELAY);
                // 交換玩家
                game.swapPlayer();
            }
            if (game_over) {  // 如果遊戲結束了
                if (xSemaphoreTake(xSerialSemaphore, portMAX_DELAY) == pdTRUE) {
                    cout << "Game is over! Winner is " << TicTacToe::toInt(winner) << endl;
                    xSemaphoreGive(xSerialSemaphore);
                }
                if (winner != TicTacToe::ChessType::None) {  // 如果不是平局的話
                    // 閃爍連線部分
                    led_render_msg.type = RenderType::LEDSetType::Pads;
                    led_render_msg.mode = RenderType::LEDSetMode::Blink;
                    led_render_msg.pads_pos = winner_chess_pos;
                    led_render_msg.led_state =
                        (game.getCurrentPlayer() == TicTacToe::Player::Crosses) ? StrikePad::LEDState::Crosses : StrikePad::LEDState::Nought;
                    // 向隊列發送LED渲染
                    xQueueSendToBack(xLEDRenderQueue, &led_render_msg, portMAX_DELAY);
                }
                // 遊戲結束後畫面停留一段時間
                delay(GAME_OVER_STAY_MS);
                // 清除遊戲畫面
                led_render_msg.type = RenderType::LEDSetType::All;
                led_render_msg.mode = RenderType::LEDSetMode::Off;
                // 向隊列發送LED渲染
                xQueueSendToBack(xLEDRenderQueue, &led_render_msg, portMAX_DELAY);
                game.clearGame();
            }
        }

        vTaskDelay(1);
    }
}

/**
 * @brief 渲染任務
 *
 * 負責渲染LED燈動畫
 * @param __attribute__
 */
void TaskRender(void *pvParameters __attribute__((unused))) {
    RenderType::LEDMsg led_render_msg;
    std::vector<TicTacToe::ChessPos> pads_pos;
    for (;;) {
        // 接收LED燈渲染隊列
        xQueueReceive(xLEDRenderQueue, &led_render_msg, portMAX_DELAY);
        switch (led_render_msg.type) {
            case RenderType::LEDSetType::All:
                pads_pos.clear();
                for (size_t i = 0; i < pads.size(); i++) {
                    for (size_t j = 0; j < pads[i].size(); j++) {
                        pads_pos.push_back(TicTacToe::ChessPos{i, j});
                    }
                }
                break;
            case RenderType::LEDSetType::Pads:
                pads_pos = led_render_msg.pads_pos;
                break;
        }
        switch (led_render_msg.mode) {
            case RenderType::LEDSetMode::Off:
                for (size_t j = 0; j < pads_pos.size(); j++) {
                    pads[pads_pos[j].y][pads_pos[j].x].setLEDState(StrikePad::LEDState::Off);
                }
                break;
            case RenderType::LEDSetMode::Set:
                for (size_t j = 0; j < pads_pos.size(); j++) {
                    pads[pads_pos[j].y][pads_pos[j].x].setLEDState(led_render_msg.led_state);
                }
                break;
            case RenderType::LEDSetMode::Blink:
                for (size_t i = 0; i < STRIKEPADS_TOUCH_BLINK_TIMES * 2; i++) {
                    for (size_t j = 0; j < pads_pos.size(); j++) {
                        pads[pads_pos[j].y][pads_pos[j].x].setLEDState(
                            i % 2 == 0 ? led_render_msg.led_state : StrikePad::LEDState::Off);
                    }
                    delay(STRIKEPADS_TOUCH_BLINK_MS);
                }
                for (size_t j = 0; j < pads_pos.size(); j++) {
                    pads[pads_pos[j].y][pads_pos[j].x].setLEDState(led_render_msg.led_state);
                }
                break;
            default:
                break;
        }

        vTaskDelay(1);
    }
}

/**
 * @brief 閃爍任務
 *
 * 閃爍主機上的內置LED燈，用於確保程序還活著
 * @param __attribute__
 */
void TaskBlink(void *pvParameters __attribute__((unused))) {
    pinMode(LED_BUILTIN, OUTPUT);
    bool status = false;
    for (;;) {
        digitalWrite(LED_BUILTIN, status = !status);
        vTaskDelay(30);  // 450ms
    }
}