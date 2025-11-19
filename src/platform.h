#pragma once

#include <string>

// Platform-specific includes
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <commdlg.h>
    #undef min
    #undef max
#endif

namespace Platform {
    // Open file dialog - returns selected filepath or empty string
    std::string openFileDialog();

    // Save file dialog - returns selected filepath or empty string
    std::string saveFileDialog();
}
