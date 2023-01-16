#include "tictactoe.h"

#include <ArduinoSTL.h>

TicTacToe::TicTacToe(size_t board_size, bool allow_override) {
    this->board_size = board_size;
    this->allow_override = allow_override;
    for (size_t i = 0; i < this->board_size; i++) {
        this->board.push_back(
            std::vector<TicTacToe::ChessType>(this->board_size, TicTacToe::ChessType::None));
    }
}

void TicTacToe::setChessType(TicTacToe::ChessPos pos, TicTacToe::ChessType status) {
    this->board[pos.y][pos.x] = status;
}

bool TicTacToe::setChess(TicTacToe::Player player, TicTacToe::ChessPos pos) {
    ChessType target_chess = (player == TicTacToe::Player::Crosses) ? TicTacToe::ChessType::Crosses : TicTacToe::ChessType::Nought;

    // 如果目標位置不是空的，而且不允許覆蓋棋子
    // 則落棋失敗
    if (board[pos.y][pos.x] != TicTacToe::ChessType::None && !(this->allow_override)) {
        return false;
    }

    // 如果目標位置的棋子與目標棋子相同
    // 則落棋失敗
    if (board[pos.y][pos.x] == target_chess) {
        return false;
    }

    board[pos.y][pos.x] = target_chess;
    return true;
}

bool TicTacToe::setChess(TicTacToe::ChessPos pos, bool swap_player) {
    bool result = this->setChess(this->getCurrentPlayer(), pos);
    if (swap_player && result) {
        this->swapPlayer();
    }
    return result;
}

void TicTacToe::setCurrentPlayer(TicTacToe::Player player) {
    this->current_player = player;
}

TicTacToe::Player TicTacToe::getCurrentPlayer() {
    return this->current_player;
}

TicTacToe::Player TicTacToe::swapPlayer() {
    this->setCurrentPlayer(
        (this->getCurrentPlayer() == TicTacToe::Player::Crosses) ? TicTacToe::Player::Nought : TicTacToe::Player::Crosses);
    return this->getCurrentPlayer();
}

bool TicTacToe::getGameResult(TicTacToe::ChessType& winner, std::vector<TicTacToe::ChessPos>& winner_chess_pos) {
    for (size_t m = 0; m < 5; m++) {
        bool b = false;
        winner_chess_pos.clear();
        size_t same_chess_number = 0;
        TicTacToe::ChessType last_chess;
        size_t x = 0, y = 0;
        for (size_t i = 0; i < this->board_size; i++) {
            switch (m) {
                case 0:  // 橫排
                    winner_chess_pos.clear();
                    same_chess_number = 0;
                    for (size_t j = 0; j < this->board_size; j++) {
                        x = j;
                        y = i;
                        if ((board[y][x] == last_chess && board[y][x] != TicTacToe::ChessType::None) || j == 0) {
                            same_chess_number++;
                            last_chess = board[y][x];
                            winner_chess_pos.push_back(TicTacToe::ChessPos{y, x});
                        } else {
                            break;
                        }
                    }
                    winner = last_chess;
                    if (same_chess_number >= this->board_size) return true;
                    break;
                case 1:  // 直排
                    winner_chess_pos.clear();
                    same_chess_number = 0;
                    for (size_t j = 0; j < this->board_size; j++) {
                        x = i;
                        y = j;
                        if ((board[y][x] == last_chess && board[y][x] != TicTacToe::ChessType::None) || j == 0) {
                            same_chess_number++;
                            last_chess = board[y][x];
                            winner_chess_pos.push_back(TicTacToe::ChessPos{y, x});
                        } else {
                            break;
                        }
                    }
                    winner = last_chess;
                    if (same_chess_number >= this->board_size) return true;
                    break;
                case 2:  // 正斜線
                    x = i;
                    y = x;
                    if ((board[y][x] == last_chess && board[y][x] != TicTacToe::ChessType::None) || i == 0) {
                        same_chess_number++;
                        last_chess = board[y][x];
                        winner_chess_pos.push_back(TicTacToe::ChessPos{y, x});
                    } else {
                        b = true;  // break
                    }
                    winner = last_chess;
                    if (same_chess_number >= this->board_size) return true;
                    break;
                case 3:  // 反斜線
                    x = i;
                    y = (this->board_size - 1) - x;
                    if ((board[y][x] == last_chess && board[y][x] != TicTacToe::ChessType::None) || i == 0) {
                        same_chess_number++;
                        last_chess = board[y][x];
                        winner_chess_pos.push_back(TicTacToe::ChessPos{y, x});
                    } else {
                        b = true;  // break
                    }
                    winner = last_chess;
                    if (same_chess_number >= this->board_size) return true;
                    break;
                case 4:  // 平局
                    for (size_t j = 0; j < this->board_size; j++) {
                        x = i;
                        y = j;
                        if (board[y][x] != TicTacToe::ChessType::None) {
                            same_chess_number++;
                        } else {
                            break;
                        }
                    }
                    winner = TicTacToe::ChessType::None;
                    if (same_chess_number >= (this->board_size * this->board_size)) return true;
                    break;
            }
            if (b) break;
        }
    }
    winner = TicTacToe::ChessType::None;
    winner_chess_pos.clear();
    return false;
}

void TicTacToe::clearGame() {
    for (size_t i = 0; i < board.size(); i++) {
        for (size_t j = 0; j < board[i].size(); j++) {
            board[i][j] = TicTacToe::ChessType::None;
        }
    }
    this->setCurrentPlayer(TicTacToe::Player::Crosses);
}

int32_t TicTacToe::toInt(TicTacToe::Player player) {
    switch (player) {
        case TicTacToe::Player::Crosses:
            return 0;
        case TicTacToe::Player::Nought:
            return 1;
        default:
            return -1;
    }
}

std::string TicTacToe::toString(TicTacToe::Player player) {
    switch (player) {
        case TicTacToe::Player::Crosses:
            return std::string("Crosses");
        case TicTacToe::Player::Nought:
            return std::string("Nought");
        default:
            return std::string("null");
    }
}

int32_t TicTacToe::toInt(TicTacToe::ChessType chess) {
    switch (chess) {
        case TicTacToe::ChessType::None:
            return 0;
        case TicTacToe::ChessType::Crosses:
            return 1;
        case TicTacToe::ChessType::Nought:
            return 2;
        default:
            return -1;
    }
}

std::string TicTacToe::toString(TicTacToe::ChessType chess) {
    switch (chess) {
        case TicTacToe::ChessType::None:
            return std::string("None");
        case TicTacToe::ChessType::Crosses:
            return std::string("Crosses");
        case TicTacToe::ChessType::Nought:
            return std::string("Nought");
        default:
            return std::string("null");
    }
}