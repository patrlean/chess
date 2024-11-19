#ifndef UTILS_HPP
#define UTILS_HPP

#include <unordered_map>
#include <string>
#include "common/chessPosition.hpp"
#include <glm/glm.hpp>
#include "Lab3/chessCommon.h"

// input: UCI coordinate 
// output: component ID
std::string getComponentIDAtFrom(ChessPosition& from, tPosition & cTposition);

// input: UCI coordinate 
// output: chess position in integer
ChessPosition uciToPosition(const std::string& uciPos);

// input: tPos
// output: chess position in integer
ChessPosition tPosToChessPosition(glm::vec3 tPos);

// input: chess position in integer
// output: tPos
glm::vec3 chessPositionToTPos(ChessPosition chessPos);

#endif // UTILS_HPP