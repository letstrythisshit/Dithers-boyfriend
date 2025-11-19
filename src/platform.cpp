#include "platform.h"
#include <iostream>
#include <cstring>

#ifndef _WIN32
#include <cstdio>
#endif

namespace Platform {

std::string openFileDialog() {
    std::string filename;

#ifdef _WIN32
    // Windows native file dialog
    char filepath[512] = {0};
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = filepath;
    ofn.nMaxFile = sizeof(filepath);
    ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.tiff;*.webp\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn)) {
        filename = filepath;
    }
#else
    // Unix-like systems (Linux/macOS)

    // Try zenity (GTK-based, most common on Linux)
    FILE* pipe = popen("zenity --file-selection --title='Select Image' --file-filter='Images | *.png *.jpg *.jpeg *.bmp *.tiff *.webp' 2>/dev/null", "r");
    if (pipe) {
        char buffer[512];
        if (fgets(buffer, sizeof(buffer), pipe)) {
            filename = buffer;
            // Remove trailing newline
            if (!filename.empty() && filename.back() == '\n') {
                filename.pop_back();
            }
        }
        pclose(pipe);
        if (!filename.empty()) return filename;
    }

    // Try kdialog (KDE)
    pipe = popen("kdialog --getopenfilename ~ 'Images (*.png *.jpg *.jpeg *.bmp *.tiff *.webp)' 2>/dev/null", "r");
    if (pipe) {
        char buffer[512];
        if (fgets(buffer, sizeof(buffer), pipe)) {
            filename = buffer;
            if (!filename.empty() && filename.back() == '\n') {
                filename.pop_back();
            }
        }
        pclose(pipe);
        if (!filename.empty()) return filename;
    }

    // Try macOS osascript
    pipe = popen("osascript -e 'POSIX path of (choose file of type {\"public.image\"} with prompt \"Select Image\")' 2>/dev/null", "r");
    if (pipe) {
        char buffer[512];
        if (fgets(buffer, sizeof(buffer), pipe)) {
            filename = buffer;
            if (!filename.empty() && filename.back() == '\n') {
                filename.pop_back();
            }
        }
        pclose(pipe);
        if (!filename.empty()) return filename;
    }

    // Fallback: terminal input
    std::cout << "\n=== File Selection ===" << std::endl;
    std::cout << "Enter image path: ";
    std::getline(std::cin, filename);
#endif

    return filename;
}

std::string saveFileDialog() {
    std::string filename;

#ifdef _WIN32
    // Windows native save dialog
    char filepath[512] = {0};
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = filepath;
    ofn.nMaxFile = sizeof(filepath);
    ofn.lpstrFilter = "PNG Image\0*.png\0JPEG Image\0*.jpg\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = "png";
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    if (GetSaveFileNameA(&ofn)) {
        filename = filepath;
    }
#else
    // Unix-like systems (Linux/macOS)

    // Try zenity (GTK-based)
    FILE* pipe = popen("zenity --file-selection --save --confirm-overwrite --title='Save Image' --file-filter='PNG | *.png' --file-filter='JPEG | *.jpg' 2>/dev/null", "r");
    if (pipe) {
        char buffer[512];
        if (fgets(buffer, sizeof(buffer), pipe)) {
            filename = buffer;
            if (!filename.empty() && filename.back() == '\n') {
                filename.pop_back();
            }
        }
        pclose(pipe);
        if (!filename.empty()) return filename;
    }

    // Try kdialog (KDE)
    pipe = popen("kdialog --getsavefilename ~ '*.png *.jpg | Image Files' 2>/dev/null", "r");
    if (pipe) {
        char buffer[512];
        if (fgets(buffer, sizeof(buffer), pipe)) {
            filename = buffer;
            if (!filename.empty() && filename.back() == '\n') {
                filename.pop_back();
            }
        }
        pclose(pipe);
        if (!filename.empty()) return filename;
    }

    // Try macOS osascript
    pipe = popen("osascript -e 'POSIX path of (choose file name with prompt \"Save Image As\" default name \"output.png\")' 2>/dev/null", "r");
    if (pipe) {
        char buffer[512];
        if (fgets(buffer, sizeof(buffer), pipe)) {
            filename = buffer;
            if (!filename.empty() && filename.back() == '\n') {
                filename.pop_back();
            }
        }
        pclose(pipe);
        if (!filename.empty()) return filename;
    }

    // Fallback: terminal input
    std::cout << "\n=== Save File ===" << std::endl;
    std::cout << "Enter output path (e.g., output.png): ";
    std::getline(std::cin, filename);
#endif

    return filename;
}

} // namespace Platform
