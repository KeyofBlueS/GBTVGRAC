# Ghostbusters: The Video Game Remastered Asset Converters (smp2ogg)

**smp2ogg:** Converts SMP audio files from the remastered version (PC) into OGG format.


# Build Instructions:

To compile this tool, use the following command:

`g++ -static -o smp2ogg smp2ogg.cpp`

To cross-compile for Windows (from Linux), use:

`x86_64-w64-mingw32-g++ -static -o smp2ogg smp2ogg.cpp`


# Usage:

Run the converter by specifying the input SMP file:
```sh
$ ./smp2ogg <input_file.smp> [OPTIONS]
```
```
Options:
  -i, --input <input_file.smp>      Specify the input SMP file path and name.
  -o, --output <output_file.ogg>    Specify the output OGG file path and name.
  -q, --quiet                       Disable output messages.
  -h, --help                        Show this help message and exit.
```

Alternatively, you can drag and drop an SMP file onto the executable.


# Mass convert assets

Script for batch processing multiple SMPs is available on NexusMods:

[GBTVGR smp2ogg Mass Converter](https://www.nexusmods.com/ghostbustersthevideogameremastered/mods/50)
