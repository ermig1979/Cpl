import os

from conan import ConanFile
from conan.errors import ConanException
from conan.tools.files import copy, load, save
from conan.tools.scm import Git

# "auto_header_only" in `implements` exists since Conan 2.0.9; older Conan ignores the
# attribute silently and would produce split package ids under the same recipe revision.
required_conan_version = ">=2.0.9"


class CplConan(ConanFile):
    name = "cpl"
    license = "MIT"
    author = "Ermig1979"
    url = "https://github.com/ermig1979/Cpl"
    description = "Common Purpose Library — header-only C++ utility library"
    topics = ("header-only", "utility", "c++")
    package_type = "header-library"
    settings = "os", "compiler", "build_type", "arch"
    # Clears the package id (Conan does it only when BOTH hold: package_type is
    # "header-library" AND "auto_header_only" is listed here; a package_id() method
    # would override this). Without it every profile axis (compiler.cppstd,
    # build_type, compiler.version, ...) splits the id of a package
    # whose content is the same headers. package() must therefore stay
    # settings-independent: one id serves every profile.
    implements = ["auto_header_only"]
    no_copy_source = True

    def _repo_root(self):
        return os.path.normpath(os.path.join(self.recipe_folder, "..", ".."))

    def set_version(self):
        # In cache — read from file saved during export()
        version_file = os.path.join(self.recipe_folder, "version.txt")
        if os.path.exists(version_file):
            self.version = load(self, version_file).strip()
            return

        # In local flow — read from git tag
        try:
            git = Git(self, folder=self._repo_root())
            tag = git.run("describe --tags --abbrev=0").strip().lstrip("v")
        except ConanException:
            raise ConanException(
                "No git tags found. Create a version tag first:\n"
                "  git tag v1.0.0\n"
                "If tags exist on remote, fetch them:\n"
                "  git fetch --tags"
            )
        if not tag:
            raise ConanException(
                "No git tags found. Create a version tag first:\n"
                "  git tag v1.0.0\n"
                "If tags exist on remote, fetch them:\n"
                "  git fetch --tags"
            )
        self.version = tag

    def export(self):
        save(self, os.path.join(self.export_folder, "version.txt"), self.version)

    def export_sources(self):
        root = self._repo_root()
        copy(self, "*.h", src=os.path.join(root, "src", "Cpl"), dst=os.path.join(self.export_sources_folder, "src", "Cpl"))

    def package(self):
        copy(
            self, "*.h",
            src=os.path.join(self.source_folder, "src", "Cpl"),
            dst=os.path.join(self.package_folder, "include", "Cpl"),
        )

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        if self.settings.os in ("Linux", "FreeBSD"):
            self.cpp_info.system_libs.extend(["pthread", "stdc++fs"])
