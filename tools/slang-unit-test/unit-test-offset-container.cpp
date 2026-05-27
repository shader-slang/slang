// unit-test-offset-container.cpp

#include "core/slang-exception.h"
#include "core/slang-offset-container.h"
#include "unit-test/slang-unit-test.h"

#include <stdlib.h>

using namespace Slang;

static void _checkEncodeDecode(uint32_t size)
{
    uint8_t encode[OffsetString::kMaxSizeEncodeSize];

    size_t encodeSize = OffsetString::calcEncodedSize(size, encode);

    size_t decodedSize;
    const char* chars = OffsetString::decodeSize((const char*)encode, decodedSize);

    SLANG_CHECK(decodedSize == size);
    SLANG_CHECK(chars - (const char*)encode == encodeSize);
}

static void _checkAllocateOverflowDoesNotWrap(size_t dataSize, size_t size, size_t alignment)
{
    OffsetContainer container;
    container.m_dataSize = dataSize;

    // Snapshot capacity and data pointer so we can verify the rejection path leaves the
    // backing buffer untouched. This guards against future regressions where a new guard
    // is added after the realloc/capacity-bump block instead of before it.
    const size_t capBefore = container.getCapacity();
    const uint8_t* dataBefore = container.getData();

    void* data = container.allocate(size, alignment);

    SLANG_CHECK(data == nullptr);
    SLANG_CHECK(container.getDataCount() == dataSize);
    SLANG_CHECK(container.getCapacity() == capBefore);
    SLANG_CHECK(container.getData() == dataBefore);
}

namespace
{ // anonymous

struct Root
{
    Offset32Array<Offset32Ptr<OffsetString>> dirs;
    Offset32Ptr<OffsetString> name;
    float value;
};

static int _writeEnvironmentVariable(const char* key, const char* val)
{
#ifdef _WIN32
    String var = String(key) + "=" + val;
    return _putenv(var.getBuffer());
#else
    return setenv(key, val, 1);
#endif
}

static int _unsetEnvironmentVariable(const char* key)
{
#ifdef _WIN32
    String var = String(key) + "=";
    return _putenv(var.getBuffer());
#else
    return unsetenv(key);
#endif
}

struct ScopedEnvVar
{
    const char* key;
    bool hadOldValue = false;
    String oldValue;

    ScopedEnvVar(const char* inKey, const char* inVal)
        : key(inKey)
    {
#ifdef _WIN32
        char* value = nullptr;
        size_t valueLength = 0;
        if (_dupenv_s(&value, &valueLength, key) == 0 && value)
        {
            hadOldValue = true;
            oldValue = value;
            free(value);
        }
#else
        if (const char* value = getenv(key))
        {
            hadOldValue = true;
            oldValue = value;
        }
#endif
        _writeEnvironmentVariable(key, inVal);
    }

    ScopedEnvVar(const ScopedEnvVar&) = delete;
    ScopedEnvVar& operator=(const ScopedEnvVar&) = delete;

    ~ScopedEnvVar()
    {
        if (hadOldValue)
        {
            _writeEnvironmentVariable(key, oldValue.getBuffer());
        }
        else
        {
            _unsetEnvironmentVariable(key);
        }
    }
};

} // namespace

SLANG_UNIT_TEST(offsetContainer)
{
    _checkEncodeDecode(253);

    for (int64_t i = 0; i < 0x100000000; i += (i / 2) + 1)
    {
        _checkEncodeDecode(uint32_t(i));
    }

    _checkAllocateOverflowDoesNotWrap(SIZE_MAX - 7, 16, 1);
    _checkAllocateOverflowDoesNotWrap(SIZE_MAX - 3, 1, 8);

    // Exceed the 32-bit offset limit (the container addresses memory via Offset32Ptr, so
    // allocations beyond the 4GB boundary must be rejected even when size_t arithmetic does
    // not overflow on a 64-bit host).
    _checkAllocateOverflowDoesNotWrap(size_t(kMax32Offset) - 7, 16, 1);
    _checkAllocateOverflowDoesNotWrap(size_t(kMax32Offset) - 3, 1, 8);
    _checkAllocateOverflowDoesNotWrap(size_t(0x80000000u), size_t(0x80000000u), 1);

    // Zero and non-power-of-two alignments must be rejected (the bitwise alignment math
    // relies on a non-zero power of two).
    _checkAllocateOverflowDoesNotWrap(0, 1, 0);
    _checkAllocateOverflowDoesNotWrap(0, 1, 3);
    _checkAllocateOverflowDoesNotWrap(0, 1, 5);
    _checkAllocateOverflowDoesNotWrap(0, 1, 6);

#if SIZE_MAX > 0xFFFFFFFFu
    // On 64-bit hosts a power-of-two alignment larger than the 32-bit offset domain must be
    // rejected before alignmentMask = alignment - 1 underflows kMaxDataSize - alignmentMask.
    _checkAllocateOverflowDoesNotWrap(0, 1, size_t(1) << 33);
    _checkAllocateOverflowDoesNotWrap(8, 1, size_t(1) << 40);
#endif

    // Exhaust the 32-bit offset domain so allocate() returns nullptr, then verify that
    // newObject / newArray / newString propagate the failure as their null sentinel.
    {
        OffsetContainer container;
        container.m_dataSize = size_t(kMax32Offset) - 3;

        auto obj = container.newObject<uint64_t>();
        SLANG_CHECK(obj.isNull());

        auto arr = container.newArray<uint64_t>(2);
        SLANG_CHECK(arr.getCount() == 0);

        auto str = container.newString(UnownedStringSlice("xyz"));
        SLANG_CHECK(str.isNull());
    }

    // newArray must reject a count whose sizeof(T) * count would exceed the 32-bit offset
    // domain, independent of the current m_dataSize.
    {
        OffsetContainer container;
        auto arr = container.newArray<uint64_t>(size_t(kMax32Offset) / sizeof(uint64_t) + 1);
        SLANG_CHECK(arr.getCount() == 0);
    }

#if SIZE_MAX > 0xFFFFFFFFu
    // On 64-bit hosts, exercise newArray's count cap (size > 0xFFFFFFFFu). Note that
    // allocate()'s 32-bit byte-cap also rejects this input (0x100000000 bytes > 4 GiB),
    // so the test covers both guards together rather than isolating just the count-cap
    // branch. Choosing an element type that would isolate the count-cap branch isn't
    // feasible on a 64-bit host: with sizeof(T) > 0, size > 0xFFFFFFFFu always implies
    // sizeof(T) * size > 0xFFFFFFFFu.
    {
        OffsetContainer container;
        auto arr = container.newArray<uint8_t>(size_t(0x100000000ull));
        SLANG_CHECK(arr.getCount() == 0);
    }

    // newString must reject a slice whose length exceeds the 32-bit offset domain after
    // accounting for the encoded header and trailing null. The size-cap branch fires
    // before any read of the slice contents, so we can use a non-deref'd placeholder
    // pointer with a fabricated length. Gated to 64-bit because b + len overflows
    // size_t on 32-bit hosts.
    {
        const char* fake = reinterpret_cast<const char*>(uintptr_t(1));
        UnownedStringSlice slice(fake, size_t(0xFFFFFFFEu));
        OffsetContainer container;
        SLANG_CHECK(container.newString(slice).isNull());
    }
#endif

    // allocateAndZero must propagate allocate()'s nullptr instead of memset'ing through it.
    {
        OffsetContainer container;
        container.m_dataSize = size_t(kMax32Offset) - 3;
        void* zeroed = container.allocateAndZero(16, 1);
        SLANG_CHECK(zeroed == nullptr);
    }

    {
        OffsetContainer container;

        const char* strings[] = {
            "Hello",
            "World",
            nullptr,
        };

        {
            auto& base = container.asBase();

            Offset32Ptr<Root> root = container.newObject<Root>();

            auto array = container.newArray<Offset32Ptr<OffsetString>>(SLANG_COUNT_OF(strings));
            for (Int i = 0; i < SLANG_COUNT_OF(strings); ++i)
            {
                base[array[i]] = container.newString(strings[i]);
            }
            base[root]->dirs = array;
        }

        {
            List<uint8_t> copy;
            copy.addRange(container.getData(), container.getDataCount());

            MemoryOffsetBase base;
            base.set(copy.getBuffer(), copy.getCount());

            Root* root = (Root*)(copy.getBuffer() + kStartOffset);

            SLANG_CHECK(root->dirs.getCount() == SLANG_COUNT_OF(strings));

            Int count = root->dirs.getCount();
            for (Int i = 0; i < count; ++i)
            {
                OffsetString* str = base.asRaw(base.asRaw(root->dirs[i]));

                const char* check = strings[i];

                if (check)
                {
                    SLANG_CHECK(str != nullptr);
                    const char* strCstr = str->getCstr();
                    SLANG_CHECK(strcmp(strCstr, check) == 0);
                }
                else
                {
                    SLANG_CHECK(str == nullptr);
                }
            }

            {
                Index index = 0;
                for (const auto v : root->dirs)
                {
                    OffsetString* str = base.asRaw(base.asRaw(v));
                    const char* check = strings[index];
                    if (check)
                    {
                        SLANG_CHECK(str != nullptr);
                        const char* strCstr = str->getCstr();
                        SLANG_CHECK(strcmp(strCstr, check) == 0);
                    }
                    else
                    {
                        SLANG_CHECK(str == nullptr);
                    }

                    index++;
                }
            }
        }
    }

    {
        uint8_t data[16] = {};

        MemoryOffsetBase base;
        base.set(data, sizeof(data));

        Offset32Ptr<uint32_t> nullPtr;
        SLANG_CHECK(base.asRaw(nullPtr) == nullptr);

        Offset32Ptr<uint32_t> validPtr(kStartOffset);
        SLANG_CHECK(base.asRaw(validPtr) == (uint32_t*)(data + kStartOffset));

        Offset32Ptr<uint8_t> lastBytePtr(uint32_t(sizeof(data) - 1));
        SLANG_CHECK(base.asRaw(lastBytePtr) == data + sizeof(data) - 1);

        Offset32Ptr<uint32_t> partialPtr(uint32_t(sizeof(data) - sizeof(uint32_t) + 1));
        SLANG_CHECK(base.asRaw(partialPtr) == nullptr);

        Offset32Ptr<uint8_t> pastEndPtr(uint32_t(sizeof(data)));
        SLANG_CHECK(base.asRaw(pastEndPtr) == nullptr);

        // offset > m_dataSize: exercises the first disjunct of the bounds
        // check in _getRaw, which guards `m_dataSize - offset` from
        // underflow. `pastEndPtr` above only hits the second disjunct.
        Offset32Ptr<uint8_t> wayPastEndPtr(uint32_t(sizeof(data) + 1));
        SLANG_CHECK(base.asRaw(wayPastEndPtr) == nullptr);
    }

    // _getRaw early-return on null m_data: a default-constructed MemoryOffsetBase
    // has m_data == nullptr; asRaw on any non-null offset must return nullptr
    // without dereferencing.
    {
        MemoryOffsetBase emptyBase;
        Offset32Ptr<uint32_t> ptr(kStartOffset);
        SLANG_CHECK(emptyBase.asRaw(ptr) == nullptr);
    }

    // _getRaw overflow safety: with m_dataSize close to UINT32_MAX and an
    // offset near UINT32_MAX, the disjunctive check
    // `offset > m_dataSize || size > m_dataSize - offset`
    // must reject. A future "simplification" to `offset + size > m_dataSize`
    // computed in uint32_t would wrap silently and yield a false negative.
    //
    // We never dereference the spoofed range — _getRaw returns nullptr before
    // touching m_data — so a 1-byte stack buffer is safe to spoof.
    {
        uint8_t dummy = 0;
        MemoryOffsetBase base;
        // Spoof a very large dataSize without actually allocating it.
        base.set(&dummy, size_t(0xFFFFFFFDu));

        // offset (0xFFFFFFFCu) > m_dataSize? false.
        // size (4) > m_dataSize - offset (1)? true → reject.
        Offset32Ptr<uint32_t> hugeOffsetPtr(0xFFFFFFFCu);
        SLANG_CHECK(base.asRaw(hugeOffsetPtr) == nullptr);
    }

    // Offset32Array::operator[] positive coverage at 0 and count-1.
    // The bounds check is a SLANG_RELEASE_ASSERT; this test would trip the
    // assert if the check were ever inverted (e.g. `<=` instead of `<`).
    {
        OffsetContainer container;
        auto& base = container.asBase();
        auto arr = container.newArray<uint32_t>(3);

        base[arr[0]] = 100u;
        base[arr[1]] = 200u;
        base[arr[2]] = 300u;

        SLANG_CHECK(base[arr[0]] == 100u);
        SLANG_CHECK(base[arr[2]] == 300u);
    }

#if SLANG_HAS_EXCEPTIONS
    // Offset32Array::operator[] must remain a release assert. With
    // release-assert-only enabled, a debug-only SLANG_ASSERT would be ignored
    // in debug builds, but SLANG_RELEASE_ASSERT still raises InternalError.
    {
        ScopedEnvVar assertMode("SLANG_ASSERT", "release-assert-only");

        OffsetContainer container;
        auto arr = container.newArray<uint32_t>(1);

        bool nonConstCaught = false;
        try
        {
            (void)arr[Index(5)];
        }
        catch (const InternalError&)
        {
            nonConstCaught = true;
        }
        SLANG_CHECK(nonConstCaught);

        const Offset32Array<uint32_t>& constArr = arr;
        bool constCaught = false;
        try
        {
            (void)constArr[Index(5)];
        }
        catch (const InternalError&)
        {
            constCaught = true;
        }
        SLANG_CHECK(constCaught);
    }

    // Offset32Ref access must also remain a release assert when the serialized
    // offset is outside the backing buffer.
    {
        ScopedEnvVar assertMode("SLANG_ASSERT", "release-assert-only");

        uint8_t data[16] = {};
        MemoryOffsetBase base;
        base.set(data, sizeof(data));

        Offset32Ref<uint32_t> badRef(uint32_t(sizeof(data)));

        bool asRawCaught = false;
        try
        {
            (void)base.asRaw(badRef);
        }
        catch (const InternalError&)
        {
            asRawCaught = true;
        }
        SLANG_CHECK(asRawCaught);

        bool operatorCaught = false;
        try
        {
            (void)base[badRef];
        }
        catch (const InternalError&)
        {
            operatorCaught = true;
        }
        SLANG_CHECK(operatorCaught);
    }
#endif
}
