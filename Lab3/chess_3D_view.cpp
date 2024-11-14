/*
Objective:
To create a dynamic 3D graphics application using lighting, shading, model transformations, and keyboard inputs.
Description :
    Uses the code from tutorial09_Assimp to create a C++ application that loads in the obj files 
    for the 3D chess pieces and the chess board.

    Obje files used - Lab3/Chess/chess.obj
                    - Lab3/Stone_Chess_Board/12951_Stone_Chess_Board_v1_L3.obj

    keyboard inputs definitions
        1) ‘w’ key moves the camera radially closer to the origin.
        2) ‘s’ key moves the camera radially farther from the origin.
        3) ‘a’ key rotates the camera to the left maintaining the radial distance from the origin.
        4) ‘d’ key rotates to camera to the right maintaining the radial distance from the origin.
        5) The up arrow key radially rotates the camera up.
        6) The down arrow radially rotates the camera down.
        7) The ‘L’ key toggles the specular and diffuse components of the light on and off but leaves the ambient component unchanged.
        8) Pressing the escape key closes the window and exits the program
*/

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
// User supporting files
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
// Lab3 specific chess class
#include "chessComponent.h"
#include "chessCommon.h"

// #include <unistd.h>

// Sets up the chess board
void setupChessBoard(tModelMap& cTModelMap);

int main( void )
{
    
    // Initialize GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make macOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "Game Of Chess 3D", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it is closer to the camera than the former one
    glDepthFunc(GL_LESS); 

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "../Lab3/StandardShading.vertexshader", "../Lab3/StandardShading.fragmentshader" );

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

    // Get a handle for our "lightToggleSwitch" uniform
    GLuint LightSwitchID = glGetUniformLocation(programID, "lightSwitch");

    // Create a vector of chess components class
    // Each component is fully self sufficient
    std::vector<chessComponent> gchessComponents;

    // Load the OBJ files
    bool cBoard = loadAssImpLab3("../Lab3/Lab3/Stone_Chess_Board/12951_Stone_Chess_Board_v1_L3.obj", gchessComponents);
    bool cComps = loadAssImpLab3("../Lab3/Lab3/Chess/chess-mod.obj", gchessComponents);
    std::cout << "cBoard loading successes" << std::endl;
    std::cout << "cComps loading successes" << std::endl;

    // Proceed iff OBJ loading is successful
    if (!cBoard || !cComps)
    {
        // Quit the program (Failed OBJ loading)
        std::cout << "Program failed due to OBJ loading failure, please CHECK!" << std::endl;
        return -1;
    }

    // Setup the Chess board locations
    tModelMap cTModelMap;
    setupChessBoard(cTModelMap);
    std::cout << "cTModelMap setup successes" << std::endl;

    // Load it into a VBO (One time activity)
    // Run through all the components for rendering
    for (auto cit = gchessComponents.begin(); cit != gchessComponents.end(); cit++)
    {
        // Setup VBO buffers
        cit->setupGLBuffers();
        // Setup Texture
        cit->setupTextureBuffers();
    }

    // Use our shader (Not changing the shader per chess component)
    glUseProgram(programID);

    // Get a handle for our "LightPosition" uniform
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    do{
        // Measure speed
        double currentTime = glfwGetTime();
        nbFrames++;

        // If last prinf() was more than 1sec ago
        if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
            // printf and reset
            printf("%f ms/frame\n", 1000.0 / double(nbFrames));
            nbFrames = 0;
            lastTime += 1.0;
        }

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Compute the VP matrix from keyboard and mouse input
        computeMatricesFromInputsLab3();
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();

        // Get light switch State (It's a toggle!)
        bool lightSwitch = getLightSwitch();
        // Pass it to Fragment Shader
        glUniform1i(LightSwitchID, static_cast<int>(lightSwitch));

        // Run through all the chess game components for rendering
        for (auto cit = gchessComponents.begin(); cit != gchessComponents.end(); cit++)
        {            
            // Seach for mesh rendering targets and counts
            tPosition cTPosition = cTModelMap[cit->getComponentID()];
            
            // Repeat for pair of players using repetition count
            for (unsigned int pit = 0; pit < cTPosition.rCnt; pit++)
            {
                // Modify the X for player repetition
                tPosition cTPositionMorph = cTPosition;
                cTPositionMorph.tPos.x += pit * cTPosition.rDis * CHESS_BOX_SIZE;
                // Pass it for Model matrix generation
                glm::mat4 ModelMatrix = cit->genModelMatrix(cTPositionMorph);
                // Genrate the MVP matrix
                glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

                // Send our transformation to the currently bound shader, 
                // in the "MVP" uniform
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
                glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
                glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

                // Light is placed right on the top of the board
                // with a decent height for good lighting across
                // the board!
                glm::vec3 lightPos = glm::vec3(0, 0, 15);
                glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

                // Bind our texture (set it up)
                cit->setupTexture(TextureID);

                // Render buffers
                cit->renderMesh();
            }
        }

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO, Texture (Done in class destructor) and shader 
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    // Shake hand for exit!
    return 0;
}

void setupChessBoard(tModelMap& cTModelMap)
{
    // Target spec Hash
    cTModelMap =
    {
        // Chess board              Count  rDis Angle      Axis             Scale                          Position (X, Y, Z)
        {"12951_Stone_Chess_Board", {1,    0,   0.f,    {1, 0, 0},    glm::vec3(CBSCALE), {0.f,     0.f,                             PHEIGHT}}},
        // First player             Count  rDis Angle      Axis             Scale                          Position (X, Y, Z)
        {"TORRE3",                  {2,   (8-1),90.f,   {1, 0, 0},    glm::vec3(CPSCALE), {-3.5*CHESS_BOX_SIZE, -3.5*CHESS_BOX_SIZE, PHEIGHT}}},
        {"Object3",                 {2,   (6-1),90.f,   {1, 0, 0},    glm::vec3(CPSCALE), {-2.5*CHESS_BOX_SIZE, -3.5*CHESS_BOX_SIZE, PHEIGHT}}},
        {"ALFIERE3",                {2,   (4-1),90.f,   {1, 0, 0},    glm::vec3(CPSCALE), {-1.5*CHESS_BOX_SIZE, -3.5*CHESS_BOX_SIZE, PHEIGHT}}},
        {"REGINA2",                 {1,    0,   90.f,   {1, 0, 0},    glm::vec3(CPSCALE), {-0.5*CHESS_BOX_SIZE, -3.5*CHESS_BOX_SIZE, PHEIGHT}}},
        {"RE2",                     {1,    0,   90.f,   {1, 0, 0},    glm::vec3(CPSCALE), { 0.5*CHESS_BOX_SIZE, -3.5*CHESS_BOX_SIZE, PHEIGHT}}},
        {"PEDONE13",                {8,    1,   90.f,   {1, 0, 0},    glm::vec3(CPSCALE), {-3.5*CHESS_BOX_SIZE, -2.5*CHESS_BOX_SIZE, PHEIGHT}}}
    };

    // Second player derived from first player!!
    // Second Player (TORRE02)
    cTModelMap["TORRE02"] = cTModelMap["TORRE3"];
    cTModelMap["TORRE02"].tPos.y = -cTModelMap["TORRE3"].tPos.y;
    // Second Player (Object02)
    cTModelMap["Object02"] = cTModelMap["Object3"];
    cTModelMap["Object02"].tPos.y = -cTModelMap["Object3"].tPos.y;
    // Second Player (ALFIERE02)
    cTModelMap["ALFIERE02"] = cTModelMap["ALFIERE3"];
    cTModelMap["ALFIERE02"].tPos.y = -cTModelMap["ALFIERE3"].tPos.y;
    // Second Player (REGINA01)
    cTModelMap["REGINA01"] = cTModelMap["REGINA2"];
    cTModelMap["REGINA01"].tPos.y = -cTModelMap["REGINA2"].tPos.y;
    // Second Player (RE01)
    cTModelMap["RE01"] = cTModelMap["RE2"];
    cTModelMap["RE01"].tPos.y = -cTModelMap["RE2"].tPos.y;
    // Second Player (PEDONE12)
    cTModelMap["PEDONE12"] = cTModelMap["PEDONE13"];
    cTModelMap["PEDONE12"].tPos.y = -cTModelMap["PEDONE13"].tPos.y;
}
