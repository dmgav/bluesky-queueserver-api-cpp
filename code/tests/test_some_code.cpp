#include <gtest/gtest.h>
#include "../include/some_code.h"

TEST(SomeCode, BasicTestA) {
  EXPECT_EQ(add_two_numbers(2, 5), 7);
  // EXPECT_EQ(add_two_numbers(3, 3), 5);  // Fail
  EXPECT_EQ(add_two_numbers(3, 9), 12);
}

TEST(SomeCode, BasicTestB) {
  EXPECT_EQ(add_two_numbers(2, 5), 7);
  EXPECT_EQ(add_two_numbers(3, 9), 12);
}
