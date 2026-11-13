// Inline records can end with a byte while containing fields with larger alignment.
// This test compares dynamic array layout, typed fossilized access, and ordinary
// deserialization so all three must agree on the padding between adjacent records.

#include "core/slang-blob-builder.h"
#include "slang-com-ptr.h"
#include "slang/slang-serialize-fossil.h"
#include "unit-test/slang-unit-test.h"

namespace Slang
{

/// An integer and a marker whose serialized fields require tail padding in an array.
struct MarkedValue
{
    uint32_t value = 0;
    bool isMarked = false;
};

template<>
struct FossilizedTypeTraits<MarkedValue>
{
    /// The typed fossil representation of an integer followed by its marker byte.
    struct FossilizedType
    {
        Fossilized<uint32_t> value;
        Fossilized<bool> isMarked;
    };
};

template<typename S>
void serialize(S const& serializer, MarkedValue& value)
{
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    serialize(serializer, value.value);
    serialize(serializer, value.isMarked);
}

} // namespace Slang

using namespace Slang;

SLANG_UNIT_TEST(fossilRecordArrayPreservesTailPadding)
{
    // The serialized fields occupy five bytes, but the integer requires four-byte
    // alignment. The second record must begin eight bytes after the first.
    MarkedValue first;
    first.value = 0x12345678;
    first.isMarked = true;
    MarkedValue second;
    second.value = 0x87654321;
    List<MarkedValue> original;
    original.add(first);
    original.add(second);

    BlobBuilder builder;
    {
        Fossil::SerialWriter writer(builder);
        Serializer<Fossil::SerialWriter, void> serializer(&writer);
        serialize(serializer, original);
    }
    ComPtr<ISlangBlob> blob;
    builder.writeToBlob(blob.writeRef());
    auto root = Fossil::getRootValue(blob);

    // The dynamic layout must describe the same stride as the typed C++ fossil
    // representation. Checking the second value also detects incorrect actual padding.
    auto pointer = Fossil::as<FossilizedPtr<void>>(root);
    SLANG_CHECK_ABORT(pointer);
    auto array = Fossil::as<FossilizedArrayObjBase>(pointer->getTargetValPtr());
    SLANG_CHECK_ABORT(array);
    SLANG_CHECK(array->getLayout()->elementStride == sizeof(Fossilized<MarkedValue>));
    auto typed = static_cast<const Fossilized<List<MarkedValue>>*>(root.getDataPtr());
    SLANG_CHECK_ABORT(typed->getElementCount() == 2);
    SLANG_CHECK((*typed)[0].value == first.value);
    SLANG_CHECK((*typed)[0].isMarked == first.isMarked);
    SLANG_CHECK((*typed)[1].value == second.value);
    SLANG_CHECK((*typed)[1].isMarked == second.isMarked);

    // The ordinary reader follows the dynamic stride. It must agree with typed
    // access rather than reading the second record from inside the first record.
    // An array root is indirect, so the reader's initial object is its pointer target.
    List<MarkedValue> loaded;
    Fossil::SerialReader::ReadContext context;
    Fossil::SerialReader reader(context, pointer->getTargetValPtr());
    Serializer<Fossil::SerialReader, void> serializer(&reader);
    serialize(serializer, loaded);
    SLANG_CHECK_ABORT(loaded.getCount() == 2);
    SLANG_CHECK(loaded[0].value == first.value);
    SLANG_CHECK(loaded[0].isMarked == first.isMarked);
    SLANG_CHECK(loaded[1].value == second.value);
    SLANG_CHECK(loaded[1].isMarked == second.isMarked);
}
