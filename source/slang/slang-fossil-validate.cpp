// slang-fossil-validate.cpp
#include "slang-fossil.h"

#if SLANG_ENABLE_VALIDATION_FOSSIL

#include "core/slang-dictionary.h"
#include "core/slang-list.h"

//
// This file implements a one-time validating walk over a fossil-format blob,
// performed before any consumer navigates it.
//
// The fossil format is designed to be traversed in place, following 32-bit
// relative pointers without deserializing anything. That makes every pointer,
// count, and size in the blob an input that a malformed or hostile file gets to
// choose. Rather than bounds-check each of the many places that dereference such
// a value, this walk visits the whole graph once and proves that every location a
// consumer can reach lies inside the blob, so that consumers can stay simple.
//

namespace Slang
{
namespace Fossil
{

namespace
{

/// Byte offset from the start of a blob, or `kNullOffset` for a null pointer.
using BlobOffset = Int64;

static const BlobOffset kNullOffset = -1;

/// How a value is represented at the location the walk arrived at.
///
/// The distinction matters because strings, containers, and variants have two
/// representations for one layout kind. Stored in place -- as a struct field or
/// an array element -- such a value is a relative pointer to its object. But
/// `Fossilized<String*>` is `FossilizedPtr<FossilizedStringObj>`, so arriving
/// through a pointer lands on the object itself; the format deliberately avoids
/// the redundant pointer-to-pointer.
///
enum class ValForm
{
    /// The location holds the value itself (for object kinds, a pointer to it).
    InPlace,

    /// The location is the target of a pointer (for object kinds, the object).
    PointerTarget,
};

/// Return the number of bytes a value of the given `kind` occupies in place.
///
/// The object-like kinds store only a relative pointer in place, so they are the
/// same size as a pointer. Records are excluded, because they have no single
/// in-place size: their extent is described field-by-field by their layout.
///
static Int64 getInPlaceSizeOfVal(FossilizedValKind kind)
{
    switch (kind)
    {
    case FossilizedValKind::Bool:
    case FossilizedValKind::Int8:
    case FossilizedValKind::UInt8:
        return 1;

    case FossilizedValKind::Int16:
    case FossilizedValKind::UInt16:
        return 2;

    case FossilizedValKind::Int32:
    case FossilizedValKind::UInt32:
    case FossilizedValKind::Float32:
        return 4;

    case FossilizedValKind::Int64:
    case FossilizedValKind::UInt64:
    case FossilizedValKind::Float64:
        return 8;

    case FossilizedValKind::StringObj:
    case FossilizedValKind::ArrayObj:
    case FossilizedValKind::OptionalObj:
    case FossilizedValKind::DictionaryObj:
    case FossilizedValKind::Ptr:
    case FossilizedValKind::VariantObj:
        return sizeof(FossilizedPtr<void>);

    default:
        SLANG_UNEXPECTED("unhandled fossilized value kind");
        UNREACHABLE_RETURN(0);
    }
}

/// A location the walk has reached, identified by where its data and layout live
/// and how the data is represented there.
///
/// The walk memoizes these, so a shared or cyclic graph is visited once per
/// distinct location rather than endlessly.
///
struct ValKey
{
    BlobOffset dataOffset;
    BlobOffset layoutOffset;
    ValForm form;

    bool operator==(ValKey const& that) const
    {
        return dataOffset == that.dataOffset && layoutOffset == that.layoutOffset &&
               form == that.form;
    }
    bool operator!=(ValKey const& that) const { return !(*this == that); }

    HashCode64 getHashCode() const
    {
        return combineHash(
            ::Slang::getHashCode(dataOffset),
            ::Slang::getHashCode(layoutOffset),
            ::Slang::getHashCode(Int32(form)));
    }
};

/// Walks a fossil blob and proves that everything reachable from its root value
/// lies within the blob.
///
/// Every read of blob memory goes through `_requireRange()` first, so the walk
/// never dereferences a location it has not already proven to be in bounds.
///
struct Validator
{
public:
    Validator(void const* data, Size size)
        : _base((Byte const*)data), _size(Int64(size))
    {
    }

    /// Validate the root variant object at `rootValueOffset`, and everything
    /// reachable from it.
    ///
    void validateFrom(BlobOffset rootValueOffset)
    {
        _visitVariantObj(rootValueOffset);

        // The graph is walked with an explicit work list rather than by recursing,
        // so that a deeply nested blob cannot overflow the stack.
        //
        while (_workList.getCount())
        {
            auto item = _workList.getLast();
            _workList.removeLast();
            _visitVal(item);
        }
    }

private:
    using WorkItem = ValKey;

    Byte const* _base = nullptr;
    Int64 _size = 0;
    HashSet<ValKey> _visited;
    List<WorkItem> _workList;

    /// Require that `[offset, offset + rangeSize)` lies within the blob.
    ///
    void _requireRange(BlobOffset offset, Int64 rangeSize)
    {
        SLANG_ASSERT(rangeSize >= 0);

        // Subtract from the bound rather than adding to `offset`, so that a large
        // `offset` cannot overflow.
        //
        SLANG_SERIALIZE_FOSSIL_VALIDATE(offset >= 0 && offset <= _size - rangeSize);
    }

    /// Read the value of type `T` stored at `offset`.
    ///
    template<typename T>
    T _read(BlobOffset offset)
    {
        _requireRange(offset, sizeof(T));

        // Copied out rather than read in place, because nothing guarantees that a
        // hostile blob put this value at an alignment suitable for `T`.
        //
        T value;
        memcpy(&value, _base + offset, sizeof(value));
        return value;
    }

    /// Read the relative pointer stored at `offset`, returning the offset of what
    /// it points at, or `kNullOffset` if it is null.
    ///
    BlobOffset _readRelativePtr(BlobOffset offset)
    {
        auto relativeOffset = _read<FossilInt>(offset);
        if (relativeOffset == 0)
            return kNullOffset;

        // A relative pointer is measured from its own address. The result is
        // computed in the offset domain, so no pointer outside the blob is ever
        // formed, let alone dereferenced.
        //
        return offset + Int64(relativeOffset);
    }

    /// Validate that the layout object at `layoutOffset` is itself readable, and
    /// return the kind it declares.
    ///
    /// Only this one layout object is checked. Layouts it refers to are validated
    /// if and when the walk reaches a value described by them, so a layout that
    /// nothing reads is never trusted.
    ///
    FossilizedValKind _validateLayoutHeader(BlobOffset layoutOffset)
    {
        auto rawKind = _read<FossilUInt>(layoutOffset);
        SLANG_SERIALIZE_FOSSIL_VALIDATE(rawKind <= FossilUInt(FossilizedValKind::VariantObj));
        auto kind = FossilizedValKind(rawKind);

        switch (kind)
        {
        case FossilizedValKind::Ptr:
        case FossilizedValKind::OptionalObj:
            _requireRange(layoutOffset, sizeof(FossilizedPtrLikeLayout));
            break;

        case FossilizedValKind::ArrayObj:
        case FossilizedValKind::DictionaryObj:
            _requireRange(layoutOffset, sizeof(FossilizedContainerLayout));
            break;

        case FossilizedValKind::Struct:
        case FossilizedValKind::Tuple:
            {
                _requireRange(layoutOffset, sizeof(FossilizedRecordLayout));

                // The field array trails the layout, so all of it has to be
                // readable before any field can be consulted. The count is a
                // 32-bit value scaled by a small constant, so the product cannot
                // overflow `Int64`.
                //
                auto fieldCount = Int64(_read<FossilUInt>(layoutOffset + kRecordFieldCountOffset));
                _requireRange(
                    layoutOffset + kRecordFieldsOffset,
                    fieldCount * Int64(sizeof(FossilizedRecordElementLayout)));
            }
            break;

        default:
            break;
        }

        return kind;
    }

    /// Queue a location, unless it has already been walked.
    ///
    void _queue(BlobOffset dataOffset, BlobOffset layoutOffset, ValForm form)
    {
        SLANG_SERIALIZE_FOSSIL_VALIDATE(layoutOffset != kNullOffset);

        ValKey key = {dataOffset, layoutOffset, form};
        if (!_visited.add(key))
            return;

        _workList.add(key);
    }

    /// Validate one location, queueing whatever it refers to.
    ///
    void _visitVal(WorkItem const& item)
    {
        auto layoutOffset = item.layoutOffset;
        auto dataOffset = item.dataOffset;
        auto kind = _validateLayoutHeader(layoutOffset);

        if (item.form == ValForm::PointerTarget &&
            _visitPointerTarget(dataOffset, layoutOffset, kind))
            return;

        switch (kind)
        {
        case FossilizedValKind::Struct:
        case FossilizedValKind::Tuple:
            _visitRecord(dataOffset, layoutOffset);
            break;

        case FossilizedValKind::Ptr:
        case FossilizedValKind::OptionalObj:
            {
                // Both kinds store a relative pointer in place, and in both cases
                // what it points at is described by the layout's element layout.
                // An absent optional is encoded as null.
                //
                auto targetOffset = _readRelativePtr(dataOffset);
                if (targetOffset != kNullOffset)
                {
                    _queue(
                        targetOffset,
                        _getElementLayoutOffset(layoutOffset),
                        ValForm::PointerTarget);
                }
            }
            break;

        case FossilizedValKind::StringObj:
        case FossilizedValKind::ArrayObj:
        case FossilizedValKind::DictionaryObj:
        case FossilizedValKind::VariantObj:
            {
                auto objOffset = _readRelativePtr(dataOffset);
                if (objOffset != kNullOffset)
                    _queue(objOffset, layoutOffset, ValForm::PointerTarget);
            }
            break;

        default:
            // What remains are the scalar kinds, which are fully described by the
            // bytes they occupy.
            //
            _requireRange(dataOffset, getInPlaceSizeOfVal(kind));
            break;
        }
    }

    /// Validate the target of a pointer, for the kinds whose representation as a
    /// pointer target differs from their representation in place. Returns true if
    /// `kind` is such a kind and has been handled.
    ///
    /// The object kinds are stored out of line, preceded by a word giving their
    /// size, element count, or content layout, and a pointer to one lands on that
    /// object rather than on a pointer to it. An optional is subtler: reached
    /// through a pointer it holds no storage of its own, and its value simply
    /// lives at the same address.
    ///
    bool _visitPointerTarget(BlobOffset dataOffset, BlobOffset layoutOffset, FossilizedValKind kind)
    {
        switch (kind)
        {
        case FossilizedValKind::StringObj:
            _visitStringObj(dataOffset);
            return true;

        case FossilizedValKind::ArrayObj:
        case FossilizedValKind::DictionaryObj:
            _visitContainerObj(dataOffset, layoutOffset);
            return true;

        case FossilizedValKind::VariantObj:
            _visitVariantObj(dataOffset);
            return true;

        case FossilizedValKind::OptionalObj:
            _queue(dataOffset, _getElementLayoutOffset(layoutOffset), ValForm::PointerTarget);
            return true;

        default:
            return false;
        }
    }

    /// Return the offset of the element layout referenced by the pointer-like or
    /// container layout at `layoutOffset`, requiring it to be present.
    ///
    BlobOffset _getElementLayoutOffset(BlobOffset layoutOffset)
    {
        auto elementLayoutOffset = _readRelativePtr(layoutOffset + kElementLayoutOffset);

        // A null element layout is only legal when nothing is described by it,
        // which the callers establish before getting here.
        //
        SLANG_SERIALIZE_FOSSIL_VALIDATE(elementLayoutOffset != kNullOffset);
        return elementLayoutOffset;
    }

    /// Validate the fields of the record at `dataOffset`.
    ///
    void _visitRecord(BlobOffset dataOffset, BlobOffset layoutOffset)
    {
        auto fieldCount = Int64(_read<FossilUInt>(layoutOffset + kRecordFieldCountOffset));
        for (Int64 i = 0; i < fieldCount; ++i)
        {
            auto fieldOffset = layoutOffset + kRecordFieldsOffset +
                               i * Int64(sizeof(FossilizedRecordElementLayout));

            auto fieldLayoutOffset = _readRelativePtr(fieldOffset);
            SLANG_SERIALIZE_FOSSIL_VALIDATE(fieldLayoutOffset != kNullOffset);

            auto fieldDataOffset =
                Int64(_read<FossilUInt>(fieldOffset + Int64(sizeof(FossilizedPtr<void>))));
            _queue(dataOffset + fieldDataOffset, fieldLayoutOffset, ValForm::InPlace);
        }
    }

    /// Validate the string object at `objOffset`, whose length is stored in the
    /// word before it and whose bytes follow it.
    ///
    void _visitStringObj(BlobOffset objOffset)
    {
        auto stringSize = Int64(_read<FossilUInt>(objOffset - Int64(sizeof(FossilUInt))));

        // The stored size excludes the terminator, but the bytes are handed out as
        // a terminated slice, so the terminator has to be inside the blob too --
        // and has to actually be there, or a consumer reads past the string.
        //
        _requireRange(objOffset, stringSize + 1);
        SLANG_SERIALIZE_FOSSIL_VALIDATE(_base[objOffset + stringSize] == 0);
    }

    /// Validate the array or dictionary object at `objOffset`, whose element count
    /// is stored in the word before it and whose elements follow it.
    ///
    void _visitContainerObj(BlobOffset objOffset, BlobOffset layoutOffset)
    {
        auto elementCount = Int64(_read<FossilUInt>(objOffset - Int64(sizeof(FossilUInt))));
        if (elementCount == 0)
            return;

        auto elementStride = Int64(_read<FossilUInt>(layoutOffset + kContainerStrideOffset));
        SLANG_SERIALIZE_FOSSIL_VALIDATE(elementStride > 0);

        // Both operands are 32-bit values widened to `Int64`, so the extent of the
        // element array is computed without overflow before being checked.
        //
        _requireRange(objOffset, elementCount * elementStride);

        auto elementLayoutOffset = _getElementLayoutOffset(layoutOffset);
        for (Int64 i = 0; i < elementCount; ++i)
        {
            _queue(objOffset + i * elementStride, elementLayoutOffset, ValForm::InPlace);
        }
    }

    /// Validate the variant object at `objOffset`, which stores the layout of its
    /// content in the word before it, and the content itself at its own address.
    ///
    void _visitVariantObj(BlobOffset objOffset)
    {
        auto contentLayoutOffset =
            _readRelativePtr(objOffset - Int64(sizeof(FossilizedPtr<FossilizedValLayout>)));
        if (contentLayoutOffset != kNullOffset)
            _queue(objOffset, contentLayoutOffset, ValForm::InPlace);
    }

    //
    // Offsets of fields within the layout structures. The walk reads these out of
    // the blob by hand, rather than through the `Fossilized*Layout` types, so that
    // no pointer into unvalidated memory is formed.
    //

    static const Int64 kElementLayoutOffset = sizeof(FossilizedValKind);
    static const Int64 kRecordFieldCountOffset = sizeof(FossilizedValKind);
    static const Int64 kRecordFieldsOffset = sizeof(FossilizedRecordLayout);
    static const Int64 kContainerStrideOffset =
        sizeof(FossilizedValKind) + sizeof(FossilizedPtr<FossilizedValLayout>);
};

// The walk decodes layout fields by offset, so it has to agree with the
// structures it is decoding.
//
static_assert(sizeof(FossilizedValLayout) == 4);
static_assert(sizeof(FossilizedPtrLikeLayout) == 8);
static_assert(sizeof(FossilizedContainerLayout) == 12);
static_assert(sizeof(FossilizedRecordLayout) == 8);
static_assert(sizeof(FossilizedRecordElementLayout) == 8);

} // namespace

void validateRootValue(void const* data, Size size, FossilizedVariantObj* rootValueVariant)
{
    Validator validator(data, size);
    validator.validateFrom(Int64(intptr_t(rootValueVariant)) - Int64(intptr_t(data)));
}

} // namespace Fossil
} // namespace Slang

#endif
