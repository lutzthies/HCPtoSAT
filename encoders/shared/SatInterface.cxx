#include <string>
#include <stdlib.h>

#include <memory>

// run cmd and write it to std::to_string
// idea by http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
std::string execToString(const char* cmd) {
        char buffer[512];
        std::string result = "";
        std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
        if (!pipe) throw std::runtime_error("popen() failed!");
        while (!feof(pipe.get())) {
                if (fgets(buffer, 512, pipe.get()) != NULL)
                        result += buffer;
        }
        return result;
}
