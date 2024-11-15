/*

Objective:
Use the common.cpp file from the Git Tutorial and add a custom function
for ECE6122 Labs. It does NOT touch the existing functions.

*/

#ifndef CONTROLS_HPP
#define CONTROLS_HPP

#include <iostream>

struct ChessPosition {
    int x; // 0-7 对应 a-h
    int y; // 0-7 对应 1-8
};

void computeMatricesFromInputs();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
bool getLightSwitch();
std::vector<std::string> splitString(const std::string& input);
void processCommand();

// A custom function for Lab3
// Creates the view and Projection matrix based on following custom key definitions
// keyboard inputs definitions
//1) w key moves the camera radially closer to the origin.
//2) s key moves the camera radially farther from the origin.
//3) a key rotates the camera to the left maintaining the radial distance from the origin.
//4) d key rotates to camera to the right maintaining the radial distance from the origin.
//5) The up arrow key radially rotates the camera up.
//6) The down arrow radially rotates the camera down.
//7) The L key toggles the specular and diffuse components of the light on and off but leaves the ambient component unchanged.
//8) Pressing the escape key closes the window and exits the program

void computeMatricesFromInputsLab3();
void startInputThread();
void cleanupInputThread();

#endif