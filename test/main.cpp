/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include <gtest/gtest.h>
#include <cstdlib>

TEST(user_custom, test_1) {
  const char *value = "";
  char *end = nullptr;
  auto result = std::strtoul(value, &end, 10);
  EXPECT_EQ(result, decltype(result) { 0 });
  EXPECT_NE(end, nullptr);
  EXPECT_EQ(*end, '\0');
}

TEST(user_custom, test_2) {
  const char *value = "   123";
  char *end = nullptr;
  auto result = std::strtoul(value, &end, 10);
  EXPECT_EQ(result, decltype(result) { 123 });
  EXPECT_NE(end, nullptr);
  EXPECT_EQ(*end, '\0');
}

TEST(user_custom, test_3) {
  const char *value = "   123:abc";
  char *end = nullptr;
  auto result = std::strtoul(value, &end, 10);
  EXPECT_EQ(result, decltype(result) { 123 });
  EXPECT_NE(end, nullptr);
  EXPECT_STREQ(end, ":abc");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
