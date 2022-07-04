from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
from conan.errors import ConanInvalidConfiguration
from conans import tools
import os


required_conan_version = ">=1.49.0"


class BluezQtConan(ConanFile):
    name = "BluezQt"
    version = "5.96.0"
    license = ("LGPL-2.1", "LGPL-3.0", "CC0-1.0")
    url = ""
    homepage = "https://api.kde.org/frameworks/bluez-qt/html/index.html"
    description = "Qt wrapper for BlueZ 5 DBus API"
    topics = ("DBUS", "BlueZ", "Frameworks", "Tier 1")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False]
    }
    default_options = {
        "shared": True
    }
    exports_sources = "*"
    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto"
    }

    _cmake_helper = None

    @property
    def _cmake(self):
        if self._cmake_helper:
            return self._cmake_helper

        self._cmake_helper = CMake(self)
        self._cmake_helper.configure()

        return self._cmake_helper

    def requirements(self):
        self.requires("extra-cmake-modules/5.96.0@kde/testing")
        self.requires("qt/5.15.2")

    def build_requrements(self):
        self.tool_requires("cmake/[>=3.16.0]")

    def validate(self):
        if self.settings.os != "Linux":
            raise ConanInvalidConfiguration(f"{self.settings.os} is not supported. Only Linux.")

        if not self.options["qt"].with_dbus:
            raise ConanInvalidConfiguration("This library requires Qt with DBUS.")

    def generate(self):
        toolchain = CMakeToolchain(self)
        qt = self.dependencies["qt"]
        ecm = self.dependencies["extra-cmake-modules"]
        toolchain.variables["REQUIRED_QT_VERSION"] = qt.ref.version
        toolchain.variables["QT_MAJOR_VERSION"] = tools.Version(qt.ref.version).major
        toolchain.variables["REQUIRED_ECM_VERSION"] = ecm.ref.version
        toolchain.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        self._cmake.build()

    def package(self):
        self._cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "none")
        self.cpp_info.builddirs = [os.path.join("lib", "cmake", "KF5BlueZQt")]
