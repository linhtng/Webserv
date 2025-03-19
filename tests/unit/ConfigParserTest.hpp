#include <gtest/gtest.h>
#include <fstream>
#include "../../src/Config/ConfigParser.hpp"
#include "../../src/Utils/StringUtils.hpp"

// Add the header content at the top or in your test fixture setup
#include <gtest/gtest.h>
// #include <string>
#include <exception>

class ConfigParserTest : public ::testing::Test
{
protected:
    // Paths shared by all tests
    std::string validConfigPath = "configs/test_files/testValid.conf";
    std::string invalidConfigPath = "configs/test_files/testInvalid.conf";
    std::string nonexistentPath = "configs/test_files/nonexistent.conf";
    std::string invalidSyntaxPath = "configs/test_files/invalidSyntax.conf";

    // Reusable parser and configs objects
    ConfigParser *validParser = nullptr;
    std::vector<ConfigData> configs;

    void SetUp() override;
    void TearDown() override;

    // Helper method for testing exceptions
    void ExpectThrowsWithMessage(const std::string &testName,
                                 const std::string &configContent);

    void ExpectThrowsFromFile(
        const std::string &description, std::string &filePath);
};

class TestConfigFile
{
private:
    std::string filePath;

public:
    TestConfigFile(const std::string &path, const std::string &content);
    ~TestConfigFile();

    // Get the path for use in the test
    const std::string &path() const;

    // Non-copyable
    TestConfigFile(const TestConfigFile &) = delete;
    TestConfigFile &operator=(const TestConfigFile &) = delete;
};