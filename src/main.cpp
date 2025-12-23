#include <iostream>
#include <string>

#include "io/Instance.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: uflp <instance_path>\n";
        return 1;
    }
    string path = argv[1];

    try {
        Instance instance(path, true);
    } catch (const std::exception& ex) {
        cerr << "Error: " << ex.what() << "\n";
        return 2;
    }
    
    return 0;
}
