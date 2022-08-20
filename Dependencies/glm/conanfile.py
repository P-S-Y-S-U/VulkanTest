from conans import ConanFile, CMake, tools


class GLMConan(ConanFile):
    name = "glm"
    version = "0.9.9.8"
    license = "MIT"
    url = "https://github.com/g-truc/glm"
    description = "OpenGL Mathematics (GLM) is a header only C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specifications."
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"

    def source(self):
        tools.get(url=f'https://github.com/g-truc/glm/archive/refs/tags/{self.version}.zip', sha256='4605259C22FEADF35388C027F07B345AD3AA3B12631A5A316347F7566C6F1839')

    def build(self):
        print("Nothing to Build")

    def package(self):
        print("Installs in Package")
        self.copy("glm/*.h", dst="include", src=f"glm-{self.version}")
        self.copy("glm/*.hpp", dst="include", src=f"glm-{self.version}")
        self.copy("glm/*.inl", dst="include", src=f"glm-{self.version}")
        
    def package_info(self):
        print("Package Info header only lib")

    def package_id(self):
        self.info.header_only()        