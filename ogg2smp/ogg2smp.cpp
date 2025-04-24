/*  Ghostbusters The Video Game OGG to SMP Converter
	Copyright 2025 KeyofBlueS

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
#include <getopt.h>

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
	std::cout << std::endl;
	std::cout << "👻 GBTVGR OGG to SMP Converter v0.2.0" << std::endl;
	std::cout << std::endl;
	std::cout << "Usage: ogg2smp <input_file.ogg> [options]" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -i, --input <input_file.dds>		Specify the input OGG file path and name." << std::endl;
	std::cout << "  -o, --output <output_file.smp>	Specify the output SMP file path and name." << std::endl;
	std::cout << "  -q, --quiet				Disable output messages." << std::endl;
	std::cout << "  -h, --help				Show this help message and exit." << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "Copyright © 2025 KeyofBlueS: <https://github.com/KeyofBlueS>." << std::endl;
	std::cout << "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>." << std::endl;
	std::cout << "This is free software: you are free to change and redistribute it." << std::endl;
	std::cout << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
}

// Main function
int main(int argc, char* argv[]) {

	std::string inputFile;
	std::string outputFile;
	bool argError = false;

	// Define the long options for getopt
	struct option long_options[] = {
		{"input", required_argument, nullptr, 'i'},
		{"output", required_argument, nullptr, 'o'},
		{"quiet", no_argument, nullptr, 'q'},
		{"help", no_argument, nullptr, 'h'},
		{nullptr, 0, nullptr, 0}	// Terminate the list of options
	};

	// Parse command-line arguments
	int opt;
	int option_index = 0;
	while ((opt = getopt_long(argc, argv, "i:o:qh", long_options, &option_index)) != -1) {
		switch (opt) {
			case 'i':
				inputFile = optarg;
				break;
			case 'o':
				outputFile = optarg;
				break;
			case 'q':
				quiet = true;
				break;
			case 'h':
				printHelpMessage();
				return 0;
			case '?':
			default:
				argError = true;
		}
	}

	// Remaining arguments (positional)
	for (int i = optind; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg.rfind("-", 0) == 0) {
			argError = true;
			return 1;
		}
		if (inputFile.empty()) {
			inputFile = arg;
		} else {
			argError = true;
			std::cerr << "* ERROR: Unexpected argument: " << arg << std::endl;
		}
	}

	// Check if input file is provided
	if (inputFile.empty()) {
		argError = true;
		std::cerr << "* ERROR: No input file specified." << std::endl;
	}

	if (argError) {
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
	outFile.write(hexToByteString(duration_hex_rev).c_str(), 4);	// bytes from 24 to 27 (subtitle timing?)
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
