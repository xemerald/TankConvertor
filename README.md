# TankConvertor
Convert the tank format to other kinds of seismological using format, like SAC or miniSEED.

## Dependencies
Nothing special, it can be compiled under standard C library & pre-attached miniSEED library.

NOTE: miniSEED library version should newer than **3.0** if you want to use your own libmseed.

## Supported Platforms
- Linux
- MacOS (**Not pass testing yet**)

## Building & Installation
- Linux/MacOS
	- Simply run `make normal`
	- Or run `make centos` under redhat-like distribution
	- Then you can run `sudo make install` to install this program to `/usr/local/bin`

## Usage
- `./tbconvert -h` show some helping tips.
- `./tbconvert -v` show version information.
- `./tbconvert -f SAC input.tnk` or `./tbconvert input.tnk` convert the input.tnk into SAC format file(s).
- `./tbconvert -f MSEED input.tnk` convert the input.tnk into **MSEED(version 2)** format file.
- `./tbconvert -f MSEED3 input.tnk` convert the input.tnk into **MSEED(version 3)** format file.
- `./tbconvert -f SAC -o output input.tnk` convert the input.tnk into SAC format file(s) & store in specified **output** directory.
- `./tbconvert -f MSEED -o output.mseed input.tnk` convert the input.tnk into **MSEED(version 2)** format file with specified file name, **output.mseed**.

## Something else
- When **NOT** specified the output path with **-o**, the result file(s) will be output to the current directory.
- Under miniSEED output, the earthworm null location code **"--"** will be omitted. However it will be kept under SAC output.

## License

Copyright&copy; 2019-2022 Benjamin Ming Yang

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

[Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0)

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

This Software contains [miniSEED Library v3.0.11](https://github.com/iris-edu/libmseed) under Apache License 2.0. For the licensing rule of miniSEED Library v3.0.11, please see the license file inside libmseed directory.
