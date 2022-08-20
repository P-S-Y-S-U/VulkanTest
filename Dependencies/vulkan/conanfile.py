from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration


class VulkanConan(ConanFile):
    name = "vulkan"
    version = "1.3.216.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"

    def configure(self):
        vulkan_sdk = tools.get_env('VULKAN_SDK')
        version = vulkan_sdk[vulkan_sdk.rfind('\\') + 1:len(vulkan_sdk)]
        self.output.info(f' Version : {version}')
        if self.version != version:
            raise ConanInvalidConfiguration(f'Invalid vulkan version!!, recipe requires {self.version} but VULKAN_SDK set to {version}')

    def build(self):
        self.output.info("Nothing to Build")

    def package(self):
        vulkan_sdk = tools.get_env('VULKAN_SDK')
        self.copy("vulkan/*", dst="include", src=f"{vulkan_sdk}/Include")
        self.copy("vulkan-1.lib", dst="lib", src=f"{vulkan_sdk}/Lib")

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
        self.output.info(f"Libs : {self.cpp_info.libs}")