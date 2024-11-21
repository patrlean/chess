/*

Objective:
Use the common.cpp file from the Git Tutorial and add a custom function
for ECE6122 Labs. It does NOT touch the existing functions.

*/

#ifndef CONTROLS_HPP
#define CONTROLS_HPP

#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <unordered_map>


#include "utils.hpp"
#include "Global.hpp"
#include "chessPosition.hpp"
#include "Lab3/chessComponent.h"
#include "ECE_ChessEngine.hpp"

void computeMatricesFromInputs();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
bool getLightSwitch();
std::vector<std::string> splitString(const std::string& input);
void processCommand(tModelMap& tModelMap, std::vector<chessComponent>& chessComponents);
void computeMatricesFromInputsLab3();
void startInputThread();
void cleanupInputThread();
void moveChessPiece(const ChessPosition& from, const ChessPosition& to, std::string pieceID);
void moveChessPieceThread(ChessPosition from, ChessPosition to, std::string pieceID, bool isFirstMove);
void moveKnightPiece(const ChessPosition& from, const ChessPosition& to, std::string pieceID);
std::string getCurrentFen(const std::string& move);
void movePieceOutOfBoard(const ChessPosition& to, std::string pieceID);
#endif