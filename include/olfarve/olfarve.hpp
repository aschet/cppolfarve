// SPDX-FileCopyrightText: 2026 Thomas Ascher <thomas.ascher@gmx.at>
//
// SPDX-License-Identifier: MIT

/// \file
/// \brief Øl farve: sRGB color rendering of SRM/EBC beer color values.
///
/// Umbrella header pulling in the whole public API:
///
/// \code
/// #include <olfarve/olfarve.hpp>
///
/// const std::string hex = olfarve::srm_to_srgb(10.0).to_hex();   // "#ba5b00"
/// const std::string ebc = olfarve::ebc_to_srgb(20.0, 1.0).to_hex();  //
/// "#f4d17e"
/// \endcode

#ifndef OLFARVE_OLFARVE_HPP
#define OLFARVE_OLFARVE_HPP

#include <olfarve/color.hpp>
#include <olfarve/version.hpp>

#endif // OLFARVE_OLFARVE_HPP
