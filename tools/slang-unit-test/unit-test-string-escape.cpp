// unit-test-string-escape.cpp

#include "../../source/core/slang-string-escape-util.h"

#include "tools/unit-test/slang-unit-test.h"

using namespace Slang;

static bool _checkConversion(StringEscapeHandler* handler, const UnownedStringSlice& check)
{
	StringBuilder buf;
	handler->appendEscaped(check, buf);

	StringBuilder decode;
	handler->appendUnescaped(buf.getUnownedSlice(), decode);

	return decode == check;
}

SLANG_UNIT_TEST(StringEscape)
{
	// Check greedy hex digits
	{
		// \x can have any number of hex digits
		const char text[] = "\x000001";
		SLANG_ASSERT(SLANG_COUNT_OF(text) == 2 && text[0] == 1);
	}

	// Check octal greedy
	{
		//\ + up to 3 octal digits
		const char text[] = "\0011";
		SLANG_ASSERT(SLANG_COUNT_OF(text) == 3 && text[0] == 1 && text[1] == '1');

		const char text2[] = "\78";
		SLANG_ASSERT(SLANG_COUNT_OF(text2) == 3 && text2[0] == 7 && text2[1] == '8');
	}

	{
		auto handler = StringEscapeUtil::getHandler(StringEscapeUtil::Style::Cpp);

		SLANG_CHECK(_checkConversion(handler, toSlice("\0\1\2""2")));
	}
}

