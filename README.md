# cppolfarve

*Øl farve* ("beer color") renders SRM and EBC beer color values as sRGB
colors, following the spectral model described by A. J. de Lange, "Color," in
*Brewing Materials and Processes*, Elsevier, 2016, pp. 199-249.

Given a color value and an optical path length (the width of the glass the
beer is viewed through), the sample's spectral transmittance is derived from
its absorption coefficient at 430 nm via the Beer-Lambert law, integrated
against the CIE 1931 color matching functions of the 2 degree standard
colorimetric observer under illuminant D65, and the resulting XYZ tristimulus
values are transformed to sRGB.

## Installation

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build
```

The library requires C++17 or newer and has no runtime dependencies. Static
and shared builds are both supported, through `BUILD_SHARED_LIBS`.

## Usage

Consume it with:

```cmake
find_package(olfarve 1.0 REQUIRED)
target_link_libraries(my_app PRIVATE olfarve::olfarve)
```

or vendor the sources and `add_subdirectory()` them, which contributes nothing
but the library.

```cpp
#include <olfarve/olfarve.hpp>

olfarve::srm_to_srgb(10.0).to_hex();
olfarve::ebc_to_srgb(20.0).to_hex();

// The default path length is 5 cm, the width of a typical sample glass
olfarve::srm_to_srgb(10.0, 1.0).to_hex();

// Results are srgb_color aggregates of gamma encoded components in [0, 1]
const olfarve::srgb_color color = olfarve::srm_to_srgb(10.0);
const auto [red, green, blue] = color;
const olfarve::rgb8 quantized = color.to_rgb8();

// Or start from an absorbance measured at 430 nm
olfarve::absorption_to_srgb(0.7874);
```

## Development

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_COMPILE_WARNING_AS_ERROR=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
ctest --test-dir build
clang-tidy -p build src/color.cpp tests/*.cpp
```

Naming and layout follow the Boost conventions, encoded in `.clang-format`;
what the code does follows the [C++ Core
Guidelines](https://isocpp.github.io/CppCoreGuidelines/), enforced by
`.clang-tidy`.
