from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration

class VulkanRendererConan(ConanFile):
    name = "VulkanRenderer"
    version = "0.1.0"
    url = "https://github.com/P-S-Y-S-U/VulkanTest"
    settings = "os", "compiler", "build_type", "arch"
    options = { 'shared':[True, False] }
    default_options = "shared=True"
    generators = [ "cmake", "virtualrunenv" ]
    requires = [
        "glfw/3.3.8@psy/dev",
        "glm/0.9.9.8@psy/dev",
        "vulkan/1.3.216.0@psy/dev"
    ] 

    @property
    def _is_msvsc(self):
        return self.settings.os == 'Windows'
    
    @property
    def _is_linux(self):
        return self.settings.os == 'linux'

    def configure_cmake(self):
        cmake = CMake(self)

        # CMAKE DEFINITIONS #
        cmake.definitions['BUILD_SHARED_LIBS'] = 'ON' if self.options.shared == True else 'OFF'
        cmake.definitions["CONAN_INSTALL_INFO"] = self.install_folder
        
        cmake.configure(source_folder=self.source_folder, build_folder=f'{self.build_folder}/build-{self.settings.build_type}' )
        return cmake

    def build(self):
        cmake = self.configure_cmake()
        cmake.build()

    def package(self):
        cmake = self.configure_cmake()
        cmake.install()
        
    def package_info(self):
        self.cpp_info.libs = ['vulkanrenderer'] # link library list #
        self.output.info(f'LIBS : {self.cpp_info.libs}')