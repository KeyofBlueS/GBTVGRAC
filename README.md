# Ghostbusters: The Video Game Remastered Asset Converters

These tools are designed to convert texture (TEX <-> DDS) and audio (SMP <-> OGG) assets for use in Ghostbusters: The Video Game and its remastered version.

**tex2dds:** Converts TEX files, whether from the original game (PC, PS3, Xbox 360) or the remastered version (PC, minimal support for Nintendo Switch), into DDS format.

**dds2tex:** Converts DDS files to TEX format for both the original game (PC, PS3, Xbox 360) and the remastered version (PC, minimal support for Nintendo Switch).

**smp2ogg:** Converts SMP audio files from the remastered version (PC) into OGG format.

**ogg2smp:** Converts OGG audio files to SMP format specifically for the remastered version (PC).


# General build Instructions:

To compile these tools, use the following command:

`g++ -static -o <toolname> <toolname>.cpp`

To cross-compile for Windows (from Linux), use:

`x86_64-w64-mingw32-g++ -static -o <toolname> <toolname>.cpp`


# Usage:

Run the converter by specifying the input asset file:
```sh
$ ./<toolname> <input_file> [OPTIONS]
```
```
Common options:
  -i, --input <input_file>          Specify the input file path and name.
  -o, --output <output_file>        Specify the output file path and name.
  -q, --quiet                       Disable output messages.
  -h, --help                        Show this help message and exit.
```

Alternatively, you can drag and drop an asset file onto the executable.


# Mass convert assets

Scripts for batch processing multiple assets are available on NexusMods:

[GBTVGR tex2dds Mass Converter](https://www.nexusmods.com/ghostbustersthevideogameremastered/mods/51)

[GBTVGR dds2tex Mass Converter](https://www.nexusmods.com/ghostbustersthevideogameremastered/mods/48)

[GBTVGR smp2ogg Mass Converter](https://www.nexusmods.com/ghostbustersthevideogameremastered/mods/50)

[GBTVGR ogg2smp Mass Converter](https://www.nexusmods.com/ghostbustersthevideogameremastered/mods/47)


# Credits:

The original tex2dds code was developed by Jonathan Wilson and barncastle.
