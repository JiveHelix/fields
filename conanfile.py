from conans import ConanFile, CMake


class FieldsConan(ConanFile):
    name = "fields"
    version = "1.0.0"

    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto"}

    # Optional metadata
    license = "MIT"
    author = "Jive Helix (jivehelix@gmail.com)"
    url = "https://github.com/JiveHelix/fields"
    description = "Automatic conversion to structured and unstructured data."
    topics = ("C++", "Serialization")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    options = {
        "tests": [True, False],
        "examples": [True, False]}

    default_options = {
        "tests": True,
        "examples": False}

    generators = "cmake"

    no_copy_source = True

    default_user = "local"
    default_channel = "testing"

    def package_info(self):
        self.user_info.BUILD_EXAMPLES = self.options.examples


    def build(self):
        cmake = CMake(self)

        # defines = {
        #     "ENABLE_TESTS": self.options.tests,
        #     "BUILD_EXAMPLES": self.options.examples}

        # print("defines: {}".format(defines))

        # cmake.configure(defs=defines)
        cmake.configure()
        cmake.build()
        cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_id(self):
        self.info.header_only()

    def build_requirements(self):
        self.test_requires("catch2/2.13.8")
        self.build_requires("nlohmann_json/3.10.5")

    def requirements(self):
        jive = "jive/1.0.0"

        if self.user and self.channel:
            self.requires(jive + "@{}/{}".format(self.user, self.channel))
        else:
            self.requires(jive)
