#include <iostream>
#include <string>
#include <windows.h>

HANDLE hInputWrite, hInputRead;
HANDLE hOutputWrite, hOutputRead;

void StartEngine() {
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
    std::string enginePath = "dragon-64bit.exe";
    if (!CreateProcess(NULL, const_cast<char*>(enginePath.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        std::cerr << "Failed to start engine" << std::endl;
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void SendToEngine(const std::string& command) {
    DWORD written;
    WriteFile(hInputWrite, command.c_str(), command.length(), &written, NULL);
    WriteFile(hInputWrite, "\n", 1, &written, NULL);
}

std::string ReadFromEngine() {
    char buffer[4096];
    DWORD read;
    std::string output;
    if (ReadFile(hOutputRead, buffer, sizeof(buffer) - 1, &read, NULL) && read > 0) {
        buffer[read] = '\0';
        output = buffer;
    }
    return output;
}

int main() {
    StartEngine();

    SendToEngine("uci");
    std::cout << "Engine Response: " << ReadFromEngine() << std::endl;

    SendToEngine("isready");
    std::cout << "Engine Response: " << ReadFromEngine() << std::endl;

    // std::cout << "sending move to engine" << std::endl;
    // // Example: position, followed by best move
    // SendToEngine("position startpos moves e2e4");
    // SendToEngine("go depth 5");

    std::string response;
    // while ((response = ReadFromEngine()).find("bestmove") == std::string::npos) {
    //     std::cout << "Engine Response: " << response << std::endl;
    // }

    std::cout << "Engine best move: " << response << std::endl;
    // Example: position, followed by best move
    SendToEngine("position startpos moves b1c3");
    SendToEngine("go depth 5");

    while ((response = ReadFromEngine()).find("bestmove") == std::string::npos) {
        std::cout << "Engine Response: " << response << std::endl;
    }

    std::cout << "Engine best move: " << response << std::endl;
    return 0;
}
