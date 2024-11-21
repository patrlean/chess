#include "utils.hpp"    

// input: string
// output: vector of strings
// split the string into tokens
std::vector<std::string> splitString(const std::string& input) {
    std::vector<std::string> tokens;
    std::stringstream ss(input);
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}


// input: from position, tPosition
// output: component ID
// get the component ID at the from position
std::string getComponentIDAtFrom(const ChessPosition& from) {
    for (auto& pieces : cTModelMap) {
        std::vector<tPosition> cTPositions = pieces.second;
        for (auto& cTPosition : cTPositions) {
            ChessPosition pos = tPosToChessPosition(cTPosition.tPos);
            if (pos.x == from.x && pos.y == from.y) {
                if (cTPosition.nameIdentifier == "chessBoard") {
                    continue;
                }
                return cTPosition.nameIdentifier;
            }
        }
    }
    return "nan";
}

// 将UCI格式坐标(如"e2")转换为内部坐标系统
ChessPosition uciToPosition(const std::string& uciPos) {
    ChessPosition pos;
    if (uciPos.length() != 2) return {-1, -1};
    
    pos.x = tolower(uciPos[0]) - 'a';
    pos.y = uciPos[1] - '1';
    
    // 验证坐标是否有效
    if (pos.x < 0 || pos.x > 7 || pos.y < 0 || pos.y > 7) {
        return {-1, -1};
    }
    
    return pos;
}

// 将tPos转换为ChessPosition
ChessPosition tPosToChessPosition(glm::vec3 tPos) {
    ChessPosition pos;
    pos.x = (tPos.x - (-3.5f * CHESS_BOX_SIZE)) / CHESS_BOX_SIZE;
    pos.y = (tPos.y - (-3.5f * CHESS_BOX_SIZE)) / CHESS_BOX_SIZE;
    return pos;
}

// input: chess position in integer
// output: tPos
glm::vec3 chessPositionToTPos(ChessPosition chessPos) {
    return glm::vec3((chessPos.x * CHESS_BOX_SIZE) + (-3.5f * CHESS_BOX_SIZE), (chessPos.y * CHESS_BOX_SIZE) + (-3.5f * CHESS_BOX_SIZE), PHEIGHT);
}
