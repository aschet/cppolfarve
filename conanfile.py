# SPDX-FileCopyrightText: 2026 Thomas Ascher <thomas.ascher@gmx.at>
#
# SPDX-License-Identifier: MIT

"""Conan 2 recipe building olfarve from this working tree.

Use it to consume the library from a local checkout::

    conan create .

or to develop against it, which brings the test dependency with it::

    conan install . --build=missing
    cmake --preset conan-release

The recipe published to Conan Center is a different one, built from a release
tarball and maintained in the conan-center-index repository.
"""

import os
import re

from conan import ConanFile
from conan.errors import ConanException
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy, load, rmdir

required_conan_version = ">=2.0"


class OlfarveConan(ConanFile):
    name = "olfarve"
    package_type = "library"
    license = "MIT"
    homepage = "https://github.com/aschet/cppolfarve"
    url = homepage
    description = "sRGB color rendering of SRM/EBC beer color values"
    topics = ("beer", "brewing", "color", "srm", "ebc", "srgb", "colorimetry")

    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }
    implements = ["auto_shared_fpic"]

    exports_sources = (
        "CMakeLists.txt",
        "LICENSE",
        "cmake/*",
        "include/*",
        "src/*",
        "tests/*",
    )

    @property
    def _skip_test(self):
        return bool(self.conf.get("tools.build:skip_test", default=False))

    def set_version(self):
        # The project declares the version once, in CMakeLists.txt.
        text = load(self, os.path.join(self.recipe_folder, "CMakeLists.txt"))
        match = re.search(r"project\(.*?VERSION\s+(\d+\.\d+\.\d+)", text, re.S)
        if match is None:
            raise ConanException("no project version found in CMakeLists.txt")
        self.version = match.group(1)

    def build_requirements(self):
        # Packaging runs with tests skipped and never resolves it.
        if not self._skip_test:
            self.test_requires("catch2/[>=3.5 <4]")

    def validate(self):
        # Falls back to the compiler's default when the profile sets no cppstd.
        check_min_cppstd(self, 17)

    def layout(self):
        cmake_layout(self)

    def generate(self):
        toolchain = CMakeToolchain(self)
        toolchain.cache_variables["BUILD_TESTING"] = not self._skip_test
        toolchain.generate()
        CMakeDeps(self).generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.test()  # a no-op when tools.build:skip_test is set

    def package(self):
        cmake = CMake(self)
        cmake.install()
        copy(
            self,
            "LICENSE",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "licenses"),
        )
        # Conan generates its own package files from the metadata below.
        for unused in ("lib/cmake", "lib/pkgconfig", "share/licenses"):
            rmdir(self, os.path.join(self.package_folder, unused))

    def package_info(self):
        # The CMake build gives the Windows debug library a distinct name.
        postfix = (
            "d"
            if self.settings.os == "Windows"
            and self.settings.build_type == "Debug"
            else ""
        )
        self.cpp_info.libs = ["olfarve" + postfix]
        self.cpp_info.set_property("cmake_file_name", "olfarve")
        self.cpp_info.set_property("cmake_target_name", "olfarve::olfarve")
        self.cpp_info.set_property("pkg_config_name", "olfarve")
