#ifndef GLOBAL_H
#define GLOBAL_H

#include "Lab3/chessCommon.h"
#include "ECE_ChessEngine.hpp"

extern float lightPower;
extern bool isRunning;
extern tModelMap cTModelMap;
extern ECE_ChessEngine engine;
extern std::vector<std::string> moveHistory;

#endif // GLOBAL_H