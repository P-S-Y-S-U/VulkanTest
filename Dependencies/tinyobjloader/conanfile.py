from conans import ConanFile, CMake, tools
from conans.client.build.compiler_flags import build_type_flags, build_type_define
from conans.errors import ConanInvalidConfiguration

# class <pak_name>Conan(ConanFile): #
class TinyobjloaderConan(ConanFile):
    name = "tinyobjloader"
    version = "1.0.6"
    license = "https://github.com/tinyobjloader/tinyobjloader/blob/release/LICENSE"
    url = "https://github.com/tinyobjloader/tinyobjloader"
    description = "Tiny but powerful single file wavefront obj loader"
    settings = "os", "compiler", "build_type", "arch"
    options = {'shared':[True, False]}
    default_options = "shared=True"
    generators = "cmake"

    @property
    def _is_msvsc(self):
        return self.settings.os == 'Windows'
    
    @property
    def _is_linux(self):
        return self.settings.os == 'linux'

    def source(self):
        tools.get(url=f'https://github.com/tinyobjloader/tinyobjloader/archive/refs/tags/v{self.version}.zip' )

    def configure_cmake(self):
        cmake = CMake(self)

        # CMAKE DEFINITIONS #
        cmake.definitions['TINYOBJLOADER_COMPILATION_SHARED'] = 'ON' if self.options.shared == True else 'OFF'
        
        cmake.configure(source_folder=f'{self.source_folder}/tinyobjloader-{self.version}', build_folder= f'{self.build_folder}/build-{self.settings.build_type}' )
        return cmake

    def build(self):
        flags = build_type_flags(self.settings)
        defines = build_type_define(self.settings)
        self.output.info(f"build_type_flags : {flags}")
        self.output.info(f"build_type_defines : {defines}")

        cmake = self.configure_cmake()
        cmake.build()

    def package(self):
        cmake = self.configure_cmake()
        cmake.install()
        
    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self) # link library list #
        self.output.info(f'LIBS : {self.cpp_info.libs}')