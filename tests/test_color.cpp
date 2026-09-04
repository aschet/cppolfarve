// SPDX-FileCopyrightText: 2026 Thomas Ascher <thomas.ascher@gmx.at>
//
// SPDX-License-Identifier: MIT

/// \file
/// \brief Tests for the color conversion functions.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <cmath>
#include <cstddef>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <olfarve/olfarve.hpp>

#include "cie.hpp"
#include "model.hpp"

using Catch::Approx;
using olfarve::absorption_to_srgb;
using olfarve::ebc_to_srgb;
using olfarve::rgb8;
using olfarve::srgb_color;
using olfarve::srm_to_srgb;

namespace
{

/// Sum of the three components, as a stand in for perceived brightness.
double total(const srgb_color& color)
{
    return color.r + color.g + color.b;
}

} // namespace

TEST_CASE("reference colors of the SRM scale", "[color]")
{
    const auto [srm, expected] = GENERATE(table<int, std::string>({
        {1, "#fae8b6"},
        {2, "#f4d180"},
        {4, "#e7aa31"},
        {10, "#ba5b00"},
        {20, "#7d1900"},
        {30, "#540000"},
        {40, "#390000"},
        {50, "#270000"},
    }));

    CAPTURE(srm);
    CHECK(srm_to_srgb(srm).to_hex() == expected);
}

TEST_CASE("the precomputed table matches the model", "[color]")
{
    // The conversion reads the table instead of recomputing the wavelength
    // dependent terms, so a drift here would silently change every result.
    const olfarve::detail::spectrum& table = olfarve::detail::cached_spectrum();
    double wavelength_nm = olfarve::detail::first_wavelength_nm;

    for (std::size_t i = 0; i < olfarve::detail::cie_sample_count; ++i)
    {
        CAPTURE(wavelength_nm);
        const olfarve::detail::spectrum_entry& entry = table.entries[i];
        const olfarve::detail::cie_sample& sample =
            olfarve::detail::cie_samples[i];

        CHECK(entry.absorption_ratio
              == olfarve::detail::absorption_ratio(wavelength_nm));
        CHECK(entry.s_d65 == sample.s_d65);
        CHECK(entry.x_bar == sample.x_bar);
        CHECK(entry.y_bar == sample.y_bar);
        CHECK(entry.z_bar == sample.z_bar);
        wavelength_nm += olfarve::detail::wavelength_step_nm;
    }
}

TEST_CASE("the colorimetric table has the documented shape", "[color]")
{
    STATIC_REQUIRE(olfarve::detail::cie_sample_count == 81);
    CHECK(olfarve::detail::first_wavelength_nm == 380.0);
    CHECK(olfarve::detail::wavelength_step_nm == 5.0);

    for (const olfarve::detail::cie_sample& sample :
         olfarve::detail::cie_samples)
    {
        CHECK(sample.x_bar >= 0.0);
        CHECK(sample.y_bar >= 0.0);
        CHECK(sample.z_bar >= 0.0);
        CHECK(sample.s_d65 >= 0.0);
    }
}

TEST_CASE("the normalization factor scales white to one", "[color]")
{
    // An unabsorbing sample renders as white.
    const srgb_color white = absorption_to_srgb(0.0);
    CHECK(white.r == Approx(1.0).margin(1e-4));
    CHECK(white.g == Approx(1.0).margin(1e-4));
    CHECK(white.b == Approx(1.0).margin(1e-4));
    CHECK(white.to_hex() == "#ffffff");
}

TEST_CASE("EBC matches the equivalent SRM value", "[color]")
{
    // EBC and SRM are the same scale up to the 25.0 / 12.7 factor.
    const int srm = GENERATE(1, 5, 10, 25, 40);
    CAPTURE(srm);

    const srgb_color from_ebc = ebc_to_srgb(srm * 25.0 / 12.7);
    const srgb_color from_srm = srm_to_srgb(srm);
    CHECK(from_ebc.r == Approx(from_srm.r));
    CHECK(from_ebc.g == Approx(from_srm.g));
    CHECK(from_ebc.b == Approx(from_srm.b));
}

TEST_CASE("components stay within the unit range", "[color]")
{
    for (int srm = 0; srm <= 60; ++srm)
    {
        CAPTURE(srm);
        const srgb_color color = srm_to_srgb(srm);
        CHECK(color.r >= 0.0);
        CHECK(color.r <= 1.0);
        CHECK(color.g >= 0.0);
        CHECK(color.g <= 1.0);
        CHECK(color.b >= 0.0);
        CHECK(color.b <= 1.0);
    }
}

TEST_CASE("the color darkens monotonically with the color value", "[color]")
{
    double previous = std::numeric_limits<double>::infinity();
    for (int srm = 0; srm <= 40; ++srm)
    {
        CAPTURE(srm);
        const double luminance = total(srm_to_srgb(srm));
        CHECK(luminance < previous);
        previous = luminance;
    }
}

TEST_CASE("a longer path length darkens the color", "[color]")
{
    CHECK(total(srm_to_srgb(10.0, 10.0)) < total(srm_to_srgb(10.0, 1.0)));
}

TEST_CASE("a zero path length is white", "[color]")
{
    CHECK(srm_to_srgb(20.0, 0.0).to_hex() == "#ffffff");
}

TEST_CASE("the default path length matches the BJCP glass width", "[color]")
{
    STATIC_REQUIRE(olfarve::default_path_length_cm == 5.0);
    CHECK(srm_to_srgb(10.0)
          == srm_to_srgb(10.0, olfarve::default_path_length_cm));
    CHECK(ebc_to_srgb(20.0)
          == ebc_to_srgb(20.0, olfarve::default_path_length_cm));
}

TEST_CASE("negative input is rejected", "[color]")
{
    CHECK_THROWS_AS(absorption_to_srgb(-0.1), std::invalid_argument);
    CHECK_THROWS_AS(absorption_to_srgb(1.0, -1.0), std::invalid_argument);
    CHECK_THROWS_AS(srm_to_srgb(-1.0), std::invalid_argument);
    CHECK_THROWS_AS(srm_to_srgb(1.0, -1.0), std::invalid_argument);
    CHECK_THROWS_AS(ebc_to_srgb(-1.0), std::invalid_argument);
    CHECK_THROWS_AS(ebc_to_srgb(1.0, -1.0), std::invalid_argument);
}

TEST_CASE("a value that is not a number is rejected", "[color]")
{
    // The Python suite has no counterpart: there a NaN travels until round()
    // raises, while here the cast to an integer would be undefined behaviour,
    // so it is refused at the door and again at the quantization.
    const double not_a_number = std::numeric_limits<double>::quiet_NaN();

    CHECK_THROWS_AS(absorption_to_srgb(not_a_number), std::invalid_argument);
    CHECK_THROWS_AS(srm_to_srgb(not_a_number), std::invalid_argument);
    CHECK_THROWS_AS(ebc_to_srgb(not_a_number), std::invalid_argument);
    CHECK_THROWS_AS(srm_to_srgb(1.0, not_a_number), std::invalid_argument);

    const srgb_color hand_built{not_a_number, not_a_number, not_a_number};
    CHECK(hand_built.to_rgb8() == rgb8{0, 0, 0});
    CHECK(hand_built.to_hex() == "#000000");
}

TEST_CASE("a color is an aggregate of three components", "[color]")
{
    const srgb_color color = srm_to_srgb(10.0);
    const auto [red, green, blue] = color;
    CHECK(red == color.r);
    CHECK(green == color.g);
    CHECK(blue == color.b);

    STATIC_REQUIRE(std::is_aggregate_v<srgb_color>);
    CHECK(srgb_color{} == srgb_color{0.0, 0.0, 0.0});
    CHECK(color != srgb_color{});
}

TEST_CASE("quantization to eight bits", "[color]")
{
    CHECK(srgb_color{1.0, 0.5, 0.0}.to_rgb8() == rgb8{255, 128, 0});
    CHECK(srgb_color{0.0, 0.0, 0.0}.to_rgb8() == rgb8{0, 0, 0});

    SECTION("out of gamut components are clamped")
    {
        CHECK(srgb_color{2.0, -1.0, 0.0}.to_rgb8() == rgb8{255, 0, 0});
        CHECK(srgb_color{1.5, 1.5, 1.5}.to_rgb8() == rgb8{255, 255, 255});
    }
}

TEST_CASE("hex output", "[color]")
{
    SECTION("endpoints")
    {
        CHECK(srgb_color{1.0, 1.0, 1.0}.to_hex() == "#ffffff");
        CHECK(srgb_color{0.0, 0.0, 0.0}.to_hex() == "#000000");
        CHECK(srgb_color{1.0, 0.0, 0.0}.to_hex() == "#ff0000");
        CHECK(srgb_color{1.0, 0.5, 0.0}.to_hex() == "#ff8000");
    }

    SECTION("output is lowercase and padded")
    {
        const std::string text = srgb_color{0.04, 0.04, 0.04}.to_hex();
        CHECK(text.size() == 7);
        CHECK(text == "#0a0a0a");
    }

    SECTION("it agrees with to_rgb8")
    {
        // to_hex is derived from to_rgb8, so the two must never disagree.
        for (int srm = 0; srm <= 60; ++srm)
        {
            CAPTURE(srm);
            const srgb_color color = srm_to_srgb(srm);
            const rgb8 quantized = color.to_rgb8();

            // Independently formatted, so a bug in to_hex cannot hide here.
            std::ostringstream expected;
            expected << '#' << std::hex << std::setfill('0');
            for (const unsigned int channel :
                 {static_cast<unsigned int>(quantized.r),
                  static_cast<unsigned int>(quantized.g),
                  static_cast<unsigned int>(quantized.b)})
            {
                expected << std::setw(2) << channel;
            }
            CHECK(color.to_hex() == expected.str());
        }
    }

    SECTION("out of gamut components still yield seven characters")
    {
        CHECK(srgb_color{2.0, 0.0, 0.0}.to_hex() == "#ff0000");
        CHECK(srgb_color{-1.0, -1.0, -1.0}.to_hex() == "#000000");
        CHECK(srgb_color{1.5, 0.0, 0.0}.to_hex() == "#ff0000");
    }
}

TEST_CASE("gamma encoding", "[color]")
{
    SECTION("it clamps out of range input")
    {
        CHECK(olfarve::detail::encode_gamma(-1.0) == 0.0);
        CHECK(olfarve::detail::encode_gamma(2.0) == Approx(1.0));
    }

    SECTION("it is continuous at the knee")
    {
        // The two branches meet, up to the rounding of the sRGB constants.
        const double knee = 0.0031308;
        CHECK(olfarve::detail::encode_gamma(knee)
              == Approx(olfarve::detail::encode_gamma(knee + 1e-12))
                     .margin(1e-7));
    }

    SECTION("endpoints")
    {
        CHECK(olfarve::detail::encode_gamma(0.0) == 0.0);
        CHECK(olfarve::detail::encode_gamma(1.0) == Approx(1.0));
    }
}
