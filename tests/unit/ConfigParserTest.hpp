#include <gtest/gtest.h>
#include <fstream>
#include "../../src/Config/ConfigParser.hpp"
#include "../../src/Utils/StringUtils.hpp"

// Add the header content at the top or in your test fixture setup
#include <gtest/gtest.h>
#include <string>
#include <exception>

// Custom macro for checking exception message
#define EXPECT_THROW_WITH_MESSAGE(statement, expected_exception, expected_message)  \
    try                                                                             \
    {                                                                               \
        statement;                                                                  \
        FAIL() << "Expected " #expected_exception " to be thrown";                  \
    }                                                                               \
    catch (const expected_exception &e)                                             \
    {                                                                               \
        SUCCEED(std::string(e.what()).find(expected_message) != std::string::npos); \
        std::cout << "Caught exception: " << e.what() << std::endl;                 \
    }                                                                               \
    catch (...)                                                                     \
    {                                                                               \
        FAIL() << "Expected " #expected_exception " to be thrown";                  \
    }
// #define EXPECT_THROW_WITH_MESSAGE(statement, exception_type, expected_message) \
//     EXPECT_THROW({ \
//         try { \
//             statement; \
//         } catch (const exception_type& e) { \
//             std::cout << "\n===============================================" << std::endl; \
//             std::cout << "EXCEPTION: " << e.what() << std::endl; \
//             std::cout << "===============================================\n" << std::endl; \
//             EXPECT_TRUE(std::string(e.what()).find(expected_message) != std::string::npos) \
//                 << "Expected substring: " << expected_message << "\n" \
//                 << "Actual message: " << e.what(); \
//             throw; \
//         } }, exception_type)

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

    void SetUp() override
    {
        // Create a file with invalid syntax for the specific test
        std::ofstream file(invalidSyntaxPath);
        file << "server {\n    listen  ;\n}\n";
        file.close();

        // Set up the valid parser that multiple tests will use
        validParser = new ConfigParser(validConfigPath);
        validParser->extractServerConfigs();
        configs = validParser->getServerConfigs();
    }

    void TearDown() override
    {
        delete validParser;
        // Clean up test files
        std::remove(invalidSyntaxPath.c_str());
    }
};

class TestConfigFile
{
private:
    std::string filePath;

public:
    TestConfigFile(const std::string &path, const std::string &content) : filePath(path)
    {
        std::ofstream file(filePath);
        file << content;
        file.close();
    }

    ~TestConfigFile()
    {
        if (!filePath.empty())
        {
            std::remove(filePath.c_str());
        }
    }

    // Get the path for use in the test
    const std::string &path() const { return filePath; }

    // Non-copyable
    TestConfigFile testFile(const TestConfigFile &) = delete;
    TestConfigFile &operator=(const TestConfigFile &) = delete;
};