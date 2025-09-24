from conan import ConanFile


class FieldsConan(ConanFile):
    name = "fields"
    version = "1.6.0"

    python_requires = "boiler/0.1"
    python_requires_extend = "boiler.HeaderConanFile"

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
        self.requires("jive/[>=1.4 <2]", transitive_headers=True)
