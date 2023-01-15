#pragma once
#include "StrikePad.h"

namespace BoardType {
// 盤面座標
typedef struct {
    size_t x;
    size_t y;
} Pos;

// 盤面狀態
enum Status {
    None,     // 無
    Crosses,  // 圈圈
    Nought    // 叉叉
};
}  // namespace BoardType

namespace PlayerType {
// 玩家
enum Player {
    Crosses = BoardType::Status::Crosses,  // 圈圈
    Nought = BoardType::Status::Nought     // 叉叉
};
}  // namespace PlayerType

namespace RenderType {
enum LEDMode {
    SetAllOff,
    SetSinglePad,
    SetAllPads
};

typedef struct {
    LEDMode mode;
    BoardType::Pos pos;
    StrikePad::LEDState led_state;
} LEDMsg;
}  // namespace RenderType