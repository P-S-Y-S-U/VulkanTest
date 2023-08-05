from email.policy import default
from logging.handlers import DEFAULT_TCP_LOGGING_PORT
from conans import ConanFile, MSBuild, CMake, tools
from conans.client.build.compiler_flags import build_type_define, build_type_flags
from conans.errors import ConanInvalidConfiguration
import os

from requests import options

# class <pak_name>Conan(ConanFile): #
class StbConan(ConanFile):
    name = "stb"
    version = "master"
    license = "https://github.com/nothings/stb/blob/master/LICENSE"
    url = "https://github.com/nothings/stb"
    description = "stb single-file public domain libraries for C/C++"
    settings = "os", "compiler", "build_type", "arch"

    @property
    def _is_msvsc(self):
        return self.settings.os == 'Windows'
    
    @property
    def _is_linux(self):
        return self.settings.os == 'linux'

    @property
    def _lib_source_folder(self):
        return "stb"

    def source(self):
        git = tools.Git(folder=self._lib_source_folder)
        git.clone(url='https://github.com/nothings/stb.git')

    def build(self):
        self.output.info("Nothing to build!")

    def package(self):
        self.copy( "*.h", src=f"{self.source_folder}/stb", dst="include/stb", excludes=["deprecated", "tests"] )

    def deploy(self):
        self.copy("*", src="include", dst="include")

    def package_info(self):
        self.output.info("Header Only Library")
    
    def package_id(self):
        self.info.header_only()