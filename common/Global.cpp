#include "Global.hpp"

float lightPower = 400.0f;
bool isRunning = true;
tModelMap cTModelMap;
ECE_ChessEngine engine;
std::vector<std::string> moveHistory;
ChessPosition whiteCapturePos;
ChessPosition blackCapturePos;
bool checkmate = false;
