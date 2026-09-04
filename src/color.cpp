// SPDX-FileCopyrightText: 2026 Thomas Ascher <thomas.ascher@gmx.at>
//
// SPDX-License-Identifier: MIT

#include <olfarve/color.hpp>
#include <olfarve/version.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

#include "model.hpp"

namespace olfarve
{
namespace
{

/// Largest value a single 8 bit channel can take.
constexpr double channel_max = 255.0;

/// Hexadecimal digits, indexed by nibble.
constexpr std::string_view hex_digits = "0123456789abcdef";
constexpr unsigned int nibble_bits = 4U;
constexpr std::uint8_t low_nibble = 0x0FU;

/// Quantize one gamma encoded component to an integer in [0, 255].
[[nodiscard]] std::uint8_t to_8bit(double component) noexcept
{
    if (std::isnan(component))
    {
        return 0;
    }
    const double scaled =
        std::round(std::min(1.0, std::max(0.0, component)) * channel_max);
    return static_cast<std::uint8_t>(scaled);
}

/// Reject an argument that is negative or not a number.
void require_non_negative(double value, const char* name)
{
    if (std::isnan(value))
    {
        throw std::invalid_argument(std::string(name) + " must be a number");
    }
    if (value < 0.0)
    {
        throw std::invalid_argument(std::string(name)
                                    + " must not be negative");
    }
}

} // namespace

rgb8 srgb_color::to_rgb8() const noexcept
{
    return rgb8{to_8bit(r), to_8bit(g), to_8bit(b)};
}

std::string srgb_color::to_hex() const
{
    const rgb8 quantized = to_rgb8();

    std::string text = "#000000";
    std::size_t position = 1;
    for (const std::uint8_t channel : {quantized.r, quantized.g, quantized.b})
    {
        text[position++] =
            hex_digits[static_cast<std::size_t>(channel >> nibble_bits)];
        text[position++] =
            hex_digits[static_cast<std::size_t>(channel & low_nibble)];
    }
    return text;
}

srgb_color absorption_to_srgb(double absorption_430, double path_length_cm)
{
    require_non_negative(absorption_430, "absorption_430");
    require_non_negative(path_length_cm, "path_length_cm");

    // Beer-Lambert law: absorbance A = a * l, and transmittance T = 10 ** -A.
    const double absorbance_430 = absorption_430 * path_length_cm;

    const detail::spectrum& table = detail::cached_spectrum();
    double tristimulus_x = 0.0;
    double tristimulus_y = 0.0;
    double tristimulus_z = 0.0;
    for (const detail::spectrum_entry& entry : table.entries)
    {
        const double transmitted_power =
            entry.s_d65
            * std::pow(detail::transmittance_base,
                       -absorbance_430 * entry.absorption_ratio);
        tristimulus_x += transmitted_power * entry.x_bar;
        tristimulus_y += transmitted_power * entry.y_bar;
        tristimulus_z += transmitted_power * entry.z_bar;
    }

    tristimulus_x *= table.k;
    tristimulus_y *= table.k;
    tristimulus_z *= table.k;

    // XYZ to linear sRGB, D65 white point.
    return srgb_color{
        detail::encode_gamma(detail::apply_row(
            detail::red_from_xyz, tristimulus_x, tristimulus_y, tristimulus_z)),
        detail::encode_gamma(detail::apply_row(detail::green_from_xyz,
                                               tristimulus_x, tristimulus_y,
                                               tristimulus_z)),
        detail::encode_gamma(detail::apply_row(detail::blue_from_xyz,
                                               tristimulus_x, tristimulus_y,
                                               tristimulus_z)),
    };
}

srgb_color srm_to_srgb(double srm, double path_length_cm)
{
    require_non_negative(srm, "srm");
    return absorption_to_srgb(srm / detail::srm_per_absorbance, path_length_cm);
}

srgb_color ebc_to_srgb(double ebc, double path_length_cm)
{
    require_non_negative(ebc, "ebc");
    return absorption_to_srgb(ebc / detail::ebc_per_absorbance, path_length_cm);
}

const char* version() noexcept
{
    return OLFARVE_VERSION_STRING;
}

} // namespace olfarve
