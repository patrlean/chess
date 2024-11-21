#ifndef UTILS_HPP
#define UTILS_HPP

#include <unordered_map>
#include <string>
#include "common/chessPosition.hpp"
#include <glm/glm.hpp>
#include "Lab3/chessCommon.h"
#include <sstream>
#include "Global.hpp"
// input: string
// output: vector of strings
// split the string into tokens
std::vector<std::string> splitString(const std::string& input);

// input: UCI coordinate 
// output: component ID
std::string getComponentIDAtFrom(const ChessPosition& from);

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