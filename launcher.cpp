#include <cstdlib>
#include <string>
#include <filesystem>
#include <unistd.h>
#include <limits.h>

int main() {
    // Get the directory of the current executable
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
        std::filesystem::path exePath(std::string(result, (count > 0) ? count : 0));
        std::string dir = exePath.parent_path().string();
        
        // Command to open terminal and run start.sh from the correct directory
        std::string cmd = "gnome-terminal -- bash -c 'cd \"" + dir + "\" && ./start.sh; echo \"[Đã kết thúc] Bạn có thể đóng cửa sổ này.\"; exec bash'";
        system(cmd.c_str());
    }
    return 0;
}
