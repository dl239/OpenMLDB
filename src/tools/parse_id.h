#include <iostream>
#include <fstream>
#include <vector>

namespace openmldb {
namespace tools {

void FillDeviceID(const std::string& path, std::vector<std::string>* device_id) {
    if (device_id == nullptr) {
        return;
    }
    device_id->clear();
    std::ifstream file;
    file.open(path, std::ios_base::in);
    if (!file.is_open()) {
        printf("open file %s failed\n", path.c_str());
        return;
    }
    std::string str;
    while (getline(file, str)) {
        device_id->push_back(str);        
    }
    file.close();
}
}
}
