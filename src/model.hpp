// SPDX-FileCopyrightText: 2026 Thomas Ascher <thomas.ascher@gmx.at>
//
// SPDX-License-Identifier: MIT

/// \file
/// \brief The spectral model behind the conversion, and the sRGB encoding.
///
/// Header only and free of state, so the tests can exercise the pieces the
/// public API composes. This header is internal to the library; it is not
/// installed and its contents may change without notice.

#ifndef OLFARVE_MODEL_HPP
#define OLFARVE_MODEL_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

#include "cie.hpp"

namespace olfarve::detail
{

// Both scales are defined as a multiple of the absorbance at 430 nm measured
// over a 1 cm path: SRM = 12.7 * A430 and EBC = 25.0 * A430.
inline constexpr double srm_per_absorbance = 12.7;
inline constexpr double ebc_per_absorbance = 25.0;

// The de Lange approximation sums two exponentials decaying away from 430 nm,
// giving absorption at any wavelength relative to the absorption there.
inline constexpr double reference_wavelength_nm = 430.0;
inline constexpr double short_decay_weight = 0.02465;
inline constexpr double short_decay_nm = 17.591;
inline constexpr double long_decay_weight = 0.97535;
inline constexpr double long_decay_nm = 82.122;

// Beer-Lambert law: transmittance T = 10 ** -A, so the base is decadic.
inline constexpr double transmittance_base = 10.0;

// Linear sRGB from XYZ, D65 white point, one row per channel.
inline constexpr std::array<double, 3> red_from_xyz = {3.2406255, -1.537208,
                                                       -0.4986286};
inline constexpr std::array<double, 3> green_from_xyz = {-0.9689307, 1.8757561,
                                                         0.0415175};
inline constexpr std::array<double, 3> blue_from_xyz = {0.0557101, -0.2040211,
                                                        1.0569959};

// Piecewise sRGB gamma encoding: linear below the threshold, a power law above
// it.
inline constexpr double gamma_threshold = 0.0031308;
inline constexpr double gamma_slope = 12.92;
inline constexpr double gamma_scale = 1.055;
inline constexpr double gamma_offset = 0.055;
inline constexpr double gamma_exponent = 1.0 / 2.4;

/// One entry of the precomputed integration table.
struct spectrum_entry
{
    double absorption_ratio;
    double s_d65;
    double x_bar;
    double y_bar;
    double z_bar;
};

/// The wavelength dependent terms of the integration, evaluated once.
struct spectrum
{
    /// Normalizing constant for illuminant D65.
    ///
    /// CIE defines k = 100 / sum(S(lambda) * y_bar(lambda)), putting the
    /// luminance of a perfectly transmitting sample at 100. Dropping the factor
    /// of 100 puts it at 1.0 instead, which is the range sRGB expects.
    double k = 0.0;
    std::array<spectrum_entry, cie_sample_count> entries = {};
};

/// Return one row of the XYZ to linear sRGB matrix applied to a tristimulus
/// triplet.
[[nodiscard]] inline double apply_row(const std::array<double, 3>& row,
                                      double x, double y, double z) noexcept
{
    return (row[0] * x) + (row[1] * y) + (row[2] * z);
}

/// Return absorption at \p wavelength_nm relative to that at 430 nm.
[[nodiscard]] inline double absorption_ratio(double wavelength_nm) noexcept
{
    const double offset_nm = wavelength_nm - reference_wavelength_nm;
    return (short_decay_weight * std::exp(-offset_nm / short_decay_nm))
           + (long_decay_weight * std::exp(-offset_nm / long_decay_nm));
}

[[nodiscard]] inline spectrum build_spectrum()
{
    spectrum table;
    double luminance = 0.0;
    double wavelength_nm = first_wavelength_nm;
    for (std::size_t i = 0; i < cie_sample_count; ++i)
    {
        const cie_sample& sample = cie_samples[i];
        table.entries[i] =
            spectrum_entry{absorption_ratio(wavelength_nm), sample.s_d65,
                           sample.x_bar, sample.y_bar, sample.z_bar};
        luminance += sample.s_d65 * sample.y_bar;
        wavelength_nm += wavelength_step_nm;
    }
    table.k = 1.0 / luminance;
    return table;
}

/// Return the integration table, built on first use.
///
/// Only the absorbance varies between conversions. The absorption ratios and
/// the colorimetric weights depend solely on wavelength, so they are evaluated
/// once rather than on every call.
[[nodiscard]] inline const spectrum& cached_spectrum()
{
    static const spectrum table = build_spectrum();
    return table;
}

/// Gamma encode one linear component, clamping it to [0, 1] first.
///
/// This is the inverse of the sRGB EOTF: it maps a linear tristimulus component
/// to the non-linear signal a display decodes.
[[nodiscard]] inline double encode_gamma(double linear) noexcept
{
    linear = std::min(1.0, std::max(0.0, linear));
    if (linear <= gamma_threshold)
    {
        return linear * gamma_slope;
    }
    return (gamma_scale * std::pow(linear, gamma_exponent)) - gamma_offset;
}

} // namespace olfarve::detail

#endif // OLFARVE_MODEL_HPP
