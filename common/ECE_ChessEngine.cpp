#include "ECE_ChessEngine.hpp"

ECE_ChessEngine::ECE_ChessEngine() {

}

ECE_ChessEngine::~ECE_ChessEngine() {

}

bool ECE_ChessEngine::InitializeEngine(std::string enginePath) {
    // Create pipes for input and output
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    CreatePipe(&hOutputRead, &hOutputWrite, &sa, 0);
    CreatePipe(&hInputRead, &hInputWrite, &sa, 0);

    // Start the Komodo engine
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hInputRead;
    si.hStdOutput = hOutputWrite;
    si.hStdError = hOutputWrite;

    // Path to Komodo executable
    
    if (!CreateProcess(NULL, const_cast<char*>(enginePath.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        std::cerr << "Failed to start engine" << std::endl;
        return false;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

bool ECE_ChessEngine::SendMove(const std::string& command) {
    DWORD written;
    WriteFile(hInputWrite, command.c_str(), command.length(), &written, NULL);
    WriteFile(hInputWrite, "\n", 1, &written, NULL);

    return true;
}

std::string ECE_ChessEngine::ReadFromEngine() {
    char buffer[4096];
    DWORD read;
    std::string output;
    if (ReadFile(hOutputRead, buffer, sizeof(buffer) - 1, &read, NULL) && read > 0) {
        buffer[read] = '\0';
        output = buffer;
    }
    return output;
}

// input:the response from engine and split it to get the best move
// output: validation of the input move
bool ECE_ChessEngine::getResponseMove(std::vector<std::string>& tokens, std::string& bestMove) {
    if (tokens.back() == "0000") {
        return false;
    }else{
        for( unsigned int i = 0; i < tokens.size(); i++) {
            if (tokens[i] == "bestmove") {  
                bestMove = tokens[i+1];
                break;
            }
        }
    }
    return true;
}
