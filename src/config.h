#pragma once

#define BOARD_SIZE 3

#define STRIKEPADS_TOUCH_THRESHOLD 1
#define STRIKEPADS_TOUCH_MS_THRESHOLD 60
// 敲擊間隔
// 敲擊超過指定秒數後才繼續偵測
#define STRIKEPADS_TOUCH_DELAY_MS 1005

#define STRIKEPADS_TOUCH_BLINK_TIMES 4
#define STRIKEPADS_TOUCH_BLINK_MS 100

// 允許覆蓋
// 如果設為true則允許覆蓋已經被點擊的格子
#define GAME_ALLOW_OVERRIDE false
// 遊戲結束後畫面停留的時間
#define GAME_OVER_STAY_MS 3000