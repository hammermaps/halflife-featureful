#include <gtest/gtest.h>
#include "parsetext.h"
#include "color_utils.h"

TEST(ParseText, Color) {
	int packedColor;
	int packedColor0x;
	int packedColorHtml;

	EXPECT_TRUE(ParseColor("255 8 32", packedColor));
	EXPECT_TRUE(ParseColor("0xFF0820", packedColor0x));
	EXPECT_TRUE(ParseColor("#FF0820", packedColorHtml));

	EXPECT_EQ(packedColor, 0xFF0820);
	EXPECT_EQ(packedColor0x, 0xFF0820);
	EXPECT_EQ(packedColorHtml, 0xFF0820);

	int temp;
	EXPECT_FALSE(ParseColor("string", temp));
	EXPECT_FALSE(ParseColor("0xstring", temp));
	EXPECT_FALSE(ParseColor("#string", temp));
}

TEST(ParseText, Boolean) {
	bool zero, falseBool, FalseBool, noBool, NoBool;
	bool one, trueBool, TrueBool, yesBool, YesBool;

	EXPECT_TRUE(ParseBoolean("0", zero));
	EXPECT_TRUE(ParseBoolean("false", falseBool));
	EXPECT_TRUE(ParseBoolean("False", FalseBool));
	EXPECT_TRUE(ParseBoolean("no", noBool));
	EXPECT_TRUE(ParseBoolean("No", NoBool));

	EXPECT_FALSE(zero);
	EXPECT_FALSE(falseBool);
	EXPECT_FALSE(FalseBool);
	EXPECT_FALSE(noBool);
	EXPECT_FALSE(NoBool);

	EXPECT_TRUE(ParseBoolean("1", one));
	EXPECT_TRUE(ParseBoolean("true", trueBool));
	EXPECT_TRUE(ParseBoolean("True", TrueBool));
	EXPECT_TRUE(ParseBoolean("yes", yesBool));
	EXPECT_TRUE(ParseBoolean("Yes", YesBool));

	EXPECT_TRUE(one);
	EXPECT_TRUE(trueBool);
	EXPECT_TRUE(TrueBool);
	EXPECT_TRUE(yesBool);
	EXPECT_TRUE(YesBool);

	bool temp;
	EXPECT_FALSE(ParseBoolean("arbitrary string", temp));
	EXPECT_FALSE(ParseBoolean("10", temp));
	EXPECT_FALSE(ParseBoolean("2", temp));
}
