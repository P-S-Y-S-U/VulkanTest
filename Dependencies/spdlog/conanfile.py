from pickle import TRUE
from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration


class SpdlogConan(ConanFile):
    name = "spdlog"
    version = "1.10.0"
    license = "https://github.com/gabime/spdlog/blob/v1.x/LICENSE"
    url = "https://github.com/gabime/spdlog"
    description = "Fast C++ logging library."
    settings = "os", "compiler", "build_type", "arch"
    options = {
        'shared':[True, False],
        'wchar_filename': [True, False],
        'wchar_api': [True, False]
    }
    default_options = "shared=True", "wchar_filename=False", "wchar_api=False"
    generators = "cmake"
    exports_sources = [f'spdlog-{version}/*']

    def source(self):
        tools.get(url=f'https://github.com/gabime/spdlog/archive/refs/tags/v{self.version}.tar.gz')

    def configure_cmake(self):
        cmake = CMake(self)

        cmake.definitions['SPDLOG_BUILD_EXAMPLE'] = 'OFF'
        cmake.definitions['SPDLOG_BUILD_SHARED'] = 'ON' if self.options.shared == True else 'OFF'
        
        cmake.configure(source_folder=f'{self.source_folder}/spdlog-{self.version}', build_folder= f'{self.build_folder}/build' )
        return cmake

    def build(self):
        cmake = self.configure_cmake()
        cmake.build()

    def package(self):
        cmake = self.configure_cmake()
        cmake.install()
        
    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
        self.output.info(f'LIBS : {self.cpp_info.libs}')