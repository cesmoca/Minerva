#include <MPY/WrapperTypes.h>
#include <gtest/gtest.h>

TEST(WrapperTypesTest, GetsItemsUsingPositiveAndNegativeIndexes) {
    VectorInt values{10, 20, 30};

    EXPECT_EQ(vector_item<int>::get(values, 0), 10);
    EXPECT_EQ(vector_item<int>::get(values, -1), 30);
}

TEST(WrapperTypesTest, AddsAndDeletesItems) {
    VectorStr values{"first", "second", "third"};

    vector_item<std::string>::del(values, -1);
    ASSERT_EQ(values.size(), 2U);
    EXPECT_EQ(values[0], "first");
    EXPECT_EQ(values[1], "second");

    vector_item<std::string>::add(values, "last");
    ASSERT_EQ(values.size(), 3U);
    EXPECT_EQ(values[2], "last");
}
