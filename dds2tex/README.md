# Ghostbusters: The Video Game Remastered Asset Converters (dds2tex)

**dds2tex:** Converts DDS files to TEX format for both the original game (PC, PS3, Xbox 360) and the remastered version (PC).

**Note:** The program will automatically swizzle textures when required.
This is necessary for certain textures used in the PS3 version and for all textures in the Xbox 360 version (the PC version does not require swizzling).
Keep in mind that this swizzling feature is experimental, and the resulting TEX files may be inaccurate or could potentially cause the game to crash.


# Build Instructions:

To compile this tool, use the following command:

`g++ -static -o dds2tex dds2tex.cpp`

To cross-compile for Windows (from Linux), use:

`x86_64-w64-mingw32-g++ -static -o dds2tex dds2tex.cpp`


# Usage:

Run the converter by specifying the input DDS file:
```sh
$ ./dds2tex <input_file.dds> [OPTIONS]
```
```
Options:
  -i, --input <input_file.dds>      Specify the input DDS file path and name.
  -o, --output <output_file.tex>    Specify the output TEX file path and name.
  -p, --platform <platform>         Output tex file for the <platform> version of the game.
                                    Supported platforms are 'pc', 'ps3' or 'xbox360'. Default is 'pc'.
  -q, --quiet                       Disable output messages.
  -h, --help                        Show this help message and exit.
```

Alternatively, you can drag and drop a DDS file onto the executable.


# Mass convert assets

Script for batch processing multiple DDSs is available on NexusMods:

[GBTVGR dds2tex Mass Converter](https://www.nexusmods.com/ghostbustersthevideogameremastered/mods/48)
