#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./convert_in_place input.csv\n";
        return 1;
    }

    std::ifstream infile(argv[1]);
    if (!infile.is_open()) {
        std::cerr << "Error: Cannot open input file.\n";
        return 1;
    }

    std::vector<std::string> converted_lines;
    std::string line;

    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string u_str, v_str;

        if (std::getline(ss, u_str, ',') && std::getline(ss, v_str)) {
            converted_lines.push_back(u_str + " " + v_str);
        }
    }
    infile.close();

    std::ofstream outfile(argv[1], std::ios::trunc); // Overwrite original file
    if (!outfile.is_open()) {
        std::cerr << "Error: Cannot reopen file for writing.\n";
        return 1;
    }

    for (const auto& converted_line : converted_lines) {
        outfile << converted_line << "\n";
    }
    outfile.close();

    std::cout << "Conversion complete: " << argv[1] << "\n";
    return 0;
}
