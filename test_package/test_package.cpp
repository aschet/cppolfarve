// SPDX-FileCopyrightText: 2026 Thomas Ascher <thomas.ascher@gmx.at>
//
// SPDX-License-Identifier: MIT

#include <cstdlib>
#include <iostream>
#include <string>

#include <olfarve/olfarve.hpp>

int main()
{
    const std::string hex = olfarve::srm_to_srgb(10.0).to_hex();
    std::cout << "olfarve " << olfarve::version() << ": SRM 10 is " << hex
              << '\n';
    return hex == "#ba5b00" ? EXIT_SUCCESS : EXIT_FAILURE;
}
