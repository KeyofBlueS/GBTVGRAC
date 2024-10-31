# Ghostbusters: The Video Game Remastered Asset Converters

These tools are designed to convert texture (TEX <-> DDS) and audio (SMP <-> OGG) assets for use in Ghostbusters: The Video Game and its remastered version.

- **tex2dds**: Converts TEX files to DDS for both the original and remastered versions of Ghostbusters: The Video Game.
- **dds2tex**: Converts DDS files back to TEX (tested with the remastered version, theoretically compatible with the original).
- **smp2ogg**: Converts SMP audio files to OGG for the remastered game.
- **ogg2smp**: Converts OGG audio files back to SMP for the remastered game.


# Build Instructions:

To compile these tools on Linux, use the following command:

`g++ -static -o <toolname> <toolname>.cpp`

To cross-compile for Windows (from Linux), use:

`x86_64-w64-mingw32-g++ -static -o <toolname> <toolname>.cpp`

# Usage:

Run the converter by specifying the input asset file:
```sh
$ ./<toolname> <input_file> [OPTIONS]
```
```
Options:
  -o, --output <output_file>        Specify the output file path and name
  -q, --quiet                       Disable output messages
  -h, --help                        Show this help message and exit
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
