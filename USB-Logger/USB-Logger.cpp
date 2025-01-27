#include <iostream>
#include <windows.h>
#include <dbt.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <thread>
#include <chrono>
#include <vector>
#include <ctime>

namespace fs = std::filesystem;

// Mutex for thread-safe logging
std::mutex log_mutex;

// Function to get current timestamp as string
std::string current_timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm;
    localtime_s(&local_tm, &now_time_t);
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// Function to log messages with timestamp
void log_message(const std::string& message) {
    std::lock_guard<std::mutex> guard(log_mutex);
    std::cout << "[" << current_timestamp() << "] " << message << std::endl;
}

// Get the directory where the executable is located
std::string get_exe_directory() {
    char path[MAX_PATH];
    if (GetModuleFileNameA(NULL, path, MAX_PATH) == 0) {
        log_message("Error retrieving executable path");
        return "";
    }
    std::string exe_path(path);
    size_t pos = exe_path.find_last_of("\\/");
    return (pos != std::string::npos) ? exe_path.substr(0, pos) : "";
}

// Sanitize the filename by replacing illegal characters with underscores
std::string sanitize_filename(const std::string& filename) {
    std::string sanitized = filename;
    const std::string illegal_chars = "\\/:?\"<>|";
    for (char& c : sanitized) {
        if (illegal_chars.find(c) != std::string::npos) {
            c = '_';
        }
    }
    return sanitized;
}

// Save the directory tree and check for suspicious files
void save_tree_and_check_files(const std::string& path, const std::string& output_file, std::vector<std::string> suspicious_extensions, int level = 0) {
    try {
        std::ofstream ofs(output_file, std::ios::app);
        if (!ofs.is_open()) {
            log_message("Error opening file to save structure: " + output_file);
            return;
        }

        for (const auto& entry : fs::directory_iterator(path)) {
            ofs << std::string(level * 4, ' ');
            ofs << (entry.is_directory() ? "[Folder] " : "[File] ") << entry.path().filename().string();

            if (entry.is_regular_file()) {
                std::string file_ext = entry.path().extension().string();
                // Convert extension to lowercase for case-insensitive comparison
                std::transform(file_ext.begin(), file_ext.end(), file_ext.begin(), ::tolower);
                if (std::find(suspicious_extensions.begin(), suspicious_extensions.end(), file_ext) != suspicious_extensions.end()) {
                    ofs << " [SUSPICIOUS]";
                    log_message("Suspicious file detected: " + entry.path().string());
                }
            }
            ofs << std::endl;

            if (entry.is_directory()) {
                save_tree_and_check_files(entry.path().string(), output_file, suspicious_extensions, level + 1);
            }
        }
    }
    catch (const std::exception& e) {
        log_message(std::string("Exception in save_tree_and_check_files: ") + e.what());
    }
}

// Check the connected USB drive
void check_usb_drive(const std::string& drive_path) {
    std::string exe_dir = get_exe_directory();
    if (exe_dir.empty()) {
        log_message("Executable directory not found.");
        return;
    }

    // Create structures directory if it does not exist
    std::string structures_dir = exe_dir + "\\Structures";
    try {
        if (!fs::exists(structures_dir)) {
            fs::create_directory(structures_dir);
            log_message("Created directory: " + structures_dir);
        }
    }
    catch (const fs::filesystem_error& e) {
        log_message(std::string("Filesystem error: ") + e.what());
        return;
    }

    char volume_name[MAX_PATH + 1] = { 0 };
    DWORD serial_number = 0;
    // Get volume information of the drive
    if (!GetVolumeInformationA(
        drive_path.c_str(),
        volume_name,
        sizeof(volume_name),
        &serial_number,
        NULL,
        NULL,
        NULL,
        0
    )) {
        log_message("Error retrieving volume information for drive: " + drive_path);
        return;
    }

    // Create a sanitized filename based on volume name and serial number
    std::ostringstream filename_stream;
    filename_stream << sanitize_filename(volume_name) << "-" << std::hex << std::uppercase << serial_number << ".txt";
    std::string output_file = structures_dir + "\\" + filename_stream.str();

    // Write initial USB information to the output file
    try {
        std::ofstream ofs(output_file, std::ios::trunc);
        if (ofs.is_open()) {
            ofs << "USB Drive: " << drive_path << std::endl;
            ofs << "Volume Name: " << volume_name << std::endl;
            ofs << "Serial Number: " << std::hex << std::uppercase << serial_number << std::endl << std::endl;
            ofs.close();
            log_message("Created log file: " + output_file);
        }
        else {
            log_message("Failed to create log file: " + output_file);
            return;
        }
    }
    catch (const std::exception& e) {
        log_message(std::string("Exception while creating log file: ") + e.what());
        return;
    }

    // Define suspicious file extensions
    std::vector<std::string> suspicious_extensions = { ".exe", ".bat", ".scr" };

    // Save the directory tree and check for suspicious files
    save_tree_and_check_files(drive_path, output_file, suspicious_extensions);
    log_message("Folder structure saved to " + output_file);
}

// Window procedure to handle device change messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DEVICECHANGE) {
        if (wParam == DBT_DEVICEARRIVAL) {
            PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
            if (pHdr->dbch_devicetype == DBT_DEVTYP_VOLUME) {
                PDEV_BROADCAST_VOLUME pVol = (PDEV_BROADCAST_VOLUME)pHdr;
                char driveLetter = 'A';
                DWORD unitmask = pVol->dbcv_unitmask;
                // Find the drive letter from the unit mask
                while (!(unitmask & 0x1)) {
                    unitmask >>= 1;
                    driveLetter++;
                    if (driveLetter > 'Z') break; // Prevent overflow
                }
                if (driveLetter > 'Z') {
                    log_message("Invalid drive letter detected.");
                    return DefWindowProc(hwnd, uMsg, wParam, lParam);
                }
                std::string drive_path = std::string(1, driveLetter) + ":\\";
                log_message("USB Drive detected: " + drive_path);

                // Launch USB check in a separate thread
                std::thread usb_thread(check_usb_drive, drive_path);
                usb_thread.detach();
            }
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Main function to set up the window and message loop
int main() {
    // Initialize COM library for multithreading (optional, if needed)
    // CoInitialize(NULL);

    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (hInstance == NULL) {
        log_message("Failed to get module handle.");
        return 1;
    }

    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "USBWatcherClass";
    if (!RegisterClassA(&wc)) {
        log_message("Failed to register window class.");
        return 1;
    }

    // Create a hidden window to receive device change messages
    HWND hwnd = CreateWindowA(
        "USBWatcherClass",
        "USB Watcher",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        log_message("Window could not be created.");
        return 1;
    }

    // Hide the window as it's not needed to be visible
    ShowWindow(hwnd, SW_HIDE);
    log_message("USB Watcher started and running in the background.");

    // Message loop to keep the application running
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup COM library (if initialized)
    // CoUninitialize();

    return 0;
}
