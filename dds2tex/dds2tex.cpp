/*  Ghostbusters The Video Game DDS to TEX Converter
	Copyright 2024 KeyofBlueS

	The Ghostbusters The Video Game DDS to TEX Converter is free software;
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
#include <vector>

//// For Windows-specific commands
//#ifdef _WIN32
	//#include <windows.h>
//#else
	//#include <sys/types.h>
	//#include <sys/stat.h>
	//#include <unistd.h>
//#endif

bool quiet = false;	// Quiet mode flag
bool forcedxtone = false;	// DXT1 compression mode flag
bool forcedxtfive = false;	// DXT5 compression mode flag

//// Function to change terminal title and print colorful output cross-platform
//void setConsoleTitleAndPrint(const std::string& title, const std::string& message) {
	//if (quiet) return;

//#ifdef _WIN32
	//SetConsoleTitle(title.c_str());
//#else
	//std::cout << "\033]2;" << title << "\007";
//#endif

//#ifdef _WIN32
	//HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	//SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	//std::cout << message << std::endl;
	//SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
//#else
	//std::cout << "\033[1;34m" << message << "\033[0m" << std::endl;
//#endif
//}

// Function to extract bytes from a binary file and return them as a hexadecimal string.
std::string extractBytes(const std::string& filePath, int offset, int length) {
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "* ERROR: Unable to open file: " << filePath << std::endl;
		return "";
	}

	file.seekg(offset, std::ios::beg);
	std::vector<char> buffer(length);
	file.read(buffer.data(), length);
	file.close();

	std::stringstream hexStream;
	for (int i = 0; i < length; ++i) {
		hexStream << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)buffer[i];
	}

	return hexStream.str();
}

// Function to reverse byte order in the hex string
std::string reverseBytes(const std::string& hex) {
	std::string reversed;
	for (int i = hex.length(); i > 0; i -= 2) {
		reversed += hex.substr(i - 2, 2);
	}
	return reversed;
}

// Function to convert a hexadecimal string into a string of bytes.
std::string hexToByteString(const std::string& hex) {
	std::string bytes;
	for (size_t i = 0; i < hex.length(); i += 2) {
		std::string byteString = hex.substr(i, 2);
		char byte = (char) strtol(byteString.c_str(), nullptr, 16);
		bytes += byte;
	}
	return bytes;
}

// Function to add padding to hexdata for writing
void addPadding(std::ofstream& outFile, int count) {
	for (int i = 0; i < count; ++i) {
		outFile.put(0x00);	// Add zero bytes
	}
}

// Function to validate the input file
bool checkFileSignature(const std::string& filePath, const std::string& expectedSignature) {
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "* ERROR: Unable to open file: " << filePath << std::endl;
		return false;
	}

	char buffer[4];
	file.read(buffer, 4);
	file.close();

	std::stringstream hexStream;
	for (int i = 0; i < 4; ++i) {
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
	std::cout << "👻 GBTVGR DDS to TEX Converter v0.0.1\n";
	std::cout << "\n";
	std::cout << "Usage: dds2tex <input_file.dds> [options]\n";
	std::cout << "\n";
	std::cout << "Options:\n";
	std::cout << "  -o, --output <output_file.tex>	Specify the output TEX file path and name\n";
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
		} else if (arg == "--dxt1") {
			forcedxtone = true;
		} else if (arg == "--dxt5") {
			forcedxtfive = true;
		} else if (arg == "-q" || arg == "--quiet") {
			quiet = true;
		} else {
			inputFile = arg;
		}
	}

	if (forcedxtone == true && forcedxtfive == true) {
		std::cerr << "* ERROR: options --dxt1 and --dxt5 cannot be used together!" << std::endl;
		return 1;
	}

	// Validate input file
	if (inputFile.empty()) {
		std::cerr << "* ERROR: No input file specified.\n";
		printHelpMessage();
		return 1;
	}

	// Generate default output file if not provided
	if (outputFile.empty()) {
		outputFile = std::filesystem::path(inputFile).replace_extension(".tex").string();
	}

	std::string pathTo = std::filesystem::path(outputFile).parent_path().string();
	std::string file = std::filesystem::path(outputFile).stem().string();

	std::string input = "dds";
	std::string output = "tex";

	//setConsoleTitleAndPrint("👻 GBTVGR Converter", "👻 GBTVGR DDS to TEX Converter v0.0.1:");

	// Check if the file has a valid DDS header
	if (!checkFileSignature(inputFile, "44445320")) {
		std::cerr << "* ERROR: \"" << inputFile << "\" is not a valid DDS!" << std::endl;
		return 3;
	}

	// Check compression type (DXT1 or DXT5)
	std::string compressionHex = extractBytes(inputFile, 84, 4);
	std::string compressionHexRev;
	if (compressionHex == "44585431") {
		if (forcedxtfive == true) {
			std::cerr << "* ERROR: MUST USE DXT5 COMPRESSION!" << std::endl;
			return 7;
		} else {
			compressionHexRev = "2b0000";
		}
	} else if (compressionHex == "44585435") {
		if (forcedxtone == true) {
			std::cerr << "* ERROR: MUST USE DXT1 COMPRESSION!" << std::endl;
			return 9;
		} else {
			compressionHexRev = "320000";
		}
	} else {
		std::cerr << "* ERROR: MUST USE DXT1 OR DXT5 COMPRESSION!" << std::endl;
		return 5;
	}

	// Extract dimensions
	std::string dimensionsHex = extractBytes(inputFile, 12, 8);
	std::string heightHexRev = reverseBytes(dimensionsHex.substr(0, 8));
	std::string widthHexRev = reverseBytes(dimensionsHex.substr(8, 8));

	// Extract mipmaps
	std::string mipmapsHex = extractBytes(inputFile, 28, 1);
	int mipmapsDec = std::stoi(mipmapsHex, nullptr, 16) - 1;
	std::stringstream mipmapsHexStream;
	mipmapsHexStream << std::setw(8) << std::setfill('0') << std::hex << mipmapsDec;
	std::string mipmapsHexRev = reverseBytes(mipmapsHexStream.str());

	// Create output directory if not exists
	createDirectories(pathTo);

	// Prepare output file
	std::ofstream outFile(outputFile, std::ios::binary);
	if (!outFile.is_open()) {
		std::cerr << "* ERROR: Unable to open output file: " << outputFile << std::endl;
		return 1;
	}

	// Prepare and write the 52 bytes header
	outFile.put(0x07);
	addPadding(outFile, 3);
	outFile.put(0x4b);
	outFile.put(0x65);
	outFile.put(0x79);
	outFile.put(0x6f);
	outFile.put(0x66);
	outFile.put(0x42);
	outFile.put(0x6c);
	outFile.put(0x75);
	outFile.put(0x65);
	outFile.put(0x53);
	addPadding(outFile, 10);
	outFile.write(hexToByteString(compressionHexRev).c_str(), 3);
	outFile.write(hexToByteString(widthHexRev).c_str(), 4);
	outFile.write(hexToByteString(heightHexRev).c_str(), 4);
	addPadding(outFile, 5);
	if (mipmapsHexRev == "ffffffff") {
		addPadding(outFile, 4);
	} else {
		outFile.write(hexToByteString(mipmapsHexRev).c_str(), 4);
	}
	addPadding(outFile, 8);

	// Append remaining content from the input DDS file after 128 bytes
	std::ifstream inFile(inputFile, std::ios::binary);
	if (!inFile.is_open()) {
		std::cerr << "* ERROR: Unable to open input file: " << inputFile << std::endl;
		return 1;
	}

	inFile.seekg(128, std::ios::beg);
	outFile << inFile.rdbuf();

	outFile.close();
	inFile.close();

	if (!quiet) std::cout << "Conversion complete: " << outputFile << std::endl;

	return 0;
}
