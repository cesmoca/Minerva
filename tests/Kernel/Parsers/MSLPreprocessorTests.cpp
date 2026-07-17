#include <Kernel/Parsers/MSLPreprocessor.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace {

void writeFile(const std::filesystem::path& path, const std::string& contents) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file << contents;
}

}

TEST(MSLPreprocessorTest, PreservesOrdinaryLines) {
    writeFile("plain.mrv", "Application Demo");

    MSLPreprocessor preprocessor;
    std::stringstream output;
    preprocessor.start("plain.mrv", output);

    EXPECT_EQ(output.str(), "Application Demo\n");
    std::filesystem::remove("plain.mrv");
}

TEST(MSLPreprocessorTest, RemovesCommentsAndExpandsIncludes) {
    writeFile("child.mrv", "ChildLine");
    std::string mainContents = "Before\n// ignored comment\ninclude ";
    mainContents += static_cast<char>(34);
    mainContents += "child.mrv";
    mainContents += static_cast<char>(34);
    mainContents += "\nAfter";
    writeFile("main.mrv", mainContents);

    MSLPreprocessor preprocessor;
    std::stringstream includeOutput;
    preprocessor.start("main.mrv", includeOutput);
    EXPECT_EQ(includeOutput.str(), "Before\nChildLine\nAfter\n");

    std::filesystem::remove("child.mrv");
    std::filesystem::remove("main.mrv");
}

TEST(MSLPreprocessorTest, ReportsMissingFiles) {
    MSLPreprocessor preprocessor;
    EXPECT_THROW({
        std::stringstream missingOutput;
        preprocessor.start("missing.mrv", missingOutput);
    }, std::string);
}
