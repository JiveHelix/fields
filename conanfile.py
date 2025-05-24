from cmake_includes.conan import HeaderConanFile


class FieldsConan(HeaderConanFile):
    name = "fields"
    version = "1.4.0"

    # Optional metadata
    license = "MIT"
    author = "Jive Helix (jivehelix@gmail.com)"
    url = "https://github.com/JiveHelix/fields"
    description = "Automatic conversion to structured and unstructured data."
    topics = ("C++", "Serialization")

    no_copy_source = True

    def build_requirements(self):
        self.test_requires("catch2/2.13.8")
        self.test_requires("nlohmann_json/[~3.11]")
        self.build_requires("nlohmann_json/[~3.11]")

    def requirements(self):
        self.requires("jive/[~1.3]", transitive_headers=True)
