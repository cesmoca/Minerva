#include <Kernel/EndController.h>
#include <Kernel/Logger.h>
#include <Kernel/Parsers/MSLPreprocessor.h>
#include <Kernel/PathPoint.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

void writeFile(const std::filesystem::path& path, const std::string& contents) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file << contents;
}

std::string readFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

void testEndController() {
    EndController* controller = EndController::getInstance();
    expect(controller->isRunning(), "EndController starts running");
    controller->end();
    expect(!controller->isRunning(), "EndController stops after end");
}

void testPathPoint() {
    PathPoint point(-1.5F, 2.25F, 3.75F, 0.5F, 10, 20, 30, true);
    expect(point.getX() == -1.5F, "PathPoint preserves x");
    expect(point.getY() == 2.25F, "PathPoint preserves y");
    expect(point.getZ() == 3.75F, "PathPoint preserves z");
    expect(point.getSize() == 0.5F, "PathPoint preserves size");
    expect(point.getR() == 10, "PathPoint preserves red");
    expect(point.getG() == 20, "PathPoint preserves green");
    expect(point.getB() == 30, "PathPoint preserves blue");
    expect(point.getVisible(), "PathPoint preserves visible true");

    PathPoint hidden(0.0F, 0.0F, 0.0F, 0.0F, 0, 0, 0, false);
    expect(!hidden.getVisible(), "PathPoint preserves visible false");
}

void testLogger() {
    std::ostringstream console;
    std::streambuf* originalBuffer = std::cout.rdbuf(console.rdbuf());

    Logger* logger = Logger::getInstance();
    logger->out("unit-out");
    logger->warning("unit-warning");
    logger->error("unit-error");

    std::cout.rdbuf(originalBuffer);

    const std::string consoleText = console.str();
    expect(consoleText.find(">unit-out\n") != std::string::npos,
           "Logger writes normal output to stdout");
    expect(consoleText.find("unit-warning") != std::string::npos,
           "Logger writes warnings to stdout");
    expect(consoleText.find("unit-error") != std::string::npos,
           "Logger writes errors to stdout");

    Logger::destroy();
    const std::string logText = readFile("log");
    expect(logText.find("--Minerva's log--") != std::string::npos,
           "Logger creates the log header");
    expect(logText.find(">unit-out") != std::string::npos,
           "Logger writes normal output to the log file");
    expect(logText.find("[WARNING]unit-warning") != std::string::npos,
           "Logger writes warnings to the log file");
    expect(logText.find("[ERROR]unit-error") != std::string::npos,
           "Logger writes errors to the log file");
}

void testPreprocessor() {
    writeFile("plain.mrv", "Application Demo");
    writeFile("child.mrv", "ChildLine");
    std::string mainContents = "Before\n// ignored comment\ninclude ";
    mainContents += static_cast<char>(34);
    mainContents += "child.mrv";
    mainContents += static_cast<char>(34);
    mainContents += "\nAfter";
    writeFile("main.mrv", mainContents);

    MSLPreprocessor preprocessor;
    std::stringstream plainOutput;
    preprocessor.start("plain.mrv", plainOutput);
    expect(plainOutput.str() == "Application Demo\n",
           "MSLPreprocessor preserves ordinary lines");

    std::stringstream includeOutput;
    preprocessor.start("main.mrv", includeOutput);
    expect(includeOutput.str() == "Before\nChildLine\nAfter\n",
           "MSLPreprocessor removes comments and expands includes");

    bool missingFileThrown = false;
    try {
        std::stringstream missingOutput;
        preprocessor.start("missing.mrv", missingOutput);
    } catch (const std::string&) {
        missingFileThrown = true;
    }
    expect(missingFileThrown, "MSLPreprocessor reports missing files");

    std::filesystem::remove("plain.mrv");
    std::filesystem::remove("child.mrv");
    std::filesystem::remove("main.mrv");
}

}  // namespace

int main() {
    testEndController();
    testPathPoint();
    testPreprocessor();
    testLogger();

    EndController::destroy();

    if (failures != 0) {
        std::cerr << failures << " test assertion(s) failed\n";
        return 1;
    }

    std::cout << "All Minerva kernel unit tests passed\n";
    return 0;
}
