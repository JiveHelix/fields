from conans import ConanFile, CMake


class FieldsConan(ConanFile):
    name = "fields"
    version = "1.3.4"

    scm = {
        "type": "git",
        "url": "https://github.com/JiveHelix/fields.git",
        "revision": "auto",
        "submodule": "recursive"}

    # Optional metadata
    license = "MIT"
    author = "Jive Helix (jivehelix@gmail.com)"
    url = "https://github.com/JiveHelix/fields"
    description = "Automatic conversion to structured and unstructured data."
    topics = ("C++", "Serialization")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    generators = "cmake"

    options = {
        "CMAKE_TRY_COMPILE_TARGET_TYPE":
            ["EXECUTABLE", "STATIC_LIBRARY", None]}

    default_options = {
        "CMAKE_TRY_COMPILE_TARGET_TYPE": None}

    no_copy_source = True

    def build(self):
        cmake = CMake(self)

        if (self.options.CMAKE_TRY_COMPILE_TARGET_TYPE):
            cmake.definitions["CMAKE_TRY_COMPILE_TARGET_TYPE"] = \
                self.options.CMAKE_TRY_COMPILE_TARGET_TYPE

        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_id(self):
        self.info.header_only()

    def build_requirements(self):
        self.test_requires("catch2/2.13.8")
        self.build_requires("nlohmann_json/[~3.11]")

    def requirements(self):
        self.requires("jive/[~1.1]")
