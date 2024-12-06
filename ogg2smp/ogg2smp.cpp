/*  Ghostbusters The Video Game OGG to SMP Converter
	Copyright 2024 KeyofBlueS

	The Ghostbusters The Video Game OGG to SMP Converter is free software;
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
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <algorithm>

bool quiet = false;	// Quiet mode flag

// Function to get the file size
std::streamsize getFileSize(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	return file.tellg();
}

// Function to convert integer to hex string
std::string intToHex(uint32_t num) {
	std::stringstream ss;
	ss << std::hex << std::setw(8) << std::setfill('0') << num;
	return ss.str();
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

// Function to get the duration of an Ogg file in milliseconds
long long getOggFileDurationMilliseconds(const char* filename) {
	OggVorbis_File vf;
	FILE* file = fopen(filename, "rb");
	
	if (!file) {
		std::cerr << "Error opening file: " << filename << std::endl;
		return -1;
	}

	if (ov_open(file, &vf, NULL, 0) < 0) {
		std::cerr << "Error reading Ogg file: " << filename << std::endl;
		fclose(file); 	// Ensure file is closed if opening fails
		return -1;
	}

	// Get the total number of samples in the file
	long totalSamples = ov_pcm_total(&vf, -1);

	// Get the sample rate of the first stream
	long sampleRate = ov_info(&vf, -1)->rate;

	// Clean up the OggVorbis_File structure
	ov_clear(&vf);

	// Convert total samples to milliseconds
	long long duration_millis = (totalSamples * 1000) / sampleRate;

	return duration_millis;
}

// Function to add padding
std::string processDuration(long long duration_millis) {
	// Add 275 ms
	duration_millis += 275;

	// Multiply by 44
	long long durationScaled = duration_millis * 44;

	// Convert to hex

	std::string duration_hex = intToHex(static_cast<uint32_t>(durationScaled));

	// Add padding to the hex string
	int hexSize = duration_hex.size();
	std::string durationHexPadded;

	if (hexSize == 1) {
		duration_hex = "000" + duration_hex + "00";
	} else if (hexSize == 2) {
		duration_hex = "00" + duration_hex + "00";
	} else if (hexSize == 3) {
		duration_hex = "0" + duration_hex + "00";
	} else if (hexSize == 4) {
		duration_hex = duration_hex + "00";
	} else if (hexSize == 5) {
		duration_hex = "0" + duration_hex;
	} else if (hexSize == 6) {
		duration_hex = duration_hex;
	}

	return duration_hex;
}

// Function to validate the input file
bool checkFileSignature(const std::string& filePath, const std::string& expectedSignature) {
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "* ERROR: Unable to open file: " << filePath << std::endl;
		return false;
	}

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
	std::cout << "👻 GBTVGR OGG to SMP Converter v0.1.0\n";
	std::cout << "\n";
	std::cout << "Usage: ogg2smp <input_file.ogg> [options]\n";
	std::cout << "\n";
	std::cout << "Options:\n";
	std::cout << "  -o, --output <output_file.smp>	Specify the output SMP file path and name\n";
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
		outputFile = std::filesystem::path(inputFile).replace_extension(".smp").string();
	}

	std::string pathTo = std::filesystem::path(outputFile).parent_path().string();
	std::string file = std::filesystem::path(outputFile).stem().string();

	std::string input = "ogg";
	std::string output = "smp";

	//setConsoleTitleAndPrint("👻 GBTVGR Converter", "👻 GBTVGR OGG to SMP Converter v0.0.1:");

	// Check if the file has a valid OGG header
	if (!checkFileSignature(inputFile, "4f6767")) {
		std::cerr << "* ERROR: \"" << inputFile << "\" is not a valid OGG!" << std::endl;
		return 3;
	}

	// Get file size in bytes, convert to hex and reverse the bytes
	std::streamsize dimension_bytes = getFileSize(inputFile);
	std::string dimension_hex = intToHex(static_cast<uint32_t>(dimension_bytes));
	std::string dimension_hex_rev = reverseBytes(dimension_hex);

	// Get duration in milliseconds, convert to hex and reverse the bytes
	long long duration_millis = getOggFileDurationMilliseconds(inputFile.c_str());
	std::string duration_hex = processDuration(duration_millis);
	std::string duration_hex_rev = reverseBytes(duration_hex);
	duration_hex_rev += "00";
	
	// Create output directory if not exists
	createDirectories(pathTo);

	// Prepare output file
	std::ofstream outFile(outputFile, std::ios::binary);
	if (!outFile.is_open()) {
		std::cerr << "* ERROR: Unable to open output file: " << outputFile << std::endl;
		return 1;
	}

	// Prepare and write the 160 bytes header
	outFile.put(0x06);			// byte 0
	addPadding(outFile, 3);		// bytes from 1 to 3
	outFile.put(0x4b);			// byte 4
	outFile.put(0x65);			// byte 5
	outFile.put(0x79);			// byte 6
	outFile.put(0x6f);			// byte 7
	outFile.put(0x66);			// byte 8
	outFile.put(0x42);			// byte 9
	outFile.put(0x6c);			// byte 10
	outFile.put(0x75);			// byte 11
	outFile.put(0x65);			// byte 12
	outFile.put(0x53);			// byte 13
	addPadding(outFile, 10);	// bytes from 14 to 23
	outFile.write(hexToByteString(duration_hex_rev).c_str(), 4);	// bytes from 32 to 35 (subtitle timing?)
	outFile.put(0xa0);			// byte 28
	addPadding(outFile, 3);		// bytes from 29 to 31
	outFile.write(hexToByteString(dimension_hex_rev).c_str(), 4);	// bytes from 32 to 35 (.ogg file size in bytes)
	outFile.put(0x09);			// byte 36
	addPadding(outFile, 7);		// bytes from 37 to 43 (byte 40???)
	outFile.put(0x10);			// byte 44
	addPadding(outFile, 3);		// bytes from 45 to 47 byte
	outFile.put(0x44);			// byte 48 (lip-sync animation?)
	outFile.put(0xac);			// byte 49 (lip-sync animation?)
	addPadding(outFile, 110);	// bytes from 50 to 160

	// Append the input OGG file
	std::ifstream inFile(inputFile, std::ios::binary);
	if (!inFile.is_open()) {
		std::cerr << "* ERROR: Unable to open input file: " << inputFile << std::endl;
		return 1;
	}
	outFile << inFile.rdbuf();

	outFile.close();
	inFile.close();

	if (!quiet) std::cout << "Conversion complete: " << outputFile << std::endl;

	return 0;
}
