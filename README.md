# TankConvertor
Convert the tank format to any kind of general seismologic format, like SAC or miniSEED.

## Dependencies
Nothing special, it can be compiled under standard C library & pre-attached libmseed.

NOTE: libmseed version should newer than **3.0** if you want to use your own libmseed.

## Supported Platforms
- Linux
- MacOS (**Not pass testing yet**)

## Installation
- Linux/MacOS
	- Simply run ```make normal```
	- Or run ```make centos``` under redhat-like distribution

## Usage
- ```./tbconvert -h``` show some helping tips.
- ```./tbconvert -v``` show version information.
- ```./tbconvert -f SAC input.tnk``` or ```./tbconvert input.tnk``` convert the input.tnk into SAC format file(s).
- ```./tbconvert -f MSEED input.tnk``` convert the input.tnk into **MSEED(version 2)** format file.
- ```./tbconvert -f MSEED3 input.tnk``` convert the input.tnk into **MSEED(version 3)** format file.
- ```./tbconvert -f SAC -o output input.tnk``` convert the input.tnk into SAC format file(s) & store in specified **output** directory.
- ```./tbconvert -f MSEED -o output.mseed input.tnk``` convert the input.tnk into **MSEED(version 2)** format file with specified file name, **output.mseed**.
