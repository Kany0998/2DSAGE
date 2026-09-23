from conan import ConanFile
from conan.tools.cmake import cmake_layout


class DSageDependencies(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    generators = "CMakeDeps", "CMakeToolchain"

    requires = (
        "sdl/2.32.10",
        "sdl_image/2.8.8",
        "sdl_ttf/2.24.0",
        "sdl_mixer/2.8.1",
        "glm/1.0.1",
        "lua/5.4.6",
        "sol2/3.5.0",
        "entt/3.16.0",
    )

    default_options = {
        "sdl/*:shared": True,
        "sdl_image/*:shared": True,
        "sdl_ttf/*:shared": True,
        "sdl_mixer/*:shared": True,
    }

    def layout(self):
        cmake_layout(self)