#include "ConfigParserTest.hpp"

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

TEST_F(ConfigParserTest, ThrowsOnDuplicateDirective)
{
    std::string duplicateServerPath = "configs/test_files/testDuplicateServer.conf";
    TestConfigFile testFile(duplicateServerPath,
                            "server {\n"
                            "    listen 10001;\n"
                            "    listen 10001;\n"
                            "    server_name duplicateListen;\n"
                            "}\n");
    // try
    // {
    //     ConfigParser parser(duplicateServerPath);
    //     parser.extractServerConfigs();
    // }
    // catch (const std::exception &e)
    // {
    //     std::cerr << e.what() << '\n';
    // }
    // The original test approach (without printing)
    EXPECT_THROW_WITH_MESSAGE({
        ConfigParser parser(duplicateServerPath);
        parser.extractServerConfigs(); }, std::runtime_error, "Duplicate directive key: listen");
}

// TEST_F(ConfigParserTest, ThrowsOnNegativeBodySize)
// {
//     std::string negativeBodySizePath = "configs/test_files/testNegativeBodySize.conf";
//     TestConfigFile testFile(negativeBodySizePath,
//                             "server {\n"
//                             "    listen 10002;\n"
//                             "    server_name negativeBodySizeServer;\n"
//                             "    client_max_body_size -100;\n"
//                             "}\n");
//     try
//     {
//         ConfigParser parser(negativeBodySizePath);
//         parser.extractServerConfigs();
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << e.what() << '\n';
//     }

//     // EXPECT_THROW({
//     //     ConfigParser parser(invalidPortPath);
//     //     parser.extractServerConfigs(); }, std::exception);
// }

// TEST_F(ConfigParserTest, ThrowsOnInvalidPort)
// {
//     std::string invalidPortPath = "configs/test_files/testInvalidPort.conf";
//     TestConfigFile testFile(invalidPortPath,
//                             "server {\n"
//                             "    listen abc;\n"
//                             "    server_name invalidPort;\n"
//                             "}\n");
//     try
//     {
//         ConfigParser parser(invalidPortPath);
//         parser.extractServerConfigs();
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << e.what() << '\n';
//     }

//     // EXPECT_THROW({
//     //     ConfigParser parser(invalidPortPath);
//     //     parser.extractServerConfigs(); }, std::exception);
// }

// TEST_F(ConfigParserTest, ThrowsOnPortOutOfRange)
// {
//     std::string portRangePath = "configs/test_files/testPortRange.conf";
//     TestConfigFile testFile(portRangePath,
//                             "server {\n"
//                             "    listen 99999;\n" // Port > 65535
//                             "    server_name portOutrange;\n"
//                             "}\n");

//     try
//     {
//         ConfigParser parser(portRangePath);
//         parser.extractServerConfigs();
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << e.what() << '\n';
//     }

//     // EXPECT_THROW({
//     //     ConfigParser parser(invalidPortPath);
//     //     parser.extractServerConfigs(); }, std::exception);
// }

// TEST_F(ConfigParserTest, ThrowsOnInvalidCgiExtension)
// {
//     std::string invalidCgiPath = "configs/test_files/testInvalidCgi.conf";
//     TestConfigFile testFile(invalidCgiPath,
//                             "server {\n"
//                             "    listen 10001;\n"
//                             "    server_name cgiTestExten;\n"
//                             "    cgi_extension .invalid;\n" // Not a valid extension
//                             "    cgi_executor /usr/bin/python3;\n"
//                             "}\n");

//     try
//     {
//         ConfigParser parser(invalidCgiPath);
//         parser.extractServerConfigs();
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << e.what() << '\n';
//     }

//     // EXPECT_THROW({
//     //     ConfigParser parser(invalidPortPath);
//     //     parser.extractServerConfigs(); }, std::exception);
// }

// TEST_F(ConfigParserTest, ThrowsOnMismatchedCgiExtensionAndExecutor)
// {
//     std::string mismatchCgiPath = "configs/test_files/testMismatchCgi.conf";
//     TestConfigFile testFile(mismatchCgiPath,
//                             "server {\n"
//                             "    listen 10001;\n"
//                             "    server_name mismatchCgiServer;\n"
//                             "    cgi_exten .sh;\n" // This will throw
//                             // "    cgi_exten .py;\n"                 // This will not throw
//                             "    cgi_executor /usr/bin/python3;\n" // Mismatched order
//                             "}\n");
//     try
//     {
//         ConfigParser parser(mismatchCgiPath);
//         parser.extractServerConfigs();
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << e.what() << '\n';
//     }
//     EXPECT_THROW({
//         ConfigParser parser(mismatchCgiPath);
//         parser.extractServerConfigs(); }, std::exception);
// }

// TEST_F(ConfigParserTest, HandlesMalformedDirectives)
// {
//     std::string malformedPath = "configs/test_files/testMalformedDirectives.conf";
//     TestConfigFile testFile(malformedPath,
//                             "server {\n"
//                             "    listen 10002\n" // Missing semicolon
//                             "    server_name malformedServer;\n"
//                             "}\n");

//     try
//     {
//         ConfigParser parser(malformedPath);
//         parser.extractServerConfigs();
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << e.what() << '\n';
//     }

//     // EXPECT_THROW({
//     //     ConfigParser parser(invalidPortPath);
//     //     parser.extractServerConfigs(); }, std::exception);
// }

// TEST_F(ConfigParserTest, HandlesValidErrorPages)
// {
//     std::string errorPagesPath = "configs/test_files/testErrorPages.conf";
//     TestConfigFile testFile(errorPagesPath,
//                             "server {\n"
//                             "    listen 10001;\n"
//                             "    server_name errorPagesServer;\n"
//                             "    error_page 404 /error/404.html;\n"
//                             "    error_page 500 /error/500.html;\n"
//                             "}\n");

//     ConfigParser parser(errorPagesPath);
//     parser.extractServerConfigs();
//     auto configs = parser.getServerConfigs();
//     ASSERT_GT(configs.size(), 0U);
//     auto errorPages = configs[0].getErrorPages();
//     auto errorPagesResult1 = StringUtils::trim(errorPages.at(404));
//     auto errorPagesResult2 = StringUtils::trim(errorPages.at(500));
//     // std::cout << "Error pages: " << errorPagesResult1 << " " << errorPagesResult2 << std::endl;
//     EXPECT_EQ(errorPagesResult1, "/error/404.html");
//     EXPECT_EQ(errorPagesResult2, "/error/500.html");
// }

// TEST_F(ConfigParserTest, ThrowsOnInvalidErrorCode)
// {
//     std::string invalidErrorPath = "configs/test_files/testInvalidErrorCode.conf";
//     TestConfigFile testFile(invalidErrorPath,
//                             "server {\n"
//                             "    listen 10001;\n"
//                             "    server_name InvalidErrorCode;\n"
//                             "    error_page 999 /error/999.html;\n" // Invalid error code
//                             "}\n");

//     try
//     {
//         ConfigParser parser(invalidErrorPath);
//         parser.extractServerConfigs();
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << e.what() << '\n';
//     }

//     // EXPECT_THROW({
//     //     ConfigParser parser(invalidPortPath);
//     //     parser.extractServerConfigs(); }, std::exception);
// }

// TEST_F(ConfigParserTest, TestNoGivenPort)
// {
//     std::string testMissingPort = "configs/test_files/testMissingPort.conf";
//     TestConfigFile testFile(testMissingPort,
//                             "server {\n"
//                             "    server_name TestNoGivenPort;\n"
//                             "    root pages;\n"
//                             "}\n");
//     // Now you can pass testFile directly to ConfigParser
//     ConfigParser parser(testMissingPort);
//     parser.extractServerConfigs();
//     auto configs = parser.getServerConfigs();
//     ASSERT_GT(configs.size(), 0U);
//     EXPECT_EQ(configs[0].getServerPort(), DefaultValues::PORT);
// }