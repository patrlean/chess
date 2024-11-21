/*
Objective:
To create a dynamic 3D graphics application using lighting, shading, model transformations, and keyboard inputs.
Description :
    Uses the code from tutorial09_Assimp to create a C++ application that loads in the obj files 
    for the 3D chess pieces and the chess board.

    Obje files used - Lab3/Chess/chess.obj
                    - Lab3/Stone_Chess_Board/12951_Stone_Chess_Board_v1_L3.obj

    keyboard inputs definitions
        1) w key moves the camera radially closer to the origin.
        2) s key moves the camera radially farther from the origin.
        3) a key rotates the camera to the left maintaining the radial distance from the origin.
        4) d key rotates to camera to the right maintaining the radial distance from the origin.
        5) The up arrow key radially rotates the camera up.
        6) The down arrow radially rotates the camera down.
        7) The L key toggles the specular and diffuse components of the light on and off but leaves the ambient component unchanged.
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

#include "common/Global.hpp"

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
    setupChessBoard(cTModelMap);
     
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

    GLuint LightPowerID = glGetUniformLocation(programID, "lightPower");

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    // Start input thread
    startInputThread();

    // Start engine
    if (!engine.InitializeEngine("../Lab3/dragon-64bit.exe")) {
        return -1;
    }
    engine.SendMove("uci");
    std::cout << "Engine Response: " << engine.ReadFromEngine() << std::endl;
    engine.SendMove("ucinewgame");
    std::cout << "Engine Response: " << engine.ReadFromEngine() << std::endl;
    engine.SendMove("isready");
    std::cout << "Engine Response: " << engine.ReadFromEngine() << std::endl;
    
    whiteCapturePos.x = -2;
    whiteCapturePos.y = 4;
    blackCapturePos.x = 9;
    blackCapturePos.y = 4;
    do{
        processCommand(cTModelMap, gchessComponents);
        if (checkmate) {
            break;
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
            std::vector<tPosition> cTPositions = cTModelMap[cit->getComponentID()];
            
            // Repeat for pair of players using repetition count
            for (unsigned int pit = 0; pit < cTPositions.size(); pit++)
            {
                // Modify the X for player repetition
                tPosition cTPositionMorph = cTPositions[pit];
                // cTPositionMorph.tPos.x += pit * cTPositionMorph.rDis * CHESS_BOX_SIZE; // ?
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
                
                // Pass light power to fragment shader
                glUniform1f(LightPowerID, lightPower);

                // Bind our texture (set it up)
                cit->setupTexture(TextureID);

                // Render buffers
                cit->renderMesh();
            }
        }

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        // break if isrunning is false
        if (!isRunning) {
            break;
        }

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 && isRunning);

    // clear input thread
    cleanupInputThread();

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
    // 使用 emplace 方法，并将 tPosition 放入 vector 中
    
    // 棋盘
    cTModelMap.emplace("12951_Stone_Chess_Board", std::vector<tPosition>{
        tPosition{
            "chessBoard",           
            0,                      
            1, 0, 0.f,              
            {1, 0, 0},              
            glm::vec3(CBSCALE),     
            {0.f, 0.f, PHEIGHT}     
        }
    });

    // 车 - 白车
    cTModelMap.emplace("TORRE3", std::vector<tPosition>{
        tPosition{
            "white_rook1",
            0,
            1, 7, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {-3.5f * CHESS_BOX_SIZE, -3.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "white_rook2", 
            0,
            1, 7, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {3.5f * CHESS_BOX_SIZE, -3.5f * CHESS_BOX_SIZE, PHEIGHT}
        }
    });

    // 马 - 两个白马的位置
    cTModelMap.emplace("Object3", std::vector<tPosition>{
        tPosition{
            "white_knight1",
            0,
            1, 5, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {-2.5f * CHESS_BOX_SIZE, -3.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "white_knight2",
            0,
            1, 5, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {2.5f * CHESS_BOX_SIZE, -3.5f * CHESS_BOX_SIZE, PHEIGHT}
        }
    });

    // 象 - 两个白象的位置
    cTModelMap.emplace("ALFIERE3", std::vector<tPosition>{
        tPosition{
            "white_bishop1",
            0,
            1, 3, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {-1.5f * CHESS_BOX_SIZE, -3.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "white_bishop2",
            0,
            1, 3, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {1.5f * CHESS_BOX_SIZE, -3.5f * CHESS_BOX_SIZE, PHEIGHT}
        }
    });

    // 后 - 一个白后
    cTModelMap.emplace("REGINA2", std::vector<tPosition>{
        tPosition{
            "white_queen",
            0,
            1, 0, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {-0.5f * CHESS_BOX_SIZE, -3.5f * CHESS_BOX_SIZE, PHEIGHT}
        }
    });

    // 王 - 一个白王
    cTModelMap.emplace("RE2", std::vector<tPosition>{
        tPosition{
            "white_king",
            0,
            1, 0, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {0.5f * CHESS_BOX_SIZE, -3.5f * CHESS_BOX_SIZE, PHEIGHT}
        }
    });

    // 兵 - 八个白兵的位置
    cTModelMap.emplace("PEDONE13", std::vector<tPosition>{
        tPosition{
            "white_pawn1",
            0,
            1, 1, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {-3.5f * CHESS_BOX_SIZE, -2.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "white_pawn2",
            0,
            1, 1, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {-2.5f * CHESS_BOX_SIZE, -2.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "white_pawn3",
            0,
            1, 1, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {-1.5f * CHESS_BOX_SIZE, -2.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "white_pawn4",
            0,
            1, 1, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {-0.5f * CHESS_BOX_SIZE, -2.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "white_pawn5",
            0,
            1, 1, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {0.5f * CHESS_BOX_SIZE, -2.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "white_pawn6",
            0,
            1, 1, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {1.5f * CHESS_BOX_SIZE, -2.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "white_pawn7",
            0,
            1, 1, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {2.5f * CHESS_BOX_SIZE, -2.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "white_pawn8",
            0,
            1, 1, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {3.5f * CHESS_BOX_SIZE, -2.5f * CHESS_BOX_SIZE, PHEIGHT}
        }
    });
    // add black chess pieces
    cTModelMap.emplace("TORRE02", std::vector<tPosition>{
        tPosition{
            "black_rook1",
            1,
            1, 7, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {-3.5f * CHESS_BOX_SIZE, 3.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "black_rook2",
            1,
            1, 7, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {3.5f * CHESS_BOX_SIZE, 3.5f * CHESS_BOX_SIZE, PHEIGHT}
        }
    });
    
    // 马 - 两个白马的位置
    cTModelMap.emplace("Object02", std::vector<tPosition>{
        tPosition{
            "black_knight1",
            1,
            1, 5, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {-2.5f * CHESS_BOX_SIZE, 3.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "black_knight2",
            1,
            1, 5, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {2.5f * CHESS_BOX_SIZE, 3.5f * CHESS_BOX_SIZE, PHEIGHT}
        }
    });

    // 象 - 两个象的位置
    cTModelMap.emplace("ALFIERE02", std::vector<tPosition>{
        tPosition{
            "black_bishop1",
            1,
            1, 3, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {-1.5f * CHESS_BOX_SIZE, 3.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "black_bishop2",
            1,
            1, 3, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {1.5f * CHESS_BOX_SIZE, 3.5f * CHESS_BOX_SIZE, PHEIGHT}
        }
    });

    // 后 - 一个后
    cTModelMap.emplace("REGINA01", std::vector<tPosition>{
        tPosition{
            "black_queen",
            1,
            1, 0, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {-0.5f * CHESS_BOX_SIZE, 3.5f * CHESS_BOX_SIZE, PHEIGHT}
        }
    });

    // 王 - 一个白王
    cTModelMap.emplace("RE01", std::vector<tPosition>{
        tPosition{
            "black_king",
            1,
            1, 0, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {0.5f * CHESS_BOX_SIZE, 3.5f * CHESS_BOX_SIZE, PHEIGHT}
        }
    });

    // 兵 - 八个黑兵的位置
    cTModelMap.emplace("PEDONE12", std::vector<tPosition>{
        tPosition{
            "black_pawn1",
            1,
            1, 1, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {-3.5f * CHESS_BOX_SIZE, 2.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "black_pawn2",
            1,
            1, 1, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {-2.5f * CHESS_BOX_SIZE, 2.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "black_pawn3",
            1,
            1, 1, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {-1.5f * CHESS_BOX_SIZE, 2.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "black_pawn4",
            1,
            1, 1, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {-0.5f * CHESS_BOX_SIZE, 2.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "black_pawn5",
            1,
            1, 1, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {0.5f * CHESS_BOX_SIZE, 2.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "black_pawn6",
            1,
            1, 1, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {1.5f * CHESS_BOX_SIZE, 2.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "black_pawn7",
            1,
            1, 1, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {2.5f * CHESS_BOX_SIZE, 2.5f * CHESS_BOX_SIZE, PHEIGHT}
        },
        tPosition{
            "black_pawn8",
            1,
            1, 1, 90.f,
            {1, 0, 0},
            glm::vec3(CPSCALE),
            {3.5f * CHESS_BOX_SIZE, 2.5f * CHESS_BOX_SIZE, PHEIGHT}
        }
    });
}
