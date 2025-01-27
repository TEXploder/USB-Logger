# USB Logger

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration](#configuration)
- [Code Structure](#code-structure)
- [License](#license)
- [Contact](#contact)

## Overview

**USB Logger** is a C++ application designed to monitor USB device insertions on Windows systems. Upon detecting a new USB drive, the program logs the directory structure and identifies suspicious files (such as `.exe`, `.bat`, and `.scr` files). The application runs in the background, ensuring minimal disruption while providing robust monitoring capabilities.

## Features

- **Real-Time USB Monitoring:** Detects when a USB drive is connected to the system.
- **Directory Structure Logging:** Recursively scans and logs the entire directory structure of the connected USB drive.
- **Suspicious File Detection:** Identifies and logs potentially malicious files based on their extensions.
- **Threaded Operations:** Performs scanning in separate threads to maintain application responsiveness.
- **Timestamped Logging:** All logs are timestamped for accurate tracking.
- **Thread-Safe Logging:** Ensures logs are written safely in multi-threaded environments.
- **Configurable Extensions:** Easily modify the list of file extensions considered suspicious.
- **Error Handling:** Comprehensive error messages and exception handling for robust performance.

## Installation

### Prerequisites

- **Operating System:** Windows 10 or later
- **Compiler:** Microsoft Visual Studio 2019 or later (with C++ support)
- **C++ Standard:** C++17 or later

### Steps

1. **Clone the Repository**

   ```bash
   git clone https://github.com/yourusername/usb-logger.git
   cd usb-logger
   ```

2. **Open the Project**

   Open the `usb-logger` project in Visual Studio.

3. **Build the Project**

   - Select the desired build configuration (`Debug` or `Release`).
   - Build the solution by pressing `Ctrl + Shift + B` or navigating to `Build > Build Solution`.

4. **Run the Application**

   - After a successful build, navigate to the output directory (e.g., `bin/Release`).
   - Execute the `USBLogger.exe` file.

## Usage

Once the application is running, it operates silently in the background. To test its functionality:

1. **Insert a USB Drive:**

   - Plug a USB drive into your computer.
   - The application will detect the insertion and begin scanning the drive.

2. **Check Logs:**

   - Logs are saved in the `Structures` directory located in the same directory as the executable.
   - Each USB drive will have a unique log file named based on its volume name and serial number, e.g., `MyUSB-1A2B3C4D.txt`.

3. **Review Suspicious Files:**

   - Suspicious files are marked with `[SUSPICIOUS]` in the log file.
   - Additionally, these detections are printed to the console with timestamps.

### Example Log Entry

```plaintext
[2025-01-27 10:15:30] USB Drive detected: E:\
[2025-01-27 10:15:31] Created log file: C:\Path\To\USBLogger\Structures\MyUSB-1A2B3C4D.txt
[2025-01-27 10:15:31] Folder structure saved to C:\Path\To\USBLogger\Structures\MyUSB-1A2B3C4D.txt
    [Folder] Documents
        [File] report.docx
    [Folder] Executables
        [File] setup.exe [SUSPICIOUS]
        [File] installer.bat [SUSPICIOUS]
```

## Configuration

### Suspicious File Extensions

The application currently flags `.exe`, `.bat`, and `.scr` files as suspicious. To modify this list:

1. Open the `main.cpp` file in your preferred code editor.
2. Locate the `suspicious_extensions` vector:

   ```cpp
   std::vector<std::string> suspicious_extensions = { ".exe", ".bat", ".scr" };
   ```

3. Add or remove extensions as needed:

   ```cpp
   std::vector<std::string> suspicious_extensions = { ".exe", ".bat", ".scr", ".dll" };
   ```

4. Rebuild the application to apply changes.

### Log Directory

By default, logs are saved in a `Structures` folder within the executable's directory. To change the log directory:

1. Open the `main.cpp` file.
2. Locate the `check_usb_drive` function and modify the `structures_dir` path:

   ```cpp
   std::string structures_dir = exe_dir + "\\YourDesiredDirectory";
   ```

3. Rebuild the application.

## Code Structure

### Main Components

- **Logging Mechanism:**

  Handles thread-safe logging with timestamps.

  ```cpp
  std::mutex log_mutex;

  std::string current_timestamp() {
      // Generates current timestamp
  }

  void log_message(const std::string& message) {
      std::lock_guard<std::mutex> guard(log_mutex);
      std::cout << "[" << current_timestamp() << "] " << message << std::endl;
  }
  ```

- **USB Detection:**

  Utilizes Windows API to detect USB device insertions.

  ```cpp
  LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
      if (uMsg == WM_DEVICECHANGE && wParam == DBT_DEVICEARRIVAL) {
          // Handle USB arrival
      }
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
  ```

- **Directory Scanning:**

  Recursively scans USB directories and logs structure and suspicious files.

  ```cpp
  void save_tree_and_check_files(const std::string& path, const std::string& output_file, std::vector<std::string> suspicious_extensions, int level = 0) {
      // Recursive directory scanning
  }
  ```

- **Multithreading:**

  Ensures scanning runs in separate threads to maintain responsiveness.

  ```cpp
  std::thread usb_thread(check_usb_drive, drive_path);
  usb_thread.detach();
  ```

## License

This project is licensed under the [MIT License](https://opensource.org/licenses/MIT). See the [LICENSE](https://github.com/texploder/usb-logger/LICENSE) file for details.

## Contact

For any inquiries or support, please contact:

- **Name:** TEXploder
- **Email:** business@texploder.com
- **GitHub:** [yourusername](https://github.com/texploder)

---

*Disclaimer: This application is intended for monitoring USB devices for legitimate purposes. Ensure you have appropriate permissions to use this software, especially in environments with sensitive data.*
