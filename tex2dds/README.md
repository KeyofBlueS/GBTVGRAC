# Ghostbusters: The Video Game Remastered Asset Converters (tex2dds)

**tex2dds:** Converts TEX files, whether from the original game (PC, PS3, Xbox 360) or the remastered version (PC), into DDS format.

**Note:** The program will automatically unswizzle textures when required.
This is necessary for certain textures used in the PS3 version and for all textures in the Xbox 360 version (the PC version does not use swizzled textures at all).
Keep in mind that this unswizzling feature is experimental, and the resulting DDS files may not always be accurate.


# Build Instructions:

To compile this tool, use the following command:

`g++ -static -o tex2dds tex2dds.cpp`

To cross-compile for Windows (from Linux), use:

`x86_64-w64-mingw32-g++ -static -o tex2dds tex2dds.cpp`


# Usage:

Run the converter by specifying the input TEX file:
```sh
$ ./tex2dds <input_file.tex> [OPTIONS]
```
```
Options:
  -i, --input <input_file.tex>      Specify the input TEX file path and name.
  -o, --output <output_file.dds>    Specify the output DDS file path and name.
  -q, --quiet                       Disable output messages.
  -h, --help                        Show this help message and exit.
```

Alternatively, you can drag and drop an TEX file onto the executable.


# Mass convert assets

Script for batch processing multiple TEXs is available on NexusMods:

[GBTVGR tex2dds Mass Converter](https://www.nexusmods.com/ghostbustersthevideogameremastered/mods/51)


# Credits:

The original tex2dds code was developed by Jonathan Wilson and barncastle.
