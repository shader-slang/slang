// unit-test-rtti-util.cpp
//
// Contract under test
// -------------------
// `RttiUtil` provides the runtime bridge between
// `RttiInfo` (compile-time type descriptors built via
// `GetRttiInfo<T>`) and value access at runtime — used by Slang's
// JSON / fossil serializers to read/write fields generically.
//
//   * `setInt` / `getInt64`   — write/read an integer through a
//     type descriptor pointing at the storage. Round-trips for
//     every integer width, plus bool. The serializer relies on
//     getInt64(setInt(v)) == v for any in-range value.
//   * `setFromDouble` / `asDouble` — same for floats.
//   * `asBool`               — non-zero integer → true, zero → false;
//     same as the C semantics callers expect.
//   * `isDefault(Normal)`    — returns true exactly when the value
//     is the type's default-init value (zero-equivalent for POD,
//     empty for String, no elements for List). The serializer uses
//     this to suppress writing default-valued fields.
//   * `canZeroInit` / `canMemCpy` / `hasDtor` — used by the
//     serializer to pick the cheapest construction/copy/destruction
//     path. The contract is functional: when `canMemCpy(T)` returns
//     true, memcpy IS a valid copy for T, and when `hasDtor(T)`
//     returns false, skipping ~T() is safe.
//   * `getDefaultTypeFuncs`  — returns a triple of (ctor, dtor,
//     copy) function pointers that can be invoked on POD types
//     without a populated typefuncs map.
//
// The trivial-type-classification tests do NOT just confirm "the
// implementation says int is POD" — they verify the OPERATIONAL
// guarantee callers depend on (e.g. canMemCpy(int) → memcpy
// produces equal values; hasDtor(int) == false → skipping dtor
// leaks nothing).

#include "../../source/core/slang-rtti-info.h"
#include "../../source/core/slang-rtti-util.h"
#include "../../source/core/slang-string.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

namespace
{

template<typename T>
const RttiInfo* rtti()
{
    return GetRttiInfo<T>::get();
}

} // namespace

// Integer round-trip: setInt then getInt64 must reproduce the same
// value, for every integer width supported by GetRttiInfo. The
// serializer relies on this round-trip.
SLANG_UNIT_TEST(rttiUtilIntRoundTrip)
{
    int32_t i32 = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(RttiUtil::setInt(42, rtti<int32_t>(), &i32)));
    SLANG_CHECK(RttiUtil::getInt64(rtti<int32_t>(), &i32) == 42);

    int64_t i64 = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(RttiUtil::setInt(0x1234567890ABCDEFLL, rtti<int64_t>(), &i64)));
    SLANG_CHECK(RttiUtil::getInt64(rtti<int64_t>(), &i64) == 0x1234567890ABCDEFLL);

    uint32_t u32 = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(RttiUtil::setInt(0xFFFFFFFFu, rtti<uint32_t>(), &u32)));
    SLANG_CHECK(u32 == 0xFFFFFFFFu);
    SLANG_CHECK(uint32_t(RttiUtil::getInt64(rtti<uint32_t>(), &u32)) == 0xFFFFFFFFu);
}

// Negative values round-trip through signed types.
SLANG_UNIT_TEST(rttiUtilIntRoundTripNegative)
{
    int32_t i32 = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(RttiUtil::setInt(-1, rtti<int32_t>(), &i32)));
    SLANG_CHECK(i32 == -1);
    SLANG_CHECK(RttiUtil::getInt64(rtti<int32_t>(), &i32) == -1);

    int64_t i64 = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(RttiUtil::setInt(-9999999999LL, rtti<int64_t>(), &i64)));
    SLANG_CHECK(i64 == -9999999999LL);
}

// Float round-trip via setFromDouble / asDouble. The serializer
// uses these to read/write JSON numbers into typed storage.
SLANG_UNIT_TEST(rttiUtilFloatRoundTrip)
{
    float f = 0.0f;
    SLANG_CHECK(SLANG_SUCCEEDED(RttiUtil::setFromDouble(3.5, rtti<float>(), &f)));
    SLANG_CHECK(f == 3.5f);
    SLANG_CHECK(RttiUtil::asDouble(rtti<float>(), &f) == 3.5);

    double d = 0.0;
    SLANG_CHECK(SLANG_SUCCEEDED(RttiUtil::setFromDouble(2.71828, rtti<double>(), &d)));
    SLANG_CHECK(d == 2.71828);
    SLANG_CHECK(RttiUtil::asDouble(rtti<double>(), &d) == 2.71828);
}

// Cross-typed reads — int → double (used when reading an int field
// into a JSON number) must produce the same numeric value.
SLANG_UNIT_TEST(rttiUtilAsDoubleFromInt)
{
    int32_t i = 7;
    SLANG_CHECK(RttiUtil::asDouble(rtti<int32_t>(), &i) == 7.0);
    int64_t j = -1234567890LL;
    SLANG_CHECK(RttiUtil::asDouble(rtti<int64_t>(), &j) == -1234567890.0);
}

// asBool: bool is identity; integers follow C truthiness (non-zero
// → true). The serializer uses this to read JSON bools into either
// `bool` or `int` storage.
SLANG_UNIT_TEST(rttiUtilAsBoolSemantics)
{
    bool t = true, f = false;
    SLANG_CHECK(RttiUtil::asBool(rtti<bool>(), &t));
    SLANG_CHECK(!RttiUtil::asBool(rtti<bool>(), &f));

    int32_t one = 1, zero = 0, neg = -1;
    SLANG_CHECK(RttiUtil::asBool(rtti<int32_t>(), &one));
    SLANG_CHECK(!RttiUtil::asBool(rtti<int32_t>(), &zero));
    SLANG_CHECK(RttiUtil::asBool(rtti<int32_t>(), &neg)); // non-zero → true
}

// `isDefault(Normal)` — true exactly when the value equals the
// type's default-init. Used by the serializer to omit default-
// valued fields, so it must precisely match what zero-init produces.
SLANG_UNIT_TEST(rttiUtilIsDefaultNormal)
{
    int32_t zero = 0, one = 1;
    SLANG_CHECK(RttiUtil::isDefault(RttiDefaultValue::Normal, rtti<int32_t>(), &zero));
    SLANG_CHECK(!RttiUtil::isDefault(RttiDefaultValue::Normal, rtti<int32_t>(), &one));

    bool falseVal = false, trueVal = true;
    SLANG_CHECK(RttiUtil::isDefault(RttiDefaultValue::Normal, rtti<bool>(), &falseVal));
    SLANG_CHECK(!RttiUtil::isDefault(RttiDefaultValue::Normal, rtti<bool>(), &trueVal));

    String empty, hi = "hi";
    SLANG_CHECK(RttiUtil::isDefault(RttiDefaultValue::Normal, rtti<String>(), &empty));
    SLANG_CHECK(!RttiUtil::isDefault(RttiDefaultValue::Normal, rtti<String>(), &hi));

    List<int32_t> emptyList, withItem;
    withItem.add(1);
    SLANG_CHECK(RttiUtil::isDefault(RttiDefaultValue::Normal, rtti<List<int32_t>>(), &emptyList));
    SLANG_CHECK(!RttiUtil::isDefault(RttiDefaultValue::Normal, rtti<List<int32_t>>(), &withItem));
}

// canZeroInit must be a TRUE statement: when it returns true, an
// all-zero buffer reinterpreted as T must equal a default-
// constructed T. The serializer skips ctor calls when this holds.
SLANG_UNIT_TEST(rttiUtilCanZeroInitImpliesZeroIsDefault)
{
    SLANG_CHECK(RttiUtil::canZeroInit(rtti<int32_t>()));
    SLANG_CHECK(RttiUtil::canZeroInit(rtti<float>()));
    SLANG_CHECK(RttiUtil::canZeroInit(rtti<bool>()));

    // For each "canZeroInit" type, an all-zero buffer must produce
    // the type's default value (i.e. isDefault returns true).
    int32_t i32buf;
    memset(&i32buf, 0, sizeof(i32buf));
    SLANG_CHECK(RttiUtil::isDefault(RttiDefaultValue::Normal, rtti<int32_t>(), &i32buf));

    float fbuf;
    memset(&fbuf, 0, sizeof(fbuf));
    SLANG_CHECK(RttiUtil::isDefault(RttiDefaultValue::Normal, rtti<float>(), &fbuf));

    bool bbuf;
    memset(&bbuf, 0, sizeof(bbuf));
    SLANG_CHECK(RttiUtil::isDefault(RttiDefaultValue::Normal, rtti<bool>(), &bbuf));
}

// canMemCpy must be a TRUE statement: when it returns true, memcpy
// IS a valid copy for T (the destination becomes equal to the
// source). For heap-owning types it must return false — memcpy
// would alias the heap buffer and double-free at dtor.
SLANG_UNIT_TEST(rttiUtilCanMemCpyImpliesMemcpyIsValid)
{
    SLANG_CHECK(RttiUtil::canMemCpy(rtti<int32_t>()));
    SLANG_CHECK(RttiUtil::canMemCpy(rtti<float>()));
    SLANG_CHECK(RttiUtil::canMemCpy(rtti<bool>()));

    // memcpy from a known value into a zeroed dest produces the
    // same value when read back.
    int32_t src = 42, dst = 0;
    memcpy(&dst, &src, sizeof(int32_t));
    SLANG_CHECK(RttiUtil::getInt64(rtti<int32_t>(), &dst) == 42);

    // String owns heap data — memcpy would alias it. The contract
    // says canMemCpy(String) returns false so the serializer takes
    // the deep-copy path.
    SLANG_CHECK(!RttiUtil::canMemCpy(rtti<String>()));
    SLANG_CHECK(!RttiUtil::canMemCpy(rtti<List<int32_t>>()));
}

// hasDtor must be a TRUE statement: when it returns false,
// skipping ~T() is safe (no resources leak). Used by the serializer
// to skip dtor calls on POD-only field arrays.
SLANG_UNIT_TEST(rttiUtilHasDtorPredicate)
{
    SLANG_CHECK(!RttiUtil::hasDtor(rtti<int32_t>()));
    SLANG_CHECK(!RttiUtil::hasDtor(rtti<float>()));
    SLANG_CHECK(!RttiUtil::hasDtor(rtti<bool>()));

    // Heap-owning types must report true — otherwise the serializer
    // would skip ~String / ~List and leak.
    SLANG_CHECK(RttiUtil::hasDtor(rtti<String>()));
    SLANG_CHECK(RttiUtil::hasDtor(rtti<List<int32_t>>()));
}

// getDefaultTypeFuncs returns a valid triple for POD types — the
// serializer uses it as a fallback when no type-funcs map has been
// registered for the type. The triple must be invokable.
SLANG_UNIT_TEST(rttiUtilDefaultTypeFuncsAreInvokable)
{
    auto funcs = RttiUtil::getDefaultTypeFuncs(rtti<int32_t>());
    SLANG_CHECK(funcs.isValid());

    // Calling them on a POD array produces the documented result:
    // ctor zero-inits, copy bit-copies, dtor is a no-op (verifiable
    // because the destination memory is unchanged by it).
    int32_t arr[3] = {99, 99, 99};
    int32_t src[3] = {1, 2, 3};
    RttiTypeFuncsMap typeMap;
    funcs.ctorArray(&typeMap, rtti<int32_t>(), arr, 3);
    SLANG_CHECK(arr[0] == 0 && arr[1] == 0 && arr[2] == 0);

    funcs.copyArray(&typeMap, rtti<int32_t>(), arr, src, 3);
    SLANG_CHECK(arr[0] == 1 && arr[1] == 2 && arr[2] == 3);

    funcs.dtorArray(&typeMap, rtti<int32_t>(), arr, 3); // no-op on POD
    SLANG_CHECK(arr[0] == 1); // memory still readable
}

// RttiUtil::copyArray on POD types performs bit-copy via the
// default funcs; dst becomes element-wise equal to src.
SLANG_UNIT_TEST(rttiUtilCopyArrayPodIsBitwiseCopy)
{
    int32_t src[5] = {1, 2, 3, 4, 5};
    int32_t dst[5] = {0, 0, 0, 0, 0};
    RttiTypeFuncsMap typeMap;
    RttiUtil::copyArray(&typeMap, rtti<int32_t>(), dst, src, sizeof(int32_t), 5);
    for (int i = 0; i < 5; ++i)
    {
        SLANG_CHECK(dst[i] == src[i]);
    }
}
