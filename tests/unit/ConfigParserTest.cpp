#include <gtest/gtest.h>
#include <fstream>
#include "../../src/Config/ConfigParser.hpp"
#include "../../src/Utils/StringUtils.hpp"

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

// Using the fixture for all tests (note TEST_F instead of TEST)
TEST_F(ConfigParserTest, ParsesValidConfig)
{
    ASSERT_GT(configs.size(), 0U);
    EXPECT_EQ(configs[0].getServerName(), "testServerName");
}

TEST_F(ConfigParserTest, ThrowsOnInvalidConfig)
{
    EXPECT_THROW({
        ConfigParser parser(invalidConfigPath);
        parser.extractServerConfigs(); }, std::exception);
}

TEST_F(ConfigParserTest, ParsesServerPort)
{
    ASSERT_GT(configs.size(), 0U);
    EXPECT_EQ(configs[0].getServerPort(), 10001);
}

/* There could be many space characters in the error pages path string, so we need to trim it
 */
TEST_F(ConfigParserTest, ParsesErrorPages)
{
    ASSERT_GT(configs.size(), 0U);
    auto errorPages = configs[0].getErrorPages();
    EXPECT_EQ(StringUtils::trim(errorPages.at(404)), "pages/404.html");
    EXPECT_EQ(StringUtils::trim(errorPages.at(421)), "pages/421.html");
}

TEST_F(ConfigParserTest, ParsesMaxClientBodySize)
{
    ASSERT_GT(configs.size(), 0U);
    EXPECT_EQ(configs[0].getMaxClientBodySize(), 10UL * 1024UL * 1024UL);
}

TEST_F(ConfigParserTest, ParsesCgiSettings)
{
    ASSERT_GT(configs.size(), 0U);
    EXPECT_EQ(configs[0].getCgiDir(), "./cgi-bin/");
    EXPECT_EQ(configs[0].getCgiExtension().size(), 2U);
    EXPECT_EQ(configs[0].getCgiExtension()[0], ".py");
    EXPECT_EQ(configs[0].getCgiExecutor().size(), 2U);
    EXPECT_EQ(configs[0].getCgiExecutor()[0], "/usr/bin/python3");
}

TEST_F(ConfigParserTest, ParsesLocations)
{
    ASSERT_GT(configs.size(), 0U);

    auto locations = configs[0].getLocations();
    EXPECT_EQ(locations.size(), 2U);

    auto rootLocation = configs[0].getMatchingLocation("/");
    EXPECT_EQ(rootLocation.getLocationRoot(), "pages0");

    auto uploadLocation = configs[0].getMatchingLocation("/upload");
    EXPECT_EQ(uploadLocation.getDefaultFile(), "uploadPage.html");

    auto methods = rootLocation.getAcceptedMethods();
    EXPECT_EQ(methods.size(), 4U);
    EXPECT_NE(methods.find(HttpMethod::DELETE), methods.end());
    EXPECT_NE(methods.find(HttpMethod::GET), methods.end());
}

TEST_F(ConfigParserTest, ThrowsOnNonexistentFile)
{
    EXPECT_THROW({
        ConfigParser parser(nonexistentPath);
        parser.extractServerConfigs(); }, std::exception);
}

TEST_F(ConfigParserTest, ThrowsOnInvalidSyntax)
{
    EXPECT_THROW({
        ConfigParser parser(invalidSyntaxPath);
        parser.extractServerConfigs(); }, std::exception);
}