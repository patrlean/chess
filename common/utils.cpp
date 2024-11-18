#include "utils.hpp"    


std::string getComponentIDAtFrom(ChessPosition& from, tPosition & cTposition) {
    ChessPosition pos = tPosToChessPosition(cTposition.tPos);
    if (pos.x == from.x && pos.y == from.y) {
        return cTposition.nameIdentifier;
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
    pos.x = (tPos.x - (-3.5f * CHESS_BOX_SIZE))/ CHESS_BOX_SIZE;
    pos.y = (tPos.y - (-3.5f * CHESS_BOX_SIZE)) / CHESS_BOX_SIZE;
    return pos;
}
