#include <Kernel/Logger.h>
#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

}

TEST(LoggerTest, WritesToConsoleAndLogFile) {
    std::ostringstream console;
    std::streambuf* originalBuffer = std::cout.rdbuf(console.rdbuf());

    Logger* logger = Logger::getInstance();
    logger->out("unit-out");
    logger->warning("unit-warning");
    logger->error("unit-error");

    std::cout.rdbuf(originalBuffer);

    const std::string consoleText = console.str();
    EXPECT_NE(consoleText.find(">unit-out\n"), std::string::npos);
    EXPECT_NE(consoleText.find("unit-warning"), std::string::npos);
    EXPECT_NE(consoleText.find("unit-error"), std::string::npos);

    Logger::destroy();
    const std::string logText = readFile("log");
    EXPECT_NE(logText.find("--Minerva's log--"), std::string::npos);
    EXPECT_NE(logText.find(">unit-out"), std::string::npos);
    EXPECT_NE(logText.find("[WARNING]unit-warning"), std::string::npos);
    EXPECT_NE(logText.find("[ERROR]unit-error"), std::string::npos);
}
