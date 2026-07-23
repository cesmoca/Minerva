#include <Factories/MAOFactory.h>
#include <gtest/gtest.h>

#include <string>

TEST(MAOFactoryTest, RegistersTypedObjectsAndResolvesReferences) {
    MAOFactory factory;

    MAORenderable3DLine& reference =
        factory.addMAORenderable3DLine("reference", 1.0F, 10, 20, 30);
    std::string referenceName = "reference";
    MAORenderable3DPath& path = factory.addMAORenderable3DPath(
        "path", 2.0F, 40, 50, 60, referenceName);
    MAOMarksGroup& group = factory.addMAOMarksGroup("group");

    reference.addPropertyInt("count", 7);

    EXPECT_EQ(&factory.getMAO("reference"), &reference);
    EXPECT_EQ(&factory.getMAOPositionator3D("reference"), &reference);
    EXPECT_EQ(&factory.getMAORenderable3D("reference"), &reference);
    EXPECT_EQ(&factory.getMAOMarksGroup("group"), &group);
    EXPECT_EQ(path.getGlobalReference(), &reference);
    EXPECT_EQ(factory.findProperty("reference", "count").getValue<int>(), 7);

    EXPECT_EQ(factory.getVectorMAORenderable3D().size(), 2U);
    EXPECT_EQ(factory.getVectorMAOPositionator3D().size(), 3U);
    EXPECT_TRUE(factory.getVectorMAORenderable2D().empty());
}

TEST(MAOFactoryTest, CreatesLinkedLineAndReportsInvalidLookups) {
    MAOFactory factory;

    factory.addMAORenderable3DLine("first", 1.0F, 1, 2, 3);
    factory.addMAORenderable3DLine("second", 1.0F, 4, 5, 6);
    std::string firstName = "first";
    std::string secondName = "second";

    EXPECT_NO_THROW(factory.addMAORenderable3DLine(
        "linked", 2.0F, 7, 8, 9, firstName, secondName));
    EXPECT_THROW(
        factory.addMAORenderable3DLine("first", 3.0F, 10, 11, 12),
        std::string);

    EXPECT_ANY_THROW(factory.getMAO("missing"));
    EXPECT_ANY_THROW(factory.getMAOPositionator3D("missing"));
    EXPECT_ANY_THROW(factory.getMAOMark("missing"));
    EXPECT_ANY_THROW(factory.getMAOMarksGroup("missing"));
    EXPECT_ANY_THROW(factory.getMAORenderable3D("missing"));
    EXPECT_ANY_THROW(factory.getMAORenderable2D("missing"));
    EXPECT_THROW(factory.findProperty("missing", "value"), std::string);
}

TEST(MAOFactoryTest, OwnsStandaloneInstantiatedRenderable) {
    MAOFactory factory;
    auto* instantiated =
        new MAORenderable3DLine("instantiated", 1.0F, 10, 20, 30);

    factory.addInstMAORenderable3D(*instantiated, 5);

    ASSERT_EQ(factory.getVectorInstMAORenderable3D().size(), 1U);
    EXPECT_EQ(factory.getVectorInstMAORenderable3D().front(), instantiated);
    EXPECT_EQ(instantiated->getTimeToExpire(), 5);
}
