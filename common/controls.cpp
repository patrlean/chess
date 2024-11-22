/*

Objective:
Use the common.cpp file from the Git Tutorial and add a custom function
for ECE6122 Labs. It does NOT touch the existing functions.

*/

#include "controls.hpp"
#include "Global.hpp"
#include "utils.hpp"
#include <chrono>
#include <thread>

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;
bool toggleSDColor=true;
int dbnceCnt = 20;

// 添加全局变量
std::thread inputThread;
std::queue<std::string> commandQueue;
std::mutex queueMutex;
std::mutex modelMapMutex;

std::mutex moveMutex;
std::condition_variable moveCondition;
std::atomic<bool> firstMoveComplete(false);


glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}

bool getLightSwitch()
{
	return toggleSDColor;
}

// Initial position : on +Z
glm::vec3 position = glm::vec3( 0, 0, 5);
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.005f;

// Lab3 key movement coordinates (spherical)
float cRadius = 35.0f;
float cPhi = -90.f;
float cTheta = 0.0f;

// Speed
float speedLab3 = 10.0f; // 3 units / second

// Debounce limit
const int DEB_LIMIT = 40;

// 验证移动是否合法
bool isValidMove(const std::string& move) {
    if (move.length() != 4) return false;
    
    ChessPosition from = uciToPosition(move.substr(0, 2));
    ChessPosition to = uciToPosition(move.substr(2, 2));
    
    // 检查坐标是否有效
    if (from.x < 0 || from.x > 8 || to.x < 0 || to.x > 8 || from.y < 0 || from.y > 8 || to.y < 0 || to.y > 8) return false;
    
    return true;
}

void computeMatricesFromInputs(){

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(window, 1024/2, 768/2);

	// Compute new orientation
	horizontalAngle += mouseSpeed * float(1024/2 - xpos );
	verticalAngle   += mouseSpeed * float( 768/2 - ypos );

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle), 
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f/2.0f), 
		0,
		cos(horizontalAngle - 3.14f/2.0f)
	);
	
	// Up vector
	glm::vec3 up = glm::cross( right, direction );

	// Move forward
	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
		position += direction * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
		position -= direction * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
		position -= right * deltaTime * speed;
	}

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix       = glm::lookAt(
								position,           // Camera is here
								position+direction, // and looks here : at the same position, plus "direction"
								up                  // Head is up (set to 0,-1,0 to look upside-down)
						   );

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}

// input thread function
void inputThreadFunction() {
    while (isRunning) {
        std::string input;
        std::cout << "Please enter a command: ";
        std::getline(std::cin, input);
        
        if (!input.empty()) {
            std::lock_guard<std::mutex> lock(queueMutex);
            commandQueue.push(input);
        }
    }
}

// process command from console
void processCommand(tModelMap& tModelMap, std::vector<chessComponent>& chessComponents) {
    std::string input;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (commandQueue.empty()) {
            return;
        }
        input = commandQueue.front();
        commandQueue.pop();
    }
	

    std::vector<std::string> tokens = splitString(input);
    if (tokens.empty()) return;

    std::string command = tokens[0];
	if (tokens.size() == 1) {
		if (command == "quit") {
			isRunning = false;
			return;
		}
		std::cout << "Invalid command or move!!" << std::endl;
		return;
	}
	std::string move = tokens[1];
    if (command == "move") {
		/*
		1st: get response from engine
		2nd: move white chess piece
		3rd: move black chess piece
		4th: update move history
		*/

		// 1st: get response from engine
		// check validation
        if (!isValidMove(move)) {
            std::cout << "Invalid move coordinates!" << std::endl;
            return;
        }
		// move is the current move
		// sending message is the combination of current move and all previous moves
		std::string sendingFen = getCurrentFen(move);
		if (!engine.SendMove(sendingFen)) {
			std::cout << "Failed to send move to engine!" << std::endl;
			return;
		}
		if (!engine.SendMove("go depth 2")) {
			std::cout << "Failed to send go depth 5 to engine!" << std::endl;
			return;
		}
		// std::cout << "Sent move to engine: " << sendingFen << std::endl;
		// get the best move from engine
		std::string response;

		while (true) {
			response = engine.ReadFromEngine();
			if (response.find("bestmove") != std::string::npos) {
				break;
			}else if (response.find("mate") != std::string::npos) {
				std::vector<std::string> tokens = splitString(response);
				if (tokens.size() > 1) {
					checkmate = true;
					for (int idx = 0; idx < tokens.size(); idx++) {
						if (tokens[idx] == "mate") {
							if (tokens[idx + 1] == "1") {
								std::cout << "Checkmate! You lose!!" << std::endl;
							} else {
								std::cout << "Checkmate! You win!!" << std::endl;
							}
						}
					}
				}
				break;
			}
		}
		// std::cout << "Received best move from engine: " << response << std::endl;
		// split the response to get the best move
		std::string bestMove;
		std::vector<std::string> tokens = splitString(response);
		if (!engine.getResponseMove(tokens, bestMove)) {
			std::cout << "Invalid move coordinates!" << std::endl;
			return;
		}
		// check move validation
		if (bestMove == "0000" || bestMove == "nan") {
			std::cout << "Invalid move!" << std::endl;
			return;
		}
		
		firstMoveComplete = false;

		// 2nd: move white chess piece
        ChessPosition from = uciToPosition(move.substr(0, 2));
        ChessPosition to = uciToPosition(move.substr(2, 2));
		
		std::string pieceID;
	
        // 查找from位置上面的棋子
		pieceID = getComponentIDAtFrom(from);
		
		std::thread moveThread(moveChessPieceThread, from, to, pieceID, true);
		moveThread.detach();
		
		// 3rd: move black chess piece
		ChessPosition from2 = uciToPosition(bestMove.substr(0, 2));
        ChessPosition to2 = uciToPosition(bestMove.substr(2, 2));
		std::string pieceID2;
		// 查找from位置上面的棋子
		pieceID2 = getComponentIDAtFrom(from2);
        
        std::thread bestMoveThread([from2, to2, pieceID2]() {
			std::unique_lock<std::mutex> lock(moveMutex);
			moveCondition.wait(lock, []{ return firstMoveComplete.load(); });
			
			// 第一个移动完成后，执行第二个移动
			moveChessPieceThread(from2, to2, pieceID2, false);
		});
		bestMoveThread.detach();

		// 4th: update move history
		moveHistory.push_back(move);
		moveHistory.push_back(bestMove);
    }
    else if (command == "camera") {
        if (tokens.size() != 4) {
            std::cout << "Invalid command or move!!" << std::endl;
            return;
        }
        try {
            float radius = std::stof(tokens[1]);
            float phi = std::stof(tokens[2]);
            float theta = std::stof(tokens[3]);
            // check if the input is valid
            if (radius < 10 || radius > 80 || phi < 0 || phi > 360 || theta < 0 ) {
                std::cout << "Invalid command or move!!" << std::endl;
                return;
            }
            // update camera position
            cRadius = radius;
            cPhi = phi;
            cTheta = theta;
        }
        catch (...) {
            std::cout << "Invalid command or move!!" << std::endl;
        }
    }
    else if (command == "power") {
        if (tokens.size() != 2) {
            std::cout << "Invalid command or move!!" << std::endl;
            return;
        }
        try {
            float power = std::stof(tokens[1]);
            lightPower = power;
        }
        catch (...) {
            std::cout << "Invalid command or move!!" << std::endl;
        }
    }
    else {
        std::cout << "Invalid command or move!!" << std::endl;
    }
}

void computeMatricesFromInputsLab3()
{
	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Keep rolling debounce count
	// if not at Debounce limit
	if (dbnceCnt <= DEB_LIMIT)
	{
		dbnceCnt++;
	}

	// Create origin
	glm::vec3 origin = glm::vec3(0, 0, 0);
	// Up vector (look in the z direction)
	glm::vec3 up = glm::vec3(0, 0, 1);

	// w key moves the camera radially closer to the origin
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
	{
		// Last cRadius to avoids camera jerks
		// at maximum zoom (Stas stable)
		float lastcRadius = cRadius;
		cRadius -= deltaTime * speedLab3;
		if (cRadius < 0)
		{ // Avoid zooming into -ve zone
			cRadius = lastcRadius;
		}
	}
	// s key moves the camera radially farther from the origin.
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) 
	{
		cRadius += deltaTime * speedLab3;
	}
	// a key rotates the camera to the left maintaining the radial distance from the origin.
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) 
	{
		cPhi += deltaTime * speedLab3;
		// Check for wrap around condition
		if (cPhi > 360.0f)
		{ // Wrap around to zero
			cPhi = cPhi - 360.f;
		}
	}
	// d key rotates to camera to the right maintaining the radial distance from the origin
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) 
	{
		cPhi -= deltaTime * speedLab3;
		// Check for wrap around condition
		if (cPhi < 0.f)
		{ // Wrap around to zero
			cPhi = 360.f + cPhi;
		}
	}
	// The up arrow key radially rotates the camera up
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) 
	{
		cTheta += deltaTime * speedLab3;
		// Check for wrap around condition
		if (cTheta > 360.0f)
		{ // Wrap around to zero
			cTheta = cTheta - 360.f;
		}
	}
	// The down arrow key radially rotates the camera down
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) 
	{
		cTheta -= deltaTime * speedLab3;
		// Check for wrap around condition
		if (cTheta < 0.0f)
		{ // Wrap around to zero
			cTheta = 360.f + cTheta;
		}
	}

	// The L key toggles the specular and diffuse components of the light on and off 
	// but leaves the ambient component unchanged.
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
		if (dbnceCnt >= DEB_LIMIT)
		{ // Toggle light switch and restart the Debounce count
		    toggleSDColor = !toggleSDColor;
			dbnceCnt = 0;
		}
	}

	// Convert to Cartesian co-ordinate system
	float posX = cRadius * sin(glm::radians(cTheta)) * cos(glm::radians(cPhi));
	float posY = cRadius * sin(glm::radians(cTheta)) * sin(glm::radians(cPhi));
	float posZ = cRadius * cos(glm::radians(cTheta));

	// Create new camera position
	glm::vec3 position = glm::vec3(posX, posY, posZ);
	float FoV = initialFoV; // - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);

	// Adjust the UP (ESSENTIAL for Continuus vertical Rotation WITHOUT a FLIP)!
	if (cTheta > 180.0f)
	{ // Flip the UP (-Z)
		up = glm::vec3(0, 0, -1);
	}

	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		origin,             // and looks here : at the same position, plus "direction"
		up                  // Look in the z-direction (set to 0,0,1 to look upside-down)
	);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}

// 在主程序初始化时启动输入线程
void startInputThread() {
    inputThread = std::thread(inputThreadFunction);
}

// 在程序结束时清理
void cleanupInputThread() {
    isRunning = false;
    if (inputThread.joinable()) {
        inputThread.join();
    }
}

void moveChessPieceThread(ChessPosition from, ChessPosition to, std::string pieceID, bool isFirstMove) {
	// check if the destination has a chess piece// check if the destination has a chess piece
	std::string destinationPieceID = getComponentIDAtFrom(to);
	if (destinationPieceID != "nan") {
		// move the destination piece out of the board
		movePieceOutOfBoard(to, destinationPieceID);
	}
	// check if the piece is a knight
	if (pieceID == "white_knight1" || pieceID == "white_knight2" || pieceID == "black_knight1" || pieceID == "black_knight2") {
		moveKnightPiece(from, to, pieceID);
	} else {
		moveChessPiece(from, to, pieceID);
	}
	// check if it is a castling move
	if ((pieceID == "white_king" || pieceID == "black_king") &&((from.x - to.x > 1)||(from.x - to.x < -1))) {
		if (from.x - to.x > 0) {
			// queen side castling
			from.x = 0;
			pieceID = getComponentIDAtFrom(from);
			to.x = 3;
			moveChessPiece(from, to, pieceID);
		} else {
			// king side castling
			from.x = 7;
			pieceID = getComponentIDAtFrom(from);
			to.x = 5;
			moveChessPiece(from, to, pieceID);
		}
	}

	if (isFirstMove) {
        // 第一个移动完成后通知等待的线程
        firstMoveComplete = true;
        moveCondition.notify_one();
    }
}

void moveKnightPiece(const ChessPosition& from, const ChessPosition& to, std::string pieceID) {
	// std::cout << "Moving piece from " << from.x << "," << from.y << " to " << to.x << "," << to.y << std::endl;
    // std::cout << "pieceID: " << pieceID << std::endl;

    // 获取起点和终点的3D坐标
    glm::vec3 startPos = chessPositionToTPos(from);
    glm::vec3 endPos = chessPositionToTPos(to);
    
    // 动画参数
    const float animationDuration = 2.0f; // 动画持续2秒
    const int frames = 60; // 60帧动画
    const float frameTime = animationDuration / frames; // 每帧时间
    
    // 执行动画
    for (int frame = 0; frame <= frames; frame++) {
        float t = static_cast<float>(frame) / frames; // 插值参数 (0.0 到 1.0)
        
        // 使用平滑的缓动函数
        t = t * t * (3 - 2 * t); // 使用平滑插值
        
        // 计算当前位置
        glm::vec3 currentPos = startPos * (1.0f - t) + endPos * t;
        // 添加一个抛物线运动
        currentPos.y += sin(t * 3.14159f) * 0.5f; // 添加垂直运动
		currentPos.z -= sin(t * 3.14159f) * 2 * PHEIGHT; // 添加垂直运动
        
        // 更新棋子位置
        for (auto& pieces : cTModelMap) {
            std::vector<tPosition>& cTPositions = pieces.second;
            for (auto& cTPosition : cTPositions) {
                if (cTPosition.nameIdentifier == pieceID) {
                    cTPosition.tPos = currentPos;
                }
            }
        }
        
        // 等待一帧的时间
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frameTime * 1000)));
    }
    
    // 确保最终位置精确
    for (auto& pieces : cTModelMap) {
        std::vector<tPosition>& cTPositions = pieces.second;
        for (auto& cTPosition : cTPositions) {
            if (cTPosition.nameIdentifier == pieceID) {
                cTPosition.tPos = endPos;
            }
        }
    }
}

void moveChessPiece(const ChessPosition& from, const ChessPosition& to, std::string pieceID) {
    // std::cout << "Moving piece from " << from.x << "," << from.y << " to " << to.x << "," << to.y << std::endl;
    // std::cout << "pieceID: " << pieceID << std::endl;

    // 获取起点和终点的3D坐标
    glm::vec3 startPos = chessPositionToTPos(from);
    glm::vec3 endPos = chessPositionToTPos(to);
    
    // 动画参数
    const float animationDuration = 2.0f; // 动画持续2秒
    const int frames = 60; // 60帧动画
    const float frameTime = animationDuration / frames; // 每帧时间
    
    // 执行动画
    for (int frame = 0; frame <= frames; frame++) {
        float t = static_cast<float>(frame) / frames;
        t = t * t * (3 - 2 * t);
        
        glm::vec3 currentPos = startPos * (1.0f - t) + endPos * t;
        currentPos.y += sin(t * 3.14159f) * 0.5f;
        
        // 添加锁保护
        {
            for (auto& pieces : cTModelMap) {
                std::vector<tPosition>& cTPositions = pieces.second;
                for (auto& cTPosition : cTPositions) {
                    if (cTPosition.nameIdentifier == pieceID) {
                        cTPosition.tPos = currentPos;
                    }
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frameTime * 1000)));
    }
    
    // 最终位置也需要加锁
    {
        
        for (auto& pieces : cTModelMap) {
            std::vector<tPosition>& cTPositions = pieces.second;
            for (auto& cTPosition : cTPositions) {
                if (cTPosition.nameIdentifier == pieceID) {
                    cTPosition.tPos = endPos;
                }
            }
        }
    }
}

// input: current move
// output: fen string
std::string getCurrentFen(const std::string& move) {
	std::string fen = "position startpos moves ";
	for (const auto& m : moveHistory) {
		fen += m + " ";
	}
	fen += move;	
	return fen;
}

void movePieceOutOfBoard(const ChessPosition& to, std::string pieceID) {
    static int blackOffsetX = 0;  // 用于记录黑棋的横向偏移
    static int blackOffsetY = 0;  // 用于记录黑棋的纵向偏移
    
    for (auto& pieces : cTModelMap) {
        std::vector<tPosition>& cTPositions = pieces.second;
        for (auto& cTPosition : cTPositions) {
            if (cTPosition.nameIdentifier == pieceID) {
                if (pieceID.find("white") != std::string::npos) {
                    // 白棋逻辑保持不变
                    ChessPosition chessPos = whiteCapturePos;
                    while (getComponentIDAtFrom(chessPos) != "nan") {
                        chessPos.y -= 1;
                        if (chessPos.y < 0) {
                            chessPos.y = 4;
                            chessPos.x -= 1;
                        }
                    }
                    cTPosition.tPos = chessPositionToTPos(chessPos);
                    whiteCapturePos = chessPos;
                } else {
                    // 黑棋新逻辑
                    ChessPosition chessPos;
                    chessPos.x = 9 + blackOffsetX;  // 从基准位置9开始
                    chessPos.y = 4 - blackOffsetY;  // 从基准位置4开始
                    
                    // 更新偏移量
                    blackOffsetY++;
                    if (blackOffsetY > 4) {  // 每列最多放5个棋子
                        blackOffsetY = 0;
                        blackOffsetX++;  // 移到下一列
                    }
                    
                    // std::cout << "Moving black piece to position: " << chessPos.x << "," << chessPos.y << std::endl;
                    
                    // 设置新位置
                    cTPosition.tPos = chessPositionToTPos(chessPos);
                    blackCapturePos = chessPos;
                }
                break;
            }
        }
    }
}
