/*  Ghostbusters The Video Game DDS to TEX Converter
	Copyright 2025 KeyofBlueS

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
#include <getopt.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <stdexcept>
#include <cstdint>

std::string platform = "pc";	// PC is the default platform
bool forcedxtone = false;	// DXT1 compression mode flag
bool forcedxtfive = false;	// DXT5 compression mode flag
bool quiet = false;	// Quiet mode flag

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

inline uint32_t swap16(uint16_t val) {
	return (val << 8) | (val >> 8);
}

inline int xgAddress2DTiledX(int blockOffset, int widthInBlocks, int texelBytePitch) {
	int alignedWidth = (widthInBlocks + 31) & ~31;
	int logBpp = (texelBytePitch >> 2) + ((texelBytePitch >> 1) >> (texelBytePitch >> 2));
	int offsetByte = blockOffset << logBpp;
	int offsetTile = (((offsetByte & ~0xFFF) >> 3) + ((offsetByte & 0x700) >> 2) + (offsetByte & 0x3F));
	int offsetMacro = offsetTile >> (7 + logBpp);

	int macroX = (offsetMacro % (alignedWidth >> 5)) << 2;
	int tile = (((offsetTile >> (5 + logBpp)) & 2) + (offsetByte >> 6)) & 3;
	int macro = (macroX + tile) << 3;
	int micro = ((((offsetTile >> 1) & ~0xF) + (offsetTile & 0xF)) & ((texelBytePitch << 3) - 1)) >> logBpp;

	return macro + micro;
}

inline int xgAddress2DTiledY(int blockOffset, int widthInBlocks, int texelBytePitch) {
	int alignedWidth = (widthInBlocks + 31) & ~31;
	int logBpp = (texelBytePitch >> 2) + ((texelBytePitch >> 1) >> (texelBytePitch >> 2));
	int offsetByte = blockOffset << logBpp;
	int offsetTile = (((offsetByte & ~0xFFF) >> 3) + ((offsetByte & 0x700) >> 2) + (offsetByte & 0x3F));
	int offsetMacro = offsetTile >> (7 + logBpp);

	int macroY = (offsetMacro / (alignedWidth >> 5)) << 2;
	int tile = ((offsetTile >> (6 + logBpp)) & 1) + ((offsetByte & 0x800) >> 10);
	int macro = (macroY + tile) << 3;
	int micro = (((offsetTile & ((texelBytePitch << 6) - 1 & ~0x1F)) + ((offsetTile & 0xF) << 1)) >> (3 + logBpp)) & ~1;

	return macro + micro + ((offsetTile & 0x10) >> 4);
}

void swizzle_x360(const std::vector<uint8_t>& input, std::vector<uint8_t>& output, int width, int height, int block_pixel_size, int texel_byte_pitch) {
	const int widthInBlocks = width / block_pixel_size;
	const int heightInBlocks = height / block_pixel_size;

	std::vector<uint8_t> swapped(input.size());
	if (input.size() % 2 != 0)
		throw std::runtime_error("Data size must be a multiple of 2 bytes!");

	for (size_t i = 0; i < input.size(); i += 2) {
		swapped[i]	 = input[i + 1];
		swapped[i + 1] = input[i];
	}

	output.resize(input.size());

	for (int j = 0; j < heightInBlocks; ++j) {
		for (int i = 0; i < widthInBlocks; ++i) {
			int blockOffset = j * widthInBlocks + i;
			int x = xgAddress2DTiledX(blockOffset, widthInBlocks, texel_byte_pitch);
			int y = xgAddress2DTiledY(blockOffset, widthInBlocks, texel_byte_pitch);

			int srcByteOffset = j * widthInBlocks * texel_byte_pitch + i * texel_byte_pitch;
			int dstByteOffset = y * widthInBlocks * texel_byte_pitch + x * texel_byte_pitch;

			if (dstByteOffset + texel_byte_pitch > output.size() ||
				srcByteOffset + texel_byte_pitch > swapped.size())
				continue;

			std::memcpy(&output[srcByteOffset], &swapped[dstByteOffset], texel_byte_pitch);
		}
	}
}

inline size_t calculate_morton_index(size_t t, size_t width, size_t height) {
	size_t num1 = 1, num2 = 1, num3 = t;
	size_t t_width = width, t_height = height;
	size_t num6 = 0, num7 = 0;

	while (t_width > 1 || t_height > 1) {
		if (t_width > 1) {
			num6 += num2 * (num3 & 1);
			num3 >>= 1;
			num2 *= 2;
			t_width >>= 1;
		}
		if (t_height > 1) {
			num7 += num1 * (num3 & 1);
			num3 >>= 1;
			num1 *= 2;
			t_height >>= 1;
		}
	}

	return num7 * width + num6;
}

void swizzle_morton(const std::vector<uint8_t>& input, std::vector<uint8_t>& output, int width, int height, int bytes_per_pixel, int block_width_height = 1) {
	int block_size_bytes = bytes_per_pixel * block_width_height * block_width_height;

	int blocks_w = width / block_width_height;
	int blocks_h = height / block_width_height;
	int total_blocks = blocks_w * blocks_h;

	output.resize(input.size());
	size_t source_index = 0;

	for (int t = 0; t < total_blocks; ++t) {
		size_t index = calculate_morton_index(t, blocks_w, blocks_h);
		size_t destination_index = index * block_size_bytes;

		std::memcpy(&output[source_index], &input[destination_index], block_size_bytes);

		source_index += block_size_bytes;
	}
}

void swizzle_switch(
	const std::vector<uint8_t>& input,
	std::vector<uint8_t>& output,
	int img_width,
	int img_height,
	int bytes_per_block,
	int block_height,
	int width_pad,
	int height_pad)
{

	// Resize output buffer
	output.resize(input.size());

	int width_show = img_width;
	int height_show = img_height;
	int width_real = img_width;
	int height_real = img_height;

	// Pad dimensions to nearest multiple of width_pad / height_pad
	if (img_width % width_pad != 0 || img_height % height_pad != 0) {
		width_real = ((img_width + width_pad - 1) / width_pad) * width_pad;
		height_real = ((img_height + height_pad - 1) / height_pad) * height_pad;
		img_width = width_real;
		img_height = height_real;
	}

	int image_width_in_gobs = img_width * bytes_per_block / 64;

	// Perform swizzling
	for (int Y = 0; Y < img_height; ++Y) {
		for (int X = 0; X < img_width; ++X) {
			size_t Z = static_cast<size_t>(Y) * img_width + X;

			size_t gob_address =
				static_cast<size_t>((Y / (8 * block_height)) * 512 * block_height * image_width_in_gobs) +
				static_cast<size_t>((X * bytes_per_block / 64) * 512 * block_height) +
				static_cast<size_t>(((Y % (8 * block_height)) / 8) * 512);

			int Xb = X * bytes_per_block;

			size_t address = gob_address
				+ ((Xb % 64) / 32) * 256
				+ ((Y % 8) / 2) * 64
				+ ((Xb % 32) / 16) * 32
				+ (Y % 2) * 16
				+ (Xb % 16);

			if (address + bytes_per_block <= output.size() &&
				Z * bytes_per_block + bytes_per_block <= input.size()) {
				std::memcpy(&output[address], &input[Z * bytes_per_block], bytes_per_block);
			}
		}
	}

	// Expand if dimensions were cropped
	if (width_show != width_real || height_show != height_real) {
		std::vector<uint8_t> cropped(width_show * height_show * bytes_per_block);

		for (int Y = 0; Y < height_show; ++Y) {
			size_t offset_in = static_cast<size_t>(Y) * width_real * bytes_per_block;
			size_t offset_out = static_cast<size_t>(Y) * width_show * bytes_per_block;

			if (offset_in + width_show * bytes_per_block <= input.size() &&
				offset_out + width_show * bytes_per_block <= cropped.size()) {
				std::memcpy(&cropped[offset_in], &input[offset_out], width_show * bytes_per_block);
			}
		}

		output.swap(cropped);
	}
}

// Function to create output directory
void createDirectories(const std::string& path) {
	std::filesystem::create_directories(path);
}

// Function to print the help message
void printHelpMessage() {
	std::cout << std::endl;
	std::cout << "👻 GBTVGR DDS to TEX Converter v0.6.0" << std::endl;
	std::cout << std::endl;
	std::cout << "Usage: dds2tex <input_file.dds> [options]" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -i, --input <input_file.dds>		Specify the input DDS file path and name." << std::endl;
	std::cout << "  -o, --output <output_file.tex>	Specify the output TEX file path and name." << std::endl;
	std::cout << "  -p, --platform <platform>		Output tex file for the <platform> version of the game." << std::endl;
	std::cout << "					Supported platforms are 'pc', 'ps3', 'xbox360' or 'switch'. Default is 'pc'." << std::endl;
	std::cout << "  -q, --quiet				Disable output messages." << std::endl;
	std::cout << "  -h, --help				Show this help message and exit." << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "Copyright © 2025 KeyofBlueS: <https://github.com/KeyofBlueS>." << std::endl;
	std::cout << "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>." << std::endl;
	std::cout << "This is free software: you are free to change and redistribute it." << std::endl;
	std::cout << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
}

// Helper function to map DDS pixel format to TEX format codes
DWORD mapDDSPixelFormatToTEX(const DDS_PIXELFORMAT& ddsPixelFormat, DWORD cubemapFlag) {
	//std::cout << "dwFourCC: " << ddsPixelFormat.dwFourCC << std::endl;
	//std::cout << "dwRGBBitCount: " << ddsPixelFormat.dwRGBBitCount << std::endl;
	//std::cout << "dwRBitMask: " << ddsPixelFormat.dwRBitMask << std::endl;
	//std::cout << "dwGBitMask: " << ddsPixelFormat.dwGBitMask << std::endl;
	//std::cout << "dwBBitMask: " << ddsPixelFormat.dwBBitMask << std::endl;
	//std::cout << "dwABitMask: " << ddsPixelFormat.dwABitMask << std::endl;
	if (ddsPixelFormat.dwFourCC == 0x31545844) {	// "DXT1"
		if (platform == "pc") {
			return 0x2b;
		} else if (platform == "ps3") {
			return 0x2c;
		} else if (platform == "xbox360") {
			return 0x28;
		}
	}
	if (ddsPixelFormat.dwFourCC == 0x33545844) {	// "DXT3"
		return 0x17;
	}
	if (ddsPixelFormat.dwFourCC == 0x35545844) {	// "DXT5"
		if (platform == "pc") {
			return 0x32;
		} else if (platform == "ps3") {
			return 0x34;
		} else if (platform == "xbox360") {
			return 0x33;
		}
	}
	if (ddsPixelFormat.dwRGBBitCount == 32 && ddsPixelFormat.dwRBitMask == 0x00FF0000) {	// A8R8G8B8
		if (platform == "pc") {
			return cubemapFlag ? 0x18 : 0x03;	// (0x18 if cubemap)
		} else if (platform == "ps3") {
			return cubemapFlag ? 0x26 : 0x27;	// (0x26 if cubemap)
		} else if (platform == "xbox360") {
			return cubemapFlag ? 0x36 : 0x16;	// (0x36 if cubemap)
		}
	}
	if (ddsPixelFormat.dwRGBBitCount == 32 && ddsPixelFormat.dwBBitMask == 0x00FF0000) {	// RGBA8888
		if (platform == "pc") {
			std::cerr << "* ERROR: Unsupported DDS pixel format for the PC version of the game." << std::endl;
			return 0;
		} else if (platform == "ps3") {
			return cubemapFlag ? 0x26 : 0x27;	// (0x26 if cubemap)
		} else if (platform == "xbox360") {
			return cubemapFlag ? 0x36 : 0x16;	// (0x36 if cubemap)
		} else if (platform == "switch") {
			return cubemapFlag ? 0x3F : 0x41;	// (0x3F if cubemap???)
		}
	}
	if (ddsPixelFormat.dwRGBBitCount == 64 && ddsPixelFormat.dwFourCC == 0x71) {	// A16B16G16R16F
		return 0x2e;
	}
	if (ddsPixelFormat.dwRGBBitCount == 16 && ddsPixelFormat.dwRBitMask == 0x00FF && ddsPixelFormat.dwABitMask == 0xFF00) {	// A8L8
		if (platform == "pc") {
			return 0x2f;
		} else if (platform == "ps3") {
			return 0x31;
		} else if (platform == "xbox360") {
			return 0x30;
		}
	}
	if (ddsPixelFormat.dwRGBBitCount == 16 && ddsPixelFormat.dwRBitMask == 0xF800) {	// R5G6B5
		return 0x04;
	}
	if (ddsPixelFormat.dwRGBBitCount == 16 && ddsPixelFormat.dwRBitMask == 0x0F00) {	// A4R4G4B4
		return 0x05;
	}
	if (ddsPixelFormat.dwRGBBitCount == 8) {	// L8
		return 0x37;
	}

	std::cerr << "* ERROR: Unsupported DDS pixel format." << std::endl;
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

	std::string inputFile;
	std::string outputFile;
	bool argError = false;

	// Define the long options for getopt
	struct option long_options[] = {
		{"input", required_argument, nullptr, 'i'},
		{"output", required_argument, nullptr, 'o'},
		{"platform", required_argument, nullptr, 'p'},
		{"dxt1", no_argument, nullptr, '1'},
		{"dxt5", no_argument, nullptr, '5'},
		{"quiet", no_argument, nullptr, 'q'},
		{"help", no_argument, nullptr, 'h'},
		{nullptr, 0, nullptr, 0}	// Terminate the list of options
	};

	// Parse command-line arguments
	int opt;
	int option_index = 0;
	while ((opt = getopt_long(argc, argv, "i:o:p:15qh", long_options, &option_index)) != -1) {
		switch (opt) {
			case 'i':
				inputFile = optarg;
				break;
			case 'o':
				outputFile = optarg;
				break;
			case 'p':
				platform = optarg;
				std::transform(platform.begin(), platform.end(), platform.begin(),
								[](unsigned char c) { return std::tolower(c); });
				break;
			case '1':
				forcedxtone = true;
				break;
			case '5':
				forcedxtfive = true;
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

	if (platform != "pc" && platform != "ps3" && platform != "xbox360" && platform != "switch") {
		argError = true;
		std::cerr << "* ERROR: Unsupported platform: '" << platform << "'. Supported platforms are 'pc', 'ps3', 'xbox360' or 'switch'." << std::endl;
	}

	if (argError) {
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
		std::cerr << "* ERROR: Not a valid DDS file!" << std::endl;
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
		std::cerr << "* ERROR: Conversion failed due to unsupported format." << std::endl;
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

	bool needsSwizzle = false;
	std::string swizzleType;
	int blockWidthHeight;
	int blockPixelSize;
	int texelBytePitch;
	bool convertToRGBA = false;

	int bytes_per_block = 4;
	int block_height = 8;
	int width_pad = 8;
	int height_pad = 8;
	
	switch (texHeader.dwFormat) {
	case 0x27:	// PS3 OK
		needsSwizzle = true;
		swizzleType = "morton";
		blockWidthHeight = 1;
		break;
	case 0x16:	// XBOX360
		needsSwizzle = true;
		swizzleType = "x360";
		blockPixelSize = 1;
		texelBytePitch = 4;
		convertToRGBA = true;
		break;
	case 0x3C:	// SWITCH
	case 0x3D:	// SWITCH
	case 0x3E:	// SWITCH
	case 0x3F:	// SWITCH
	case 0x40:	// SWITCH
	case 0x41:	// SWITCH
		needsSwizzle = true;
		swizzleType = "switch";
		bytes_per_block = 4;
		block_height = 16;
		width_pad = 8;
		height_pad = 8;
		break;
	case 0x28:	// XBOX360
		needsSwizzle = true;
		swizzleType = "x360";
		blockPixelSize = 4;
		texelBytePitch = 8;
		break;
	case 0x26:	// PS3
	case 0x36:	// XBOX360
	case 0x1b:	// XBOX360
		needsSwizzle = true;
		swizzleType = "morton";
		blockWidthHeight = 1;
		break;
	case 0x31:	// PS3
		needsSwizzle = true;
		swizzleType = "morton";
		blockWidthHeight = 1;
		break;
	case 0x30:	// XBOX360
		needsSwizzle = true;
		swizzleType = "x360";
		blockPixelSize = 1;
		texelBytePitch = 2;
		break;
	case 0x33:	// XBOX360
		needsSwizzle = true;
		swizzleType = "x360";
		blockPixelSize = 4;
		texelBytePitch = 16;
		break;
	}

	// Create output directory if not exists
	createDirectories(pathTo);

	// Write TEX file
	std::ofstream texFile(outputFile, std::ios::binary);
	if (!texFile.is_open()) {
		std::cerr << "* ERROR: Unable to open TEX file: " << outputFile << std::endl;
		return 1;
	}

	if (needsSwizzle) {
		std::vector<uint8_t> swizzled(ddsData.size());
		if (swizzleType == "x360") {
			swizzle_x360(reinterpret_cast<const std::vector<uint8_t>&>(ddsData), swizzled, texHeader.dwWidth, texHeader.dwHeight, blockPixelSize, texelBytePitch);
		} else if (swizzleType == "morton") {
			int bytesPerPixel = (ddsHeader.ddspf.dwRGBBitCount + 7) / 8;
			swizzle_morton(reinterpret_cast<const std::vector<uint8_t>&>(ddsData), swizzled, texHeader.dwWidth, texHeader.dwHeight, bytesPerPixel, blockWidthHeight);
			for (size_t i = 0; i + 1 < swizzled.size(); i += bytesPerPixel) {
				std::reverse(swizzled.begin() + i, swizzled.begin() + i + bytesPerPixel);
			}
		} else if (swizzleType == "switch") {
			swizzle_switch(reinterpret_cast<const std::vector<uint8_t>&>(ddsData), swizzled, texHeader.dwWidth, texHeader.dwHeight, bytes_per_block, block_height, width_pad, height_pad);
		}
		ddsData = std::vector<char>(swizzled.begin(), swizzled.end());
		if (convertToRGBA) {
			switch (texHeader.dwFormat) {
			case 0x16:	// XBOX360
				for (size_t i = 0; i + 3 < ddsData.size(); i += 4) {
					uint8_t a = static_cast<uint8_t>(ddsData[i + 0]);
					uint8_t r = static_cast<uint8_t>(ddsData[i + 2]);
					uint8_t g = static_cast<uint8_t>(ddsData[i + 1]);
					uint8_t b = static_cast<uint8_t>(ddsData[i + 3]);
					ddsData[i + 0] = r;
					ddsData[i + 1] = g;
					ddsData[i + 2] = b;
					ddsData[i + 3] = a;
				}
				break;
			}
		}
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
