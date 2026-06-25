from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout


class DISPatchConan(ConanFile):
    name = "dispatch"
    version = "0.1.0"
    package_type = "application"
    settings = "os", "compiler", "build_type", "arch"
    options = {"tests": [True, False]}
    default_options = {
        "tests": False,
        "qt/*:shared": True,
    }
    exports_sources = (
        "CMakeLists.txt",
        "README.md",
        "etc/*",
        "include/*",
        "main.cpp",
        "resources/*",
        "src/*",
        "tests/*",
    )

    def requirements(self):
        self.requires("qt/5.15.14")
        if self.options.tests:
            self.requires("catch2/3.7.1")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        toolchain = CMakeToolchain(self)
        toolchain.variables["DISPATCH_WITH_TESTS"] = bool(self.options.tests)
        toolchain.generate()
        dependencies = CMakeDeps(self)
        dependencies.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        if self.options.tests:
            cmake.test()
