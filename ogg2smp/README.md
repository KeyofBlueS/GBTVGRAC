# Ghostbusters: The Video Game Remastered Asset Converters (ogg2smp)

**ogg2smp:** Converts OGG audio files to SMP format specifically for the remastered version (PC).


# Build Instructions:

To compile this tool, use the following command:

`g++ -static -o ogg2smp ogg2smp.cpp -lvorbisfile -lvorbis -logg -pthread`

To cross-compile for Windows (from Linux), use:

`x86_64-w64-mingw32-g++ -static -o ogg2smp ogg2smp.cpp -lvorbisfile -lvorbis -logg -pthread`


# Usage:

Run the converter by specifying the input OGG file:
```sh
$ ./ogg2smp <input_file.ogg> [OPTIONS]
```
```
Options:
  -i, --input <input_file.ogg>      Specify the input OGG file path and name.
  -o, --output <output_file.smp>    Specify the output SMP file path and name.
  -q, --quiet                       Disable output messages.
  -h, --help                        Show this help message and exit.
```

Alternatively, you can drag and drop an OGG file onto the executable.


# Mass convert assets

Script for batch processing multiple OGGs is available on NexusMods:

[GBTVGR ogg2smp Mass Converter](https://www.nexusmods.com/ghostbustersthevideogameremastered/mods/47)
