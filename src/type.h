#pragma once
#include <ArduinoSTL.h>

#include "strike_pad.h"
#include "tictactoe.h"

namespace RenderType {
enum LEDSetType {
    All,   // 全部敲擊墊
    Pads,  // 指定敲擊墊
};
enum LEDSetMode {
    Off,    // 關閉
    Set,    // 長亮
    Blink,  // 閃爍
};

typedef struct {
    LEDSetType type;
    LEDSetMode mode;
    std::vector<TicTacToe::ChessPos> pads_pos;
    StrikePad::LEDState led_state;
} LEDMsg;
}  // namespace RenderType