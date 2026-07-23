#include <Factories/MAOFactory.h>
#include <Kernel/Parsers/MSLParser.h>
#include <Kernel/Parsers/MSLProperties.h>
#include <Kernel/World.h>
#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <string>

namespace {

class CinRedirect {
public:
    explicit CinRedirect(std::istream& input)
        : previousBuffer(std::cin.rdbuf(input.rdbuf())) {
        std::cin.clear();
    }

    ~CinRedirect() {
        std::cin.rdbuf(previousBuffer);
        std::cin.clear();
    }

private:
    std::streambuf* previousBuffer;
};

TEST(MSLPropertiesTest, InitializesAndFillsNonDefaultParserValues) {
    MSLProperties defaults;
    EXPECT_EQ(defaults.string1, nullptr);
    EXPECT_FLOAT_EQ(defaults.float1, 0.0F);
    EXPECT_EQ(defaults.int1, 0);
    EXPECT_FALSE(defaults.bool1);
    EXPECT_EQ(defaults.btvector1, nullptr);
    EXPECT_EQ(defaults.pose1, nullptr);

    std::string name = "parsed";
    btVector3 color(10.0F, 20.0F, 30.0F);
    MSLProperties values;
    values.string1 = &name;
    values.float1 = 2.5F;
    values.int1 = 7;
    values.bool1 = true;
    values.btvector1 = &color;

    MSLProperties copy(values);
    EXPECT_EQ(copy.string1, &name);
    EXPECT_FLOAT_EQ(copy.float1, 2.5F);
    EXPECT_EQ(copy.int1, 7);
    EXPECT_TRUE(copy.bool1);
    EXPECT_EQ(copy.btvector1, &color);
}

TEST(MSLParserTest, ParsesApplicationAndRenderableSemanticActions) {
    MAORenderable3DLine& reference =
        MAOFactory::getInstance()->addMAORenderable3DLine(
            "parserReference", 1.0F, 1, 2, 3);
    std::istringstream input(
        "Application \"Bundle M\"\n"
        "Path3D parsedPath {\n"
        "size = 2.5\n"
        "color = (10, 20, 30)\n"
        "reference = parserReference\n"
        "}\n");
    CinRedirect redirect(input);

    MSLParser parser;
    ASSERT_EQ(parser.yyparse(), 0);

    EXPECT_EQ(World::getInstance()->getAppName(), "Bundle M");
    MAORenderable3DPath& parsed =
        static_cast<MAORenderable3DPath&>(
            MAOFactory::getInstance()->getMAORenderable3D("parsedPath"));
    EXPECT_FLOAT_EQ(parsed.getSize(), 2.5F);
    EXPECT_EQ(parsed.getR(), 10);
    EXPECT_EQ(parsed.getG(), 20);
    EXPECT_EQ(parsed.getB(), 30);
    EXPECT_EQ(parsed.getGlobalReference(), &reference);
}

}  // namespace
