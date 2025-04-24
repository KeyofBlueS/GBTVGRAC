/*  Ghostbusters The Video Game TEX to DDS Converter
	Copyright 2010 Jonathan Wilson
	Copyright barncastle
	Copyright 2025 KeyofBlueS - https://github.com/KeyofBlueS

	The Ghostbusters The Video Game TEX to DDS Converter is free software;
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
#include <cstdio>
#include <vector>
#include <getopt.h>

typedef uint32_t DWORD;
typedef uint8_t BYTE;

bool quiet = false;	// Quiet mode flag

#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
	((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) | \
	((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))

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

constexpr DWORD DDS_MAGIC = 0x20534444;
constexpr DWORD DDS_FOURCC = 0x00000004;
constexpr DWORD DDS_RGB = 0x00000040;
constexpr DWORD DDS_RGBA = 0x00000041;
constexpr DWORD DDS_LUMINANCE = 0x00020000;
constexpr DWORD DDS_LUMINANCEA = 0x00020001;
constexpr DWORD DDS_PF_SIZE = sizeof(DDS_PIXELFORMAT);

// Predefined DDS_PIXELFORMATs
const DDS_PIXELFORMAT DDSPF_DXT1 = { DDS_PF_SIZE, DDS_FOURCC, MAKEFOURCC('D','X','T','1'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_DXT2 = { DDS_PF_SIZE, DDS_FOURCC, MAKEFOURCC('D','X','T','2'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_DXT3 = { DDS_PF_SIZE, DDS_FOURCC, MAKEFOURCC('D','X','T','3'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_DXT4 = { DDS_PF_SIZE, DDS_FOURCC, MAKEFOURCC('D','X','T','4'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_DXT5 = { DDS_PF_SIZE, DDS_FOURCC, MAKEFOURCC('D','X','T','5'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_A8R8G8B8 = { DDS_PF_SIZE, DDS_RGBA, 0, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000 };
const DDS_PIXELFORMAT DDSPF_A1R5G5B5 = { DDS_PF_SIZE, DDS_RGBA, 0, 16, 0x00007C00, 0x000003E0, 0x0000001F, 0x00008000 };
const DDS_PIXELFORMAT DDSPF_A4R4G4B4 = { DDS_PF_SIZE, DDS_RGBA, 0, 16, 0x00000F00, 0x000000F0, 0x0000000F, 0x0000F000 };
const DDS_PIXELFORMAT DDSPF_R8G8B8 = { DDS_PF_SIZE, DDS_RGB, 0, 24, 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000 };
const DDS_PIXELFORMAT DDSPF_R5G6B5 = { DDS_PF_SIZE, DDS_RGB, 0, 16, 0x0000F800, 0x000007E0, 0x0000001F, 0x00000000 };
const DDS_PIXELFORMAT DDSPF_A8L8 = { DDS_PF_SIZE, DDS_LUMINANCEA, 0, 16, 0xFF, 0, 0, 0xFF00 };
const DDS_PIXELFORMAT DDSPF_L8 = { DDS_PF_SIZE, DDS_LUMINANCE, 0, 8, 0xFF, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_A16B16G16R16F = { DDS_PF_SIZE, DDS_FOURCC, 113, 0, 0, 0, 0, 0 };

constexpr DWORD DDS_HEADER_FLAGS_TEXTURE =	0x00001007;	// DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT ;
constexpr DWORD DDS_HEADER_FLAGS_MIPMAP =	0x00020000;	// DDSD_MIPMAPCOUNT;
constexpr DWORD DDS_HEADER_FLAGS_VOLUME =	0x00800000;	// DDSD_DEPTH;
constexpr DWORD DDS_HEADER_FLAGS_PITCH =	0x00000008;	// DDSD_PITCH;
constexpr DWORD DDS_HEADER_FLAGS_LINEARSIZE = 0x00080000;	// DDSD_LINEARSIZE;
constexpr DWORD DDS_SURFACE_FLAGS_TEXTURE =	0x00001000;	// DDSCAPS_TEXTURE
constexpr DWORD DDS_SURFACE_FLAGS_MIPMAP =	0x00400008;	// DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
constexpr DWORD DDS_SURFACE_FLAGS_CUBEMAP =	0x00000008;	// DDSCAPS_COMPLEX
constexpr DWORD DDS_CUBEMAP_POSITIVEX =		0x00000600;	// DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
constexpr DWORD DDS_CUBEMAP_NEGATIVEX =		0x00000a00;	// DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
constexpr DWORD DDS_CUBEMAP_POSITIVEY =		0x00001200;	// DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
constexpr DWORD DDS_CUBEMAP_NEGATIVEY =		0x00002200;	// DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
constexpr DWORD DDS_CUBEMAP_POSITIVEZ =		0x00004200;	// DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
constexpr DWORD DDS_CUBEMAP_NEGATIVEZ =		0x00008200;	// DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ
constexpr DWORD DDS_FLAGS_VOLUME =			0x00200000;	// DDSCAPS2_VOLUME

constexpr DWORD DDS_CUBEMAP_ALLFACES = DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX | \
										DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY | \
										DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ;

struct DDS_HEADER
{
	DWORD dwSize = 124;
	DWORD dwHeaderFlags = DDS_HEADER_FLAGS_TEXTURE;
	DWORD dwHeight = 0;
	DWORD dwWidth = 0;
	DWORD dwPitchOrLinearSize = 0;
	DWORD dwDepth = 0;	// only if DDS_HEADER_FLAGS_VOLUME is set in dwHeaderFlags
	DWORD dwMipMapCount = 0;
	DWORD dwReserved1[11] = {};
	DDS_PIXELFORMAT ddspf = {};
	DWORD dwSurfaceFlags = DDS_SURFACE_FLAGS_TEXTURE;
	DWORD dwCubemapFlags = 0;
	DWORD dwReserved2[3] = {};
};

struct TEX_Header
{
	DWORD dwVersion;
	BYTE bHash[16];
	DWORD dwUnknown14;
	DWORD dwFormat;
	DWORD dwWidth;
	DWORD dwHeight;
	DWORD dwUnknown24;
	DWORD dwMipCount;
	DWORD dwUnknown2C;
	DWORD dwUnknown30;
};

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
	std::cout << std::endl;
	std::cout << "👻 GBTVGR TEX to DDS Converter v0.4.0" << std::endl;
	std::cout << std::endl;
	std::cout << "Usage: tex2dds <input_file.tex> [options]" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -i, --input <input_file.dds>		Specify the input TEX file path and name." << std::endl;
	std::cout << "  -o, --output <output_file.dds>	Specify the output DDS file path and name." << std::endl;
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
		outputFile = std::filesystem::path(inputFile).replace_extension(".dds").string();
	}

	std::string pathTo = std::filesystem::path(outputFile).parent_path().string();
	std::string file = std::filesystem::path(outputFile).stem().string();

	std::string input = "tex";
	std::string output = "dds";

	//setConsoleTitleAndPrint("👻 GBTVGR Converter", "👻 GBTVGR TEX to DDS Converter v0.2.0:");

	// Check if the file has a valid TEX header
	if (!checkFileSignature(inputFile, "07000000")) {
		std::cerr << "* ERROR: \"" << inputFile << "\" is not a valid TEX!" << std::endl;
		return 3;
	}

	// Convert TEX to DDS
	// Open TEX file
	std::ifstream texFile(inputFile, std::ios::binary);
	if (!texFile.is_open()) {
		std::cerr << "* ERROR: Unable to open input file: " << inputFile << std::endl;
		return 1;
	}

	// Read TEX header
	TEX_Header texHeader;
	texFile.read(reinterpret_cast<char*>(&texHeader), sizeof(TEX_Header));

	// Determine file size
	texFile.seekg(0, std::ios::end);
	size_t fileSize = texFile.tellg() - sizeof(TEX_Header);
	texFile.seekg(sizeof(TEX_Header), std::ios::beg);

	// Read TEX data
	std::vector<char> texData(fileSize);
	texFile.read(texData.data(), fileSize);
	texFile.close();

	// Populate DDS header
	DDS_HEADER ddsHeader;
	ddsHeader.dwHeight = texHeader.dwHeight;
	ddsHeader.dwWidth = texHeader.dwWidth;

	if (texHeader.dwMipCount > 0) {
		ddsHeader.dwHeaderFlags |= 0x00020000;	// DDS_HEADER_FLAGS_MIPMAP
		ddsHeader.dwMipMapCount = texHeader.dwMipCount + 1;
		ddsHeader.dwSurfaceFlags |= 0x00400008;	// DDS_SURFACE_FLAGS_MIPMAP
	}

	switch (texHeader.dwFormat) {
	case 0x03:	// PC
	case 0x27:	// PS3
	case 0x16:	// XBOX360
		ddsHeader.ddspf = DDSPF_A8R8G8B8;
		break;
	case 0x04:
		ddsHeader.ddspf = DDSPF_R5G6B5;
		break;
	case 0x05:
		ddsHeader.ddspf = DDSPF_A4R4G4B4;
		break;
	case 0x17:
		ddsHeader.ddspf = DDSPF_DXT3;
		break;
	case 0x2B:	// PC
	case 0x2C:	// PS3
	case 0x28:	// XBOX360
		ddsHeader.ddspf = DDSPF_DXT1;
		break;
	case 0x18:	// PC
	case 0x26:	// PS3
	case 0x36:	// XBOX360
	case 0x1b:	// XBOX360
		ddsHeader.ddspf = DDSPF_A8R8G8B8;
		ddsHeader.dwSurfaceFlags |= DDS_SURFACE_FLAGS_CUBEMAP;
		ddsHeader.dwCubemapFlags = DDS_CUBEMAP_ALLFACES;
		break;
	case 0x2E:
		ddsHeader.ddspf = DDSPF_A16B16G16R16F;
		break;
	case 0x2F:	// PC
	case 0x31:	// PS3
	case 0x30:	// XBOX360
		ddsHeader.ddspf = DDSPF_A8L8;
		break;
	case 0x32:	// PC
	case 0x34:	// PS3
	case 0x33:	// XBOX360
		ddsHeader.ddspf = DDSPF_DXT5;
		break;
	case 0x37:
		ddsHeader.ddspf = DDSPF_L8;
		break;
		default:
			std::cerr << "* ERROR: Unsupported TEX format: " << texHeader.dwFormat << std::endl;
			return 1;
	}

	// Create output directory if not exists
	std::string outputPath = std::filesystem::path(outputFile).parent_path().string();
	createDirectories(outputPath);

	// Open DDS file for writing
	std::ofstream ddsFile(outputFile, std::ios::binary);
	if (!ddsFile.is_open()) {
		std::cerr << "* ERROR: Unable to open output file: " << outputFile << std::endl;
		return 1;
	}

	// Write DDS file contents
	ddsFile.write(reinterpret_cast<const char*>(&DDS_MAGIC), sizeof(DWORD));
	ddsFile.write(reinterpret_cast<const char*>(&ddsHeader), sizeof(DDS_HEADER));
	ddsFile.write(texData.data(), fileSize);
	ddsFile.close();

	if (!quiet) std::cout << "Conversion complete: " << outputFile << std::endl;
	return 0;
}
