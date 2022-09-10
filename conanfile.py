from json import tool
from re import sub
from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration
from os import path
import glob

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
        "spdlog/[~1.10.0]@psy/dev",
        "vulkan/1.3.216.0@psy/dev"
    ] 

    @property
    def _vulkan_sdk(self):
        return tools.get_env('VULKAN_SDK')
    
    @property
    def _vulkan_bin_path(self):
        return path.join( self._vulkan_sdk, "Bin" )

    @property
    def _vulkan_shader_compiler(self):
        return path.join( self._vulkan_bin_path, "glslc.exe" )

    @property
    def _project_media_dir(self):
        return path.join( self.source_folder, "media" )
    
    @property
    def _shaders_dir(self):
        return path.join( self._project_media_dir, "shaders" )

    @property
    def _project_build_dir(self):
        return path.join( self.build_folder, f"build-{self.settings.build_type}")
    @property
    def _project_bin_dir(self):
        return path.join( self._project_build_dir, "bin" )

    @property
    def _is_msvsc(self):
        return self.settings.os == 'Windows'
    
    @property
    def _is_linux(self):
        return self.settings.os == 'linux'

    def compile_shaders(self):
        self.output.info("compiling Shaders")

        shaders = glob.glob( f"{self._shaders_dir}/*.vert" )

        for shaderpath in shaders:
            filename = shaderpath[ shaderpath.rfind("\\") + 1 : shaderpath.rfind(".") ]
            args = [
                self._vulkan_shader_compiler,
                shaderpath,
                "-o", f"{self._project_bin_dir}\\{filename}Vert.spv"
            ]
            cmd = " ".join(args)
            self.output.info(f"Compiling {shaderpath}")
            self.output.info(cmd)
            self.run(cmd)
        
        shaders = glob.glob( f"{self._shaders_dir}/*.frag" )

        for shaderpath in shaders:
            filename = shaderpath[ shaderpath.rfind("\\") + 1 : shaderpath.rfind(".") ]
            args = [
                self._vulkan_shader_compiler,
                shaderpath,
                "-o", f"{self._project_bin_dir}\\{filename}Frag.spv"
            ]
            cmd = " ".join(args)
            self.output.info(f"Compiling {shaderpath}")
            self.output.info(cmd)
            self.run(cmd)
        

    def configure_cmake(self):
        cmake = CMake(self)

        # CMAKE DEFINITIONS #
        cmake.definitions['BUILD_SHARED_LIBS'] = 'ON' if self.options.shared == True else 'OFF'
        cmake.definitions["CONAN_INSTALL_INFO"] = self.install_folder
        
        cmake.configure(source_folder=self.source_folder, build_folder=self._project_build_dir )
        return cmake

    def build(self):
        cmake = self.configure_cmake()
        cmake.build()
        self.compile_shaders()

    def package(self):
        cmake = self.configure_cmake()
        cmake.install()
        
    def package_info(self):
        self.cpp_info.libs = ['vulkanrenderer'] # link library list #
        self.output.info(f'LIBS : {self.cpp_info.libs}')