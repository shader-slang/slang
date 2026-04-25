// unit-test-rtti-util.cpp
//
// Tests for `RttiUtil` — runtime-type-info-based scalar conversion,
// default detection, and array ctor/copy/dtor helpers. Distinct
// from the existing `unit-test-rtti.cpp` which only covers
// RttiInfo::append (type printing).

#include "../../source/core/slang-rtti-info.h"
#include "../../source/core/slang-rtti-util.h"
#include "../../source/core/slang-string.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

template<typename T>
const RttiInfo* rtti()
{
    return GetRttiInfo<T>::get();
}

} // namespace

SLANG_UNIT_TEST(rttiUtilSetInt)
{
    // setInt() may assert on certain unsupported type combos; cover
    // the most common int targets only.
    int32_t i32 = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(RttiUtil::setInt(42, rtti<int32_t>(), &i32)));
    SLANG_CHECK(i32 == 42);

    int64_t i64 = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(RttiUtil::setInt(0x1234567890ABCDEFLL, rtti<int64_t>(), &i64)));
    SLANG_CHECK(i64 == 0x1234567890ABCDEFLL);
}

SLANG_UNIT_TEST(rttiUtilGetInt64)
{
    int32_t i32 = 42;
    SLANG_CHECK(RttiUtil::getInt64(rtti<int32_t>(), &i32) == 42);

    uint32_t u32 = 200;
    SLANG_CHECK(RttiUtil::getInt64(rtti<uint32_t>(), &u32) == 200);

    int64_t i64 = -7;
    SLANG_CHECK(RttiUtil::getInt64(rtti<int64_t>(), &i64) == -7);
}

SLANG_UNIT_TEST(rttiUtilSetFromDouble)
{
    float f = 0.0f;
    SLANG_CHECK(SLANG_SUCCEEDED(RttiUtil::setFromDouble(3.5, rtti<float>(), &f)));
    SLANG_CHECK(f == 3.5f);

    double d = 0.0;
    SLANG_CHECK(SLANG_SUCCEEDED(RttiUtil::setFromDouble(2.71828, rtti<double>(), &d)));
    SLANG_CHECK(d == 2.71828);

    // Setting double on an int — implementation may convert or fail;
    // either way, calling shouldn't crash.
    int32_t i = 0;
    (void)RttiUtil::setFromDouble(7.5, rtti<int32_t>(), &i);
}

SLANG_UNIT_TEST(rttiUtilAsDouble)
{
    float f = 3.14f;
    SLANG_CHECK(RttiUtil::asDouble(rtti<float>(), &f) == double(3.14f));

    double d = 1.5;
    SLANG_CHECK(RttiUtil::asDouble(rtti<double>(), &d) == 1.5);

    int32_t i = 7;
    SLANG_CHECK(RttiUtil::asDouble(rtti<int32_t>(), &i) == 7.0);
}

SLANG_UNIT_TEST(rttiUtilAsBool)
{
    bool t = true;
    SLANG_CHECK(RttiUtil::asBool(rtti<bool>(), &t));
    bool f = false;
    SLANG_CHECK(!RttiUtil::asBool(rtti<bool>(), &f));

    // Non-zero int → true; zero → false.
    int32_t one = 1;
    int32_t zero = 0;
    SLANG_CHECK(RttiUtil::asBool(rtti<int32_t>(), &one));
    SLANG_CHECK(!RttiUtil::asBool(rtti<int32_t>(), &zero));
}

SLANG_UNIT_TEST(rttiUtilIsDefault)
{
    int32_t zero = 0;
    int32_t one = 1;
    SLANG_CHECK(RttiUtil::isDefault(RttiDefaultValue::Normal, rtti<int32_t>(), &zero));
    SLANG_CHECK(!RttiUtil::isDefault(RttiDefaultValue::Normal, rtti<int32_t>(), &one));

    bool falseVal = false;
    bool trueVal = true;
    SLANG_CHECK(RttiUtil::isDefault(RttiDefaultValue::Normal, rtti<bool>(), &falseVal));
    SLANG_CHECK(!RttiUtil::isDefault(RttiDefaultValue::Normal, rtti<bool>(), &trueVal));

    String emptyStr;
    String nonEmpty = "hi";
    SLANG_CHECK(RttiUtil::isDefault(RttiDefaultValue::Normal, rtti<String>(), &emptyStr));
    SLANG_CHECK(!RttiUtil::isDefault(RttiDefaultValue::Normal, rtti<String>(), &nonEmpty));
}

SLANG_UNIT_TEST(rttiUtilCanZeroInit)
{
    SLANG_CHECK(RttiUtil::canZeroInit(rtti<int32_t>()));
    SLANG_CHECK(RttiUtil::canZeroInit(rtti<float>()));
    SLANG_CHECK(RttiUtil::canZeroInit(rtti<bool>()));
    SLANG_CHECK(RttiUtil::canZeroInit(rtti<double>()));
}

SLANG_UNIT_TEST(rttiUtilCanMemCpy)
{
    SLANG_CHECK(RttiUtil::canMemCpy(rtti<int32_t>()));
    SLANG_CHECK(RttiUtil::canMemCpy(rtti<float>()));
    SLANG_CHECK(RttiUtil::canMemCpy(rtti<bool>()));

    // String can't be memcpy'd (it owns a heap buffer).
    SLANG_CHECK(!RttiUtil::canMemCpy(rtti<String>()));
}

SLANG_UNIT_TEST(rttiUtilHasDtor)
{
    // POD types have no dtor.
    SLANG_CHECK(!RttiUtil::hasDtor(rtti<int32_t>()));
    SLANG_CHECK(!RttiUtil::hasDtor(rtti<float>()));
    SLANG_CHECK(!RttiUtil::hasDtor(rtti<bool>()));

    // String + List have dtors (own heap data).
    SLANG_CHECK(RttiUtil::hasDtor(rtti<String>()));
    SLANG_CHECK(RttiUtil::hasDtor(rtti<List<int32_t>>()));
}

SLANG_UNIT_TEST(rttiUtilGetDefaultTypeFuncs)
{
    auto funcs = RttiUtil::getDefaultTypeFuncs(rtti<int32_t>());
    SLANG_CHECK(funcs.isValid());
    SLANG_CHECK(funcs.ctorArray != nullptr);
    SLANG_CHECK(funcs.dtorArray != nullptr);
    SLANG_CHECK(funcs.copyArray != nullptr);
}

SLANG_UNIT_TEST(rttiUtilCopyArray)
{
    // Use the high-level RttiUtil::copyArray for plain-old-data.
    int32_t src[3] = {1, 2, 3};
    int32_t dst[3] = {0, 0, 0};
    RttiTypeFuncsMap typeMap;
    RttiUtil::copyArray(&typeMap, rtti<int32_t>(), dst, src, sizeof(int32_t), 3);
    SLANG_CHECK(dst[0] == 1 && dst[1] == 2 && dst[2] == 3);
}

// Note: tests for ctorArray / dtorArray / setListCount on heap-
// owning types require a populated RttiTypeFuncsMap (the default
// map is empty for non-POD types). Adding those tests requires
// understanding the type-funcs registration machinery, which is
// itself not currently unit-tested. Skipping for now; tracked as
// a follow-up alongside the IR-fixture work.
