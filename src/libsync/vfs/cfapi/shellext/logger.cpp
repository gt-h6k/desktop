#include "logger.h"
#include <Windows.h>
#include <ctime>
#include <iomanip>
#include <sstream>

#define LOG_FILE_PATH L"D:\\work\\NextcloudTumbProvider.log.txt"

void writeLog(const std::string log)
{
    HANDLE hFile = CreateFile(L"D:\\work\\NextcloudTumbProvider.log.txt", // Filename
        FILE_APPEND_DATA, // Desired access
        FILE_SHARE_READ, // Share mode
        NULL, // Security attributes
        OPEN_ALWAYS, // Creates a new file, only if it doesn't already exist
        FILE_ATTRIBUTE_NORMAL, // Flags and attributes
        NULL); // Template file handle

    // Write data to the file
    DWORD bytesWritten;
    WriteFile(hFile, // Handle to the file
        (log + std::string("\n")).c_str(), // Buffer to write
        log.size() + 1, // Buffer size
        &bytesWritten, // Bytes written
        nullptr); // Overlapped

    // Close the handle once we don't need it.
    CloseHandle(hFile);
}