#include <windows.h>
#include <iostream>
#include <fstream>
#include <cstring> 


const char* SHARED_MEMORY_NAME = "Global\\SharedMemoryExample";
const char* MUTEX_NAME = "Global\\SharedMemoryMutex";
const size_t SHARED_MEMORY_SIZE = 256 * 1024;

struct SharedMemoryHeader {
    bool isWriting;
    size_t dataSize;
    char fileName[260];  // Размер буфера для имени файла (максимальный размер пути в Windows)
};

void sendFileToServer(const std::string& fileName) {
    HANDLE hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEMORY_NAME);
    if (hMapFile == NULL) {
        std::cerr << "Could not open file mapping: " << GetLastError() << std::endl;
        return;
    }

    HANDLE hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME);
    if (hMutex == NULL) {
        std::cerr << "Could not open mutex: " << GetLastError() << std::endl;
        return;
    }

    auto pBuf = static_cast<SharedMemoryHeader*>(MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEMORY_SIZE));
    if (pBuf == NULL) {
        std::cerr << "Could not map view of file: " << GetLastError() << std::endl;
        return;
    }

    std::ifstream file(fileName, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << fileName << std::endl;
        return;
    }

    while (file) {
        WaitForSingleObject(hMutex, INFINITE);

        if (!pBuf->isWriting) {
            file.read((char*)(pBuf + 1), SHARED_MEMORY_SIZE - sizeof(SharedMemoryHeader));
            pBuf->dataSize = file.gcount();
            pBuf->isWriting = true;
            strcpy_s(pBuf->fileName, fileName.c_str());
            std::cout << "Sent data to server: " << pBuf->dataSize << " bytes." << std::endl;
        }

        ReleaseMutex(hMutex);
        Sleep(10);
    }

    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
    CloseHandle(hMutex);
}

int main() {
    sendFileToServer("example.txt");
    return 0;
}
