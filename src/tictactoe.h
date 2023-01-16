#pragma once
#include <ArduinoSTL.h>

class TicTacToe {
   public:
    // 棋子座標
    typedef struct {
        size_t y;
        size_t x;
    } ChessPos;

    // 棋子類型
    enum class ChessType {
        None,     // 無
        Crosses,  // 圈圈
        Nought    // 叉叉
    };

    // 玩家
    enum class Player {
        Crosses,  // 圈圈
        Nought    // 叉叉
    };

    typedef std::vector<std::vector<ChessType>> Board;

    /**
     * @brief Tic-Tac-Toe 井字棋
     *
     * @param board_size 棋盤尺寸
     * @param allow_override 允許玩家覆蓋已有旗子的位置
     */
    TicTacToe(size_t board_size, bool allow_override = true);

    /**
     * @brief 設置棋盤指定位置的狀態
     *
     * @param pos 位置
     * @param status 狀態
     */
    void setChessType(ChessPos pos, ChessType status);

    /**
     * @brief 指定玩家出棋
     *
     * @param player 玩家
     * @param pos 位置
     * @return true 落棋成功
     * @return false 落棋失敗
     */
    bool setChess(Player player, ChessPos pos);

    /**
     * @brief 當前玩家出棋
     *
     * @param pos 位置
     * @param swap_player 交換選手
     * @return true 落棋成功
     * @return false 落棋失敗
     */
    bool setChess(ChessPos pos, bool swap_player = true);

    /**
     * @brief 設置當前玩家
     *
     * @param player 玩家
     */
    void setCurrentPlayer(Player player);

    /**
     * @brief 獲取當前玩家
     *
     * @return 玩家
     */
    Player getCurrentPlayer();

    /**
     * @brief 交換玩家
     *
     * @return 當前玩家
     */
    Player swapPlayer();

    /**
     * @brief 獲取遊戲結果
     *
     * @param winner 贏家
     * @param winner_chess_pos 獲勝的棋子
     * @return true 已結束
     * @return false 未結束
     */
    bool getGameResult(TicTacToe::ChessType& winner, std::vector<TicTacToe::ChessPos>& winner_chess_pos);

    /**
     * @brief 清除遊戲
     *
     */
    void clearGame();

    static int32_t toInt(Player player);
    static std::string toString(Player player);

    static int32_t toInt(ChessType chess);
    static std::string toString(ChessType chess);

   private:
    size_t board_size = 0;
    bool allow_override;
    Board board;
    Player current_player = Player::Crosses;
};