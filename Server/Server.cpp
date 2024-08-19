#include <windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <fstream>

const char* SHARED_MEMORY_NAME = "Global\\SharedMemoryExample";
const char* MUTEX_NAME = "Global\\SharedMemoryMutex";
const size_t SHARED_MEMORY_SIZE = 256 * 1024;

struct SharedMemoryHeader {
    bool isWriting;
    size_t dataSize;
    char fileName[256];
};

void handleClient(HANDLE hMapFile, HANDLE hMutex) {
    auto pBuf = static_cast<SharedMemoryHeader*>(MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEMORY_SIZE));
    if (pBuf == NULL) {
        std::cerr << "Could not map view of file: " << GetLastError() << std::endl;
        return;
    }

    while (true) {
        WaitForSingleObject(hMutex, INFINITE);

        if (pBuf->isWriting) {
            std::cout << "Received data from client: " << std::string((char*)(pBuf + 1), pBuf->dataSize) << std::endl;
            std::cout << "Filename: " << pBuf->fileName << std::endl;
            std::string fileName = pBuf->fileName;
            std::ofstream file(fileName, std::ios::binary | std::ios::app);
            if (file.is_open()) {
                file.write((char*)(pBuf + 1), pBuf->dataSize);
                file.close();
            }

            pBuf->isWriting = false;
        }

        ReleaseMutex(hMutex);
        Sleep(10);
    }

    UnmapViewOfFile(pBuf);
}

int main() {
    std::cout << "Starting server..." << std::endl;
    HANDLE hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SHARED_MEMORY_SIZE, SHARED_MEMORY_NAME);
    if (hMapFile == NULL) {
        std::cerr << "Could not create file mapping: " << GetLastError() << std::endl;
        return 1;
    }

    HANDLE hMutex = CreateMutexA(NULL, FALSE, MUTEX_NAME);
    if (hMutex == NULL) {
        std::cerr << "Could not create mutex: " << GetLastError() << std::endl;
        CloseHandle(hMapFile);
        return 1;
    }
    std::vector<std::thread> clients;
    for (int i = 0; i < 5; ++i) { // Поддержка 5 клиентов
        clients.emplace_back(handleClient, hMapFile, hMutex);
    }

    for (auto& client : clients) {
        client.join();
    }
    CloseHandle(hMapFile);
    CloseHandle(hMutex);

    return 0;
}
