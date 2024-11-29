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
#include <filesystem>
#include <vector>
#include <sstream>
#include <iomanip>

bool quiet = false;	// Quiet mode flag
bool forcedxtone = false;	// DXT1 compression mode flag
bool forcedxtfive = false;	// DXT5 compression mode flag

typedef uint32_t DWORD;
typedef uint8_t BYTE;

constexpr DWORD DDS_MAGIC = 0x20534444;	// DDS file magic number

struct DDS_PIXELFORMAT {
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwABitMask;
};

struct DDS_HEADER {
	DWORD dwSize;
	DWORD dwHeaderFlags;
	DWORD dwHeight;
	DWORD dwWidth;
	DWORD dwPitchOrLinearSize;
	DWORD dwDepth;
	DWORD dwMipMapCount;
	DWORD dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	DWORD dwSurfaceFlags;
	DWORD dwCubemapFlags;
	DWORD dwReserved2[3];
};

struct TEX_Header {
	DWORD dwVersion = 0x00000007;	// TEX magic number
	BYTE bHash[16] = {};			// Placeholder, generally unused in our conversion
	DWORD dwUnknown14 = 0;			// Placeholder
	DWORD dwFormat;					// TEX format derived from DDS format
	DWORD dwWidth;
	DWORD dwHeight;
	DWORD dwUnknown24 = 0;			// Placeholder
	DWORD dwMipCount;
	DWORD dwUnknown2C = 0;			// Placeholder
	DWORD dwUnknown30 = 0;			// Placeholder
};

// Function to create output directory
void createDirectories(const std::string& path) {
	std::filesystem::create_directories(path);
}

// Function to print the help message
void printHelpMessage() {
	std::cout << "\n";
	std::cout << "👻 GBTVGR DDS to TEX Converter v0.1.0\n";
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

// Helper function to map DDS pixel format to TEX format codes
DWORD mapDDSPixelFormatToTEX(const DDS_PIXELFORMAT& ddsPixelFormat, DWORD cubemapFlag) {
	if (ddsPixelFormat.dwFourCC == 0x31545844)	// "DXT1"
		return 0x2B;
	if (ddsPixelFormat.dwFourCC == 0x33545844)	// "DXT3"
		return 0x17;
	if (ddsPixelFormat.dwFourCC == 0x35545844)	// "DXT5"
		return 0x32;
	if (ddsPixelFormat.dwRGBBitCount == 32 && ddsPixelFormat.dwRBitMask == 0x00FF0000) {
		return cubemapFlag ? 0x18 : 0x03;	// A8R8G8B8 (0x18 if cubemap)
	}
	if (ddsPixelFormat.dwRGBBitCount == 64 && ddsPixelFormat.dwFourCC == 0x71) {
		return 0x2E;	// A16B16G16R16F
	}
	if (ddsPixelFormat.dwRGBBitCount == 16 && ddsPixelFormat.dwRBitMask == 0x00FF && ddsPixelFormat.dwABitMask == 0xFF00) {
		return 0x2F;	// A8L8
	}
	if (ddsPixelFormat.dwRGBBitCount == 16 && ddsPixelFormat.dwRBitMask == 0xF800) {
		return 0x04;	// R5G6B5
	}
	if (ddsPixelFormat.dwRGBBitCount == 16 && ddsPixelFormat.dwRBitMask == 0x0F00) {
		return 0x05;	// A4R4G4B4
	}
	if (ddsPixelFormat.dwRGBBitCount == 8) {
		return 0x37;	// L8
	}

	std::cerr << "* ERROR: Unsupported DDS pixel format.\n";
	return 0;
}

// Function to validate the DDS file header
bool validateDDSFile(const std::string& filePath) {
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "* ERROR: Unable to open file: " << filePath << std::endl;
		return false;
	}

	DWORD magic;
	file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
	file.close();

	return magic == DDS_MAGIC;
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

	// Check if input file is provided
	if (inputFile.empty()) {
		std::cerr << "* ERROR: No input file specified.\n";
		printHelpMessage();
		return 1;
	}

	// Generate default output file if not provided
	if (outputFile.empty()) {
		outputFile = std::filesystem::path(inputFile).replace_extension(".tex").string();
	}

	// Convert DDS to TEX
	// Validate DDS file
	if (!validateDDSFile(inputFile)) {
		std::cerr << "* ERROR: Not a valid DDS file!\n";
		return 3;
	}

	// Open DDS file
	std::ifstream ddsFile(inputFile, std::ios::binary);
	if (!ddsFile.is_open()) {
		std::cerr << "* ERROR: Unable to open DDS file: " << inputFile << std::endl;
		return 1;
	}

	// Read DDS header
	DWORD magic;
	DDS_HEADER ddsHeader;
	ddsFile.read(reinterpret_cast<char*>(&magic), sizeof(magic));
	ddsFile.read(reinterpret_cast<char*>(&ddsHeader), sizeof(DDS_HEADER));

	// Check if the DDS file is a cubemap
	bool isCubemap = (ddsHeader.dwCubemapFlags & 0x200) != 0;

	// Map DDS format to TEX format
	DWORD texFormat = mapDDSPixelFormatToTEX(ddsHeader.ddspf, isCubemap);
	if (texFormat == 0) {
		std::cerr << "* ERROR: Conversion failed due to unsupported format.\n";
		return 1;
	}

	// Check compression type (DXT1 or DXT5)
	if (forcedxtone && !forcedxtfive && texFormat != 43) {
		std::cerr << "* ERROR: MUST USE DXT1 COMPRESSION!" << std::endl;
		return 9;
	} else if (!forcedxtone && forcedxtfive && texFormat != 50) {
		std::cerr << "* ERROR: MUST USE DXT5 COMPRESSION!" << std::endl;
		return 7;
	} else if (forcedxtone && forcedxtfive && texFormat != 43 && texFormat != 50) {
		std::cerr << "* ERROR: MUST USE DXT1 OR DXT5 COMPRESSION!" << std::endl;
		return 5;
	}

	// Populate TEX header
	TEX_Header texHeader;
	texHeader.dwFormat = texFormat;
	texHeader.dwWidth = ddsHeader.dwWidth;
	texHeader.dwHeight = ddsHeader.dwHeight;
	texHeader.dwMipCount = ddsHeader.dwMipMapCount > 0 ? ddsHeader.dwMipMapCount - 1 : 0;

	// Calculate DDS data size
	ddsFile.seekg(0, std::ios::end);
	size_t fileSize = static_cast<std::streamoff>(ddsFile.tellg()) - static_cast<std::streamoff>(sizeof(DDS_HEADER)) - static_cast<std::streamoff>(sizeof(DWORD));	// subtract header sizes
	ddsFile.seekg(sizeof(DDS_HEADER) + sizeof(DWORD), std::ios::beg);

	// Read DDS data
	std::vector<char> ddsData(fileSize);
	ddsFile.read(ddsData.data(), fileSize);
	ddsFile.close();

	std::string pathTo = std::filesystem::path(outputFile).parent_path().string();
	std::string file = std::filesystem::path(outputFile).stem().string();

	// Create output directory if not exists
	createDirectories(pathTo);

	// Write TEX file
	std::ofstream texFile(outputFile, std::ios::binary);
	if (!texFile.is_open()) {
		std::cerr << "* ERROR: Unable to open TEX file: " << outputFile << std::endl;
		return 1;
	}

	texFile.write(reinterpret_cast<const char*>(&texHeader), sizeof(TEX_Header));
	texFile.write(ddsData.data(), fileSize);

	const std::vector<char> overwriteBytes = {0x4b, 0x65, 0x79, 0x6f, 0x66, 0x42, 0x6c, 0x75, 0x65, 0x53};
	texFile.seekp(4);
	texFile.write(overwriteBytes.data(), overwriteBytes.size());

	texFile.close();

	if (!quiet) std::cout << "Conversion complete: " << outputFile << std::endl;

	return 0;
}
