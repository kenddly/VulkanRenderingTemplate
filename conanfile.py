import os

from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.files import copy


class ImGuiExample(ConanFile):
    version = "0.0.1"
    name = "vulkan"
    generators = "CMakeDeps", "CMakeToolchain"
    settings = "os", "arch", "compiler", "build_type"

    def requirements(self):
        self.requires("imgui/1.92.4")
        self.requires("glfw/3.4")
        self.requires("glm/1.0.1")
        self.requires("glslang/1.4.313.0")
        self.requires("doctest/2.4.11")
        self.requires("vulkan-memory-allocator/3.3.0")
        self.requires("entt/3.15.0")

    def generate(self):
        copy(
            self,
            "*glfw*",
            os.path.join(self.dependencies["imgui"].package_folder, "res", "bindings"),
            os.path.join(self.source_folder, "bindings"),
        )
        copy(
            self,
            "*vulkan*",
            os.path.join(self.dependencies["imgui"].package_folder, "res", "bindings"),
            os.path.join(self.source_folder, "bindings"),
        )

    def layout(self):
        cmake_layout(self)
