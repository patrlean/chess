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
// output: ChessPosition
ChessPosition uciToPosition(const std::string& uciPos);
// input: tPos
// output: ChessPosition
ChessPosition tPosToChessPosition(glm::vec3 tPos);

#endif // UTILS_HPP