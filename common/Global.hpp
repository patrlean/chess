#ifndef GLOBAL_H
#define GLOBAL_H

#include "Lab3/chessCommon.h"
#include "ECE_ChessEngine.hpp"
#include "chessPosition.hpp"

extern float lightPower;
extern bool isRunning;
extern tModelMap cTModelMap;
extern ECE_ChessEngine engine;
extern std::vector<std::string> moveHistory;
extern ChessPosition whiteCapturePos;
extern ChessPosition blackCapturePos;
extern bool checkmate;

#endif // GLOBAL_H