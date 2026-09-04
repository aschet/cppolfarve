// SPDX-FileCopyrightText: 2026 Thomas Ascher <thomas.ascher@gmx.at>
//
// SPDX-License-Identifier: MIT

/// \file
/// \brief sRGB rendering of SRM and EBC beer color values.
///
/// The spectral model is A. J. de Lange, "Color," in *Brewing Materials and
/// Processes*, Elsevier, 2016, pp. 199-249: beer's transmittance across the
/// visible range is approximated from its absorption at 430 nm. Integrating
/// that against the CIE 1931 color matching functions under illuminant D65
/// gives XYZ tristimulus values, which are then transformed to sRGB.
///
/// The sRGB primaries, white point and gamma encoding follow
/// https://www.w3.org/Graphics/Color/srgb.

#ifndef OLFARVE_COLOR_HPP
#define OLFARVE_COLOR_HPP

#include <cstdint>
#include <string>

#include <olfarve/olfarve_export.hpp>

namespace olfarve
{

/// Default optical path length in cm, set to the typical sample glass width
/// specified by the BJCP color guide.
/// https://www.bjcp.org/education-training/education-resources/color-guide
inline constexpr double default_path_length_cm = 5.0;

/// An sRGB color quantized to 8 bits per channel.
struct rgb8
{
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
};

/// \relates rgb8
/// \brief Compare two 8 bit triplets channel by channel.
constexpr bool operator==(const rgb8& lhs, const rgb8& rhs) noexcept
{
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
}

/// \relates rgb8
constexpr bool operator!=(const rgb8& lhs, const rgb8& rhs) noexcept
{
    return !(lhs == rhs);
}

/// An sRGB color, gamma encoded, with components in [0, 1].
///
/// An aggregate, so it can be built, copied and structured-binding unpacked
/// like a plain triplet:
///
/// \code
/// auto [red, green, blue] = olfarve::srm_to_srgb(10.0);
/// \endcode
struct OLFARVE_EXPORT srgb_color
{
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    /// Return the color quantized to 8 bits per channel.
    ///
    /// Components are clamped into gamut first, so the result is a valid 8 bit
    /// triplet even for an instance built by hand out of range.
    ///
    /// \code
    /// olfarve::srgb_color{1.0, 0.5, 0.0}.to_rgb8();   // {255, 128, 0}
    /// olfarve::srgb_color{2.0, -1.0, 0.0}.to_rgb8();  // {255, 0, 0}
    /// \endcode
    [[nodiscard]] rgb8 to_rgb8() const noexcept;

    /// Return the color as a `#rrggbb` string.
    ///
    /// \code
    /// olfarve::srgb_color{1.0, 0.5, 0.0}.to_hex();   // "#ff8000"
    /// olfarve::srgb_color{2.0, -1.0, 0.0}.to_hex();  // "#ff0000"
    /// \endcode
    [[nodiscard]] std::string to_hex() const;
};

/// \relates srgb_color
/// \brief Compare two colors component by component.
///
/// Exact floating point comparison; use it to check that two conversions
/// agree, not to compare independently computed colors.
constexpr bool operator==(const srgb_color& lhs, const srgb_color& rhs) noexcept
{
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
}

/// \relates srgb_color
constexpr bool operator!=(const srgb_color& lhs, const srgb_color& rhs) noexcept
{
    return !(lhs == rhs);
}

/// Convert a beer's absorption at 430 nm into an sRGB color.
///
/// Prefer olfarve::srm_to_srgb() or olfarve::ebc_to_srgb() when you have a
/// color value, which is what brewing software reports. This function is for a
/// photometer reading taken directly, where the absorbance is the measurement
/// and the SRM or EBC value is derived from it.
///
/// \param absorption_430 Linear decadic absorption coefficient at 430 nm, in
///        cm^-1. Numerically this is the ASBC/EBC absorbance A430, which is
///        defined for a 1 cm path length.
/// \param path_length_cm Optical path length in cm, e.g. the glass width.
/// \return The gamma encoded color, with components in [0, 1].
/// \throw std::invalid_argument If either argument is negative or not a number.
[[nodiscard]] OLFARVE_EXPORT srgb_color absorption_to_srgb(
    double absorption_430, double path_length_cm = default_path_length_cm);

/// Convert a Standard Reference Method color value into an sRGB color.
///
/// \param srm The SRM color value.
/// \param path_length_cm Optical path length in cm, e.g. the glass width.
/// \return The gamma encoded color, with components in [0, 1].
/// \throw std::invalid_argument If either argument is negative or not a number.
OLFARVE_EXPORT srgb_color
srm_to_srgb(double srm, double path_length_cm = default_path_length_cm);

/// Convert a European Brewery Convention color value into an sRGB color.
///
/// \param ebc The EBC color value.
/// \param path_length_cm Optical path length in cm, e.g. the glass width.
/// \return The gamma encoded color, with components in [0, 1].
/// \throw std::invalid_argument If either argument is negative or not a number.
OLFARVE_EXPORT srgb_color
ebc_to_srgb(double ebc, double path_length_cm = default_path_length_cm);

} // namespace olfarve

#endif // OLFARVE_COLOR_HPP
