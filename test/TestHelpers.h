#pragma once

#include <string>
#include <gtest/gtest.h>

std::string GetAssetPath(const char* path);

#define EXPECT_QUAT_NEAR(val1, val2, error) \
  EXPECT_NEAR(val1[0], val2[0], error); \
  EXPECT_NEAR(val1[1], val2[1], error); \
  EXPECT_NEAR(val1[2], val2[2], error); \
  EXPECT_NEAR(val1[3], val2[3], error);

#define EXPECT_QUAT_EQ(val1, val2) \
  EXPECT_FLOAT_EQ(val1[0], val2[0]); \
  EXPECT_FLOAT_EQ(val1[1], val2[1]); \
  EXPECT_FLOAT_EQ(val1[2], val2[2]); \
  EXPECT_FLOAT_EQ(val1[3], val2[3]);

#define EXPECT_VEC3_NEAR(val1, val2, error) \
  EXPECT_NEAR(val1[0], val2[0], error); \
  EXPECT_NEAR(val1[1], val2[1], error); \
  EXPECT_NEAR(val1[2], val2[2], error);

#define EXPECT_VEC3_EQ(val1, val2) \
  EXPECT_FLOAT_EQ(val1[0], val2[0]); \
  EXPECT_FLOAT_EQ(val1[1], val2[1]); \
  EXPECT_FLOAT_EQ(val1[2], val2[2]);

#define EXPECT_MAT4_NEAR(val1, val2, error) \
  EXPECT_NEAR(val1[0][0], val2[0][0], error); \
  EXPECT_NEAR(val1[0][1], val2[0][1], error); \
  EXPECT_NEAR(val1[0][2], val2[0][2], error); \
  EXPECT_NEAR(val1[0][3], val2[0][3], error); \
  EXPECT_NEAR(val1[1][0], val2[1][0], error); \
  EXPECT_NEAR(val1[1][1], val2[1][1], error); \
  EXPECT_NEAR(val1[1][2], val2[1][2], error); \
  EXPECT_NEAR(val1[1][3], val2[1][3], error); \
  EXPECT_NEAR(val1[2][0], val2[2][0], error); \
  EXPECT_NEAR(val1[2][1], val2[2][1], error); \
  EXPECT_NEAR(val1[2][2], val2[2][2], error); \
  EXPECT_NEAR(val1[2][3], val2[2][3], error); \
  EXPECT_NEAR(val1[3][0], val2[3][0], error); \
  EXPECT_NEAR(val1[3][1], val2[3][1], error); \
  EXPECT_NEAR(val1[3][2], val2[3][2], error); \
  EXPECT_NEAR(val1[3][3], val2[3][3], error)

#define EXPECT_MAT4_EQ(val1, val2) \
  EXPECT_FLOAT_EQ(val1[0][0], val2[0][0]); \
  EXPECT_FLOAT_EQ(val1[0][1], val2[0][1]); \
  EXPECT_FLOAT_EQ(val1[0][2], val2[0][2]); \
  EXPECT_FLOAT_EQ(val1[0][3], val2[0][3]); \
  EXPECT_FLOAT_EQ(val1[1][0], val2[1][0]); \
  EXPECT_FLOAT_EQ(val1[1][1], val2[1][1]); \
  EXPECT_FLOAT_EQ(val1[1][2], val2[1][2]); \
  EXPECT_FLOAT_EQ(val1[1][3], val2[1][3]); \
  EXPECT_FLOAT_EQ(val1[2][0], val2[2][0]); \
  EXPECT_FLOAT_EQ(val1[2][1], val2[2][1]); \
  EXPECT_FLOAT_EQ(val1[2][2], val2[2][2]); \
  EXPECT_FLOAT_EQ(val1[2][3], val2[2][3]); \
  EXPECT_FLOAT_EQ(val1[3][0], val2[3][0]); \
  EXPECT_FLOAT_EQ(val1[3][1], val2[3][1]); \
  EXPECT_FLOAT_EQ(val1[3][2], val2[3][2]); \
  EXPECT_FLOAT_EQ(val1[3][3], val2[3][3])