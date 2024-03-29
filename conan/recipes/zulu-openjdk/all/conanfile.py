import os

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.files import get, copy


class ZuluOpenJDK(ConanFile):
    name = "zulu-openjdk"
    url = "https://github.com/conan-io/conan-center-index/"
    description = "A OpenJDK distribution"
    homepage = "https://www.azul.com"
    license = "https://www.azul.com/products/zulu-and-zulu-enterprise/zulu-terms-of-use/"
    topics = ("java", "jdk", "openjdk")
    settings = "os", "arch"
    package_type = "application"

    @property
    def _settings_build(self):
        return getattr(self, "settings_build", self.settings)

    @property
    def _jni_folder(self):
        folder = {"Linux": "linux", "Macos": "darwin", "Windows": "win32"}.get(str(self._settings_build.os))
        return os.path.join("include", folder)


    def validate(self):
        supported_archs = ["x86_64", "armv8"]
        if self._settings_build.arch not in supported_archs:
            raise ConanInvalidConfiguration(f"Unsupported Architecture ({self._settings_build.arch}). "
                                             "This version {self.version} currently only supports {supported_archs}.")
        supported_os = ["Windows", "Macos", "Linux"]
        if self._settings_build.os not in supported_os:
            raise ConanInvalidConfiguration(f"Unsupported os ({self._settings_build.os}). "
                                             "This package currently only support {supported_os}.")

    def build(self):
        get(self, **self.conan_data["sources"][self.version][str(self._settings_build.os)][str(self._settings_build.arch)], strip_root=True)

    def package(self):
        copy(self, pattern="*", dst=os.path.join(self.package_folder, "bin"),
             src=os.path.join(self.source_folder, "bin"), 
             excludes=("msvcp140.dll", "vcruntime140.dll"))
        copy(self, pattern="*", dst=os.path.join(self.package_folder, "include"), 
             src=os.path.join(self.source_folder, "include"))
        copy(self, pattern="*", dst=os.path.join(self.package_folder, "lib"), 
             src=os.path.join(self.source_folder, "lib"))
        copy(self, pattern="*", dst=os.path.join(self.package_folder, "res"), 
             src=os.path.join(self.source_folder, "conf"))
        # conf folder is required for security settings, to avoid
        # java.lang.SecurityException: Can't read cryptographic policy directory: unlimited
        # https://github.com/conan-io/conan-center-index/pull/4491#issuecomment-774555069
        copy(self, pattern="*", dst=os.path.join(self.package_folder, "conf"), 
             src=os.path.join(self.source_folder, "conf"))
        copy(self, pattern="*", dst=os.path.join(self.package_folder, "licenses"), 
             src=os.path.join(self.source_folder, "legal"))
        copy(self, pattern="*", dst=os.path.join(self.package_folder, "lib", "jmods"), 
             src=os.path.join(self.source_folder, "jmods"))
        copy(self, pattern="*", dst=os.path.join(self.package_folder, "jre"), 
             src=os.path.join(self.source_folder, "jre"))

    def package_info(self):
        self.output.info(f"Creating JAVA_HOME environment variable with : {self.package_folder}")

        self.cpp_info.includedirs.append(self._jni_folder)
        self.cpp_info.libdirs = []

        self.runenv_info.define_path("JAVA_HOME", self.package_folder)
        self.buildenv_info.define_path("JAVA_HOME", self.package_folder)

        self.runenv_info.define_path("JDK_HOME", self.package_folder)
        self.buildenv_info.define_path("JDK_HOME", self.package_folder)

        # TODO: remove `env_info` once the recipe is only compatible with Conan >= 2.0
        self.env_info.JAVA_HOME = self.package_folder
        self.env_info.PATH.append(os.path.join(self.package_folder, "bin"))

