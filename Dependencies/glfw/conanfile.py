from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration


class GLFWConan(ConanFile):
    name = "glfw"
    version = "3.3.4"
    license = "MIT"
    url = "https://github.com/glfw/glfw"
    description = "A multi-platform library for OpenGL, OpenGL ES, Vulkan, window and input"
    settings = "os", "compiler", "build_type", "arch"
    options = {'shared':[True, False]}
    default_options = "shared=True"
    generators = "cmake"
    exports_sources = [f'glfw-{version}/*']

    def source(self):
        tools.get(url=f'https://github.com/glfw/glfw/archive/refs/tags/{self.version}.zip', sha256='19A1048439A35E49F9B48FBE2E42787CFABAE70DF80FFD096B3B553BBD8A09F7')

    def configure_cmake(self):
        cmake = CMake(self)

        cmake.definitions['BUILD_SHARED_LIBS'] = 'ON' if self.options.shared == True else 'OFF'
        cmake.definitions['GLFW_BUILD_DOCS'] = 'OFF'
        cmake.definitions['GLFW_BUILD_EXAMPLES'] = 'OFF'
        cmake.definitions['GLFW_BUILD_TESTS'] = 'OFF'
        cmake.configure(source_folder=f'{self.source_folder}/glfw-{self.version}', build_folder= f'{self.build_folder}/build' )
        return cmake

    def build(self):
        cmake = self.configure_cmake()
        cmake.build()

    def package(self):
        cmake = self.configure_cmake()
        cmake.install()
        
    def package_info(self):
        if self.settings.os != 'Windows':
            self.cpp_info.libs = ['glfw']
        else:
            self.cpp_info.libs = ['glfw3dll']
        self.output.info(f'LIBS : {self.cpp_info.libs}')