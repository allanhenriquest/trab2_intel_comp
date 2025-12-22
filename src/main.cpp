#include <iostream>
#include <string>

#include "core/Instance.h"
#include "core/InstanceMEDParser.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: uflp <instance_path>\n";
        return 1;
    }
    std::string path = argv[1];

    try {
        InstanceMEDParser parser;
        Instance instance = parser.parseFile(path);
        std::cout << "Loaded instance: n=" << instance.n << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 2;
    }

    std::cout << "Project scaffold OK.\n";
    return 0;
}
