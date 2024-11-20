#ifndef ECE_CHESSENGINE_HPP
#define ECE_CHESSENGINE_HPP

#include <string>
#include <vector>
#include <iostream>
#include <windows.h>

class ECE_ChessEngine {
private:
    HANDLE hOutputRead, hOutputWrite;
    HANDLE hInputRead, hInputWrite;
public:
    ECE_ChessEngine();
    ~ECE_ChessEngine();
    bool InitializeEngine(std::string enginePath);
    bool SendMove(const std::string& command);
    std::string ReadFromEngine();
    bool getResponseMove(std::vector<std::string>& tokens, std::string& bestMove);
};

#endif // ECE_CHESSENGINE_HPP