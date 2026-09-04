// SPDX-FileCopyrightText: 2026 Thomas Ascher <thomas.ascher@gmx.at>
//
// SPDX-License-Identifier: MIT

/// \file
/// \brief Tests for the packaging and the public surface of the library.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include <olfarve/olfarve.hpp>

TEST_CASE("the library reports the version of its headers", "[package]")
{
    // A mismatch means a shared library was loaded that does not belong to the
    // headers this test was compiled against.
    CHECK(std::string(olfarve::version()) == OLFARVE_VERSION_STRING);
    CHECK(std::string(OLFARVE_VERSION_STRING)
          == std::to_string(OLFARVE_VERSION_MAJOR) + "."
                 + std::to_string(OLFARVE_VERSION_MINOR) + "."
                 + std::to_string(OLFARVE_VERSION_PATCH));
}

TEST_CASE("the umbrella header is enough to use the library", "[package]")
{
    // Nothing below names a header other than <olfarve/olfarve.hpp>.
    const olfarve::srgb_color color = olfarve::srm_to_srgb(10.0);
    const olfarve::rgb8 quantized = color.to_rgb8();
    CHECK(quantized == olfarve::rgb8{186, 91, 0});
    CHECK(color.to_hex() == "#ba5b00");
    CHECK(olfarve::ebc_to_srgb(20.0, 1.0).to_hex() == "#f4d17e");
    CHECK(olfarve::absorption_to_srgb(0.7874).to_hex() == "#ba5b00");
    CHECK(olfarve::default_path_length_cm == 5.0);
}
