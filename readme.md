# softcut

### introduction

`softcut` is a C/C++ library for manipulating audio buffers in real time.

it provides a relatively low-level but robust interface for commonly needed functions in musical systems development:

- crossfaded looping with subsample accuracy
- resampling read and write heads
- crossfaded overdub behavior
- input and output multimode filters

this library was developed at the behest of [monome](https://monome.org), originally for use with the [norns](https://monome.org/norns/) sound computer.


### project structure

`./CMakeLists.txt` builds the library and the sample client(s).

`./softcut/` contains the library project itself.

`./softcut/include/` contains headers that will need to be included by the client project. (since the header names are pretty generic, it's recommended to include with the full path: `include "softcut/Voice.h"`, &c)

`./clients/` contains sample projects that use the library.

`./clients/softcut_jack_osc/` is a straightforward example: a JACK application that maps OSC commands to the softcut API.

### building

#### requirements:

- [cmake]() version 3.7 or greater
- C++ compiler supporting C++14 standard 
- [boost](https://www.boost.org/)
- [liblo](https://github.com/radarsat1/liblo)

#### steps:

standard CMake procedure: 
```
cd softcut/softcut
mkdir build-release
cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

this creates a static library at `build-release/libsoftcut.a`

if you are integrating softcut into a cmake-based project, you may prefer to include the softcut project as a subdirectory in your `CMakeLists.txt`.


### usage


big TODO here: API reference!
