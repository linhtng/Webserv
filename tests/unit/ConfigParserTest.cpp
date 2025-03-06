#include <gtest/gtest.h>
#include "../../src/Config/ConfigParser.hpp"

TEST(ConfigParserTest, ParsesValidConfig)
{
    std::string configPath = "configs/test_files/testValid.conf";
    ConfigParser parser(configPath);
    parser.extractServerConfigs();
    auto configs = parser.getServerConfigs();
    ASSERT_GT(configs.size(), 0U);
    EXPECT_EQ(configs[0].getServerName(), "testServerName");
}

TEST(ConfigParserTest, ThrowsOnInvalidConfig)
{
    std::string invalidConfigPath = "configs/test_files/testInvalid.conf";
    EXPECT_THROW({
        ConfigParser parser(invalidConfigPath);
        parser.extractServerConfigs(); }, std::exception);
}