/*  Ghostbusters The Video Game SMP to OGG Converter
	Copyright 2024 KeyofBlueS

	The Ghostbusters The Video Game SMP to OGG Converter is free software;
	you can redistribute it and/or modify it under the terms of the
	GNU General Public License as published by the Free Software Foundation;
	either version 3, or (at your option) any later version.
	See the file COPYING for more details.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <filesystem>

bool quiet = false;	// Quiet mode flag

// Function to validate the input file
bool checkFileSignature(const std::string& filePath, const std::string& expectedSignature) {
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "* ERROR: Unable to open file: " << filePath << std::endl;
		return false;
	}

	file.seekg(160, std::ios::beg);
	char buffer[3];
	file.read(buffer, 3);
	file.close();

	std::stringstream hexStream;
	for (int i = 0; i < 3; ++i) {
		hexStream << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)buffer[i];
	}

	return hexStream.str() == expectedSignature;
}

// Function to create output directory
void createDirectories(const std::string& path) {
	std::filesystem::create_directories(path);
}

// Function to print the help message
void printHelpMessage() {
	std::cout << "\n";
	std::cout << "👻 GBTVGR SMP to OGG Converter v0.0.1\n";
	std::cout << "\n";
	std::cout << "Usage: smp2ogg <input_file.smp> [options]\n";
	std::cout << "\n";
	std::cout << "Options:\n";
	std::cout << "  -o, --output <output_file.ogg>	Specify the output OGG file path and name\n";
	std::cout << "  -q, --quiet				Disable output messages\n";
	std::cout << "  -h, --help				Show this help message and exit\n";
	std::cout << "\n";
	std::cout << "\n";
	std::cout << "Copyright © 2024 KeyofBlueS: <https://github.com/KeyofBlueS>.\n";
	std::cout << "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n";
	std::cout << "This is free software: you are free to change and redistribute it.\n";
	std::cout << "There is NO WARRANTY, to the extent permitted by law.\n";
}

// Main function
int main(int argc, char* argv[]) {
	std::string inputFile, outputFile;

	// Parse command-line arguments
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];

		if (arg == "-h" || arg == "--help") {
			printHelpMessage();
			return 0;
		} else if (arg == "-o" || arg == "--output") {
			if (i + 1 < argc) {
				outputFile = argv[++i];	// Get the next argument as the output file
			} else {
				std::cerr << "* ERROR: Missing output file after " << arg << std::endl;
				return 1;
			}
		} else if (arg == "-q" || arg == "--quiet") {
			quiet = true;
		} else {
			inputFile = arg;
		}
	}

	// Validate input file
	if (inputFile.empty()) {
		std::cerr << "* ERROR: No input file specified.\n";
		printHelpMessage();
		return 1;
	}

	// Generate default output file if not provided
	if (outputFile.empty()) {
		outputFile = std::filesystem::path(inputFile).replace_extension(".ogg").string();
	}

	std::string pathTo = std::filesystem::path(outputFile).parent_path().string();
	std::string file = std::filesystem::path(outputFile).stem().string();

	std::string input = "smp";
	std::string output = "ogg";

	//setConsoleTitleAndPrint("👻 GBTVGR Converter", "👻 GBTVGR SMP to OGG Converter v0.0.1:");

	// Check if the file has a valid OGG header
	if (!checkFileSignature(inputFile, "4f6767")) {
		std::cerr << "* ERROR: \"" << inputFile << "\" is not a valid SMP!" << std::endl;
		return 3;
	}

	// Create output directory if not exists
	createDirectories(pathTo);

	// Prepare output file
	std::ofstream outFile(outputFile, std::ios::binary);
	if (!outFile.is_open()) {
		std::cerr << "* ERROR: Unable to open output file: " << outputFile << std::endl;
		return 1;
	}

	// Append the input OGG file
	std::ifstream inFile(inputFile, std::ios::binary);
	if (!inFile.is_open()) {
		std::cerr << "* ERROR: Unable to open input file: " << inputFile << std::endl;
		return 1;
	}
	inFile.seekg(160, std::ios::beg);
	outFile << inFile.rdbuf();

	outFile.close();
	inFile.close();

	if (!quiet) std::cout << "Conversion complete: " << outputFile << std::endl;

	return 0;
}
