// slang-fossil.h
#ifndef SLANG_FOSSIL_H
#define SLANG_FOSSIL_H

//
// This file defines a memory-mappable binary format for
// serialized object graphs. To distinguish this specific
// format from other serialization formats used in the
// Slang project, we refer to these serialized objects
// as *fossilized* objects, and to the format as the
// *fossil format*.
//

#include "../core/slang-relative-ptr.h"

namespace Slang
{
// A key part of the fossil representation is the use of *relative pointers*,
// so that a fossilized object graph can be traversed dirctly in memory
// without having to deserialize any of the intermediate objects.
//
// Fossil uses 32-bit relative pointers, to keep the format compact.

template<typename T>
using FossilizedPtr = RelativePtr32<T>;

// Various other parts of the format need to store offsets or counts,
// and for consistency we will store them with the same number of
// bits as the relative pointers already used in the format.
//
// To make it easier for us to (potentially) change the relative
// pointer size down the line, we define type aliases for the
// general-purpose integer types that will be used in fossilized data.

using FossilInt = FossilizedPtr<void>::Offset;
using FossilUInt = FossilizedPtr<void>::UOffset;

//
// The fossil format supports data that is *self-describing*.
//
// A `FossilizedValLayout` describes the in-memory layout of a fossilized
// value. Given a `FossilizedValLayout` and a pointer to the data
// for a particular value, it is possible to inspect the structure
// of the fossilized data.
//
// If all you have is a `FossilizedVal*`, then there is no way to access
// its contents without assuming it is of some particular type and casting
// it.
//
// A `FossilizedVariantObj` is a fossilized value that is self-describing;
// it stores a (relative) pointer to a layout, which can be used to inspect
// its own data/state.
//

struct FossilizedVal;
struct FossilizedValLayout;
struct FossilizedVariantObj;

/// Kinds of values that can appear in fossilized data.
enum class FossilizedValKind : FossilUInt
{
    Bool,
    Int8,
    Int16,
    Int32,
    Int64,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    Float32,
    Float64,
    String,
    Array,
    Optional,
    Dictionary,
    Tuple,
    Struct,
    Ptr,
    Variant,
};

/// Layout information about a fossilized value in memory.
///
///
/// Every `FossilizedValLayout` stores the kind of the value.
/// Based on that kind, specific additional fields may be
/// available as part of the layout.
///
struct FossilizedValLayout
{
    FossilizedValKind kind;
};

struct FossilizedPtrLikeLayout
{
    // Note: we aren't using inheritance in the definitions
    // of these types, because per the letter of the law in
    // C++, a type is only "standard layout" when there is
    // only a single type in the inheritance hierarchy that
    // has (non-static) data members.

    FossilizedValKind kind;
    FossilizedPtr<FossilizedValLayout> elementLayout;
};

struct FossilizedContainerLayout
{
    FossilizedValKind kind;
    FossilizedPtr<FossilizedValLayout> elementLayout;
    FossilUInt elementStride;
};

struct FossilizedRecordElementLayout
{
    FossilizedPtr<FossilizedValLayout> layout;
    FossilUInt offset;
};

struct FossilizedRecordLayout
{
    FossilizedValKind kind;
    FossilUInt fieldCount;

    // FossilizedRecordElementLayout elements[];

    FossilizedRecordElementLayout* getField(Index index) const;
};

/// A reference to a fossilized value in memory (of type T), and its layout.
///
template<typename T>
struct FossilizedValRef_
{
public:
    using Val = T;
    using Layout = typename T::Layout;

    /// Construct a null reference.
    ///
    FossilizedValRef_() {}

    /// Construct a reference to the given `data`, assuming it has the given `layout`.
    ///
    FossilizedValRef_(T* data, Layout* layout)
        : _data(data), _layout(layout)
    {
    }

    /// Get the kind of value being referenced.
    ///
    /// This reference must not be null.
    ///
    FossilizedValKind getKind()
    {
        SLANG_ASSERT(getLayout());
        return getLayout()->kind;
    }

    /// Get the layout of the value being referenced.
    ///
    Layout* getLayout() { return _layout; }

    /// Get a pointer to the value being referenced.
    ///
    T* getData() { return _data; }

    operator T*() const { return _data; }

    T* operator->() { return _data; }

private:
    T* _data = nullptr;
    Layout* _layout = nullptr;
};

using FossilizedValRef = FossilizedValRef_<FossilizedVal>;

/// A fossilized value in memory.
///
/// There isn't a lot that can be done with a bare pointer to
/// a `FossilizedVal`. This type is mostly declared to allow
/// us to make it explicit when a pointer points to a fossilized
/// value (even if we don't know anything about its layout).
///
struct FossilizedVal
{
public:
    using Kind = FossilizedValKind;
    using Layout = FossilizedValLayout;

    /// Determine if a value with the given `kind` should be allowed to cast to this type.
    static bool _isMatchingKind(Kind kind) { return true; }

protected:
    FossilizedVal() = default;
    FossilizedVal(FossilizedVal const&) = default;
    FossilizedVal(FossilizedVal&&) = default;
    ~FossilizedVal() = default;
};

template<typename T, FossilizedValKind kKind>
struct FossilizedSimpleVal : FossilizedVal
{
public:
    T getValue() const { return _value; }

    /// Determine if a value with the given `kind` should be allowed to cast to this type.
    static bool _isMatchingKind(Kind kind) { return kind == kKind; }

private:
    T _value;
};

using FossilizedInt8Val = FossilizedSimpleVal<int8_t, FossilizedValKind::Int8>;
using FossilizedInt16Val = FossilizedSimpleVal<int16_t, FossilizedValKind::Int16>;
using FossilizedInt32Val = FossilizedSimpleVal<int32_t, FossilizedValKind::Int32>;
using FossilizedInt64Val = FossilizedSimpleVal<int64_t, FossilizedValKind::Int64>;

using FossilizedUInt8Val = FossilizedSimpleVal<uint8_t, FossilizedValKind::UInt8>;
using FossilizedUInt16Val = FossilizedSimpleVal<uint16_t, FossilizedValKind::UInt16>;
using FossilizedUInt32Val = FossilizedSimpleVal<uint32_t, FossilizedValKind::UInt32>;
using FossilizedUInt64Val = FossilizedSimpleVal<uint64_t, FossilizedValKind::UInt64>;

using FossilizedFloat32Val = FossilizedSimpleVal<float, FossilizedValKind::Float32>;
using FossilizedFloat64Val = FossilizedSimpleVal<double, FossilizedValKind::Float64>;

struct FossilizedBoolVal : FossilizedVal
{
public:
    bool getValue() const { return _value != 0; }

    /// Determine if a value with the given `kind` should be allowed to cast to this type.
    static bool _isMatchingKind(Kind kind) { return kind == Kind::Bool; }

private:
    uint8_t _value;
};

struct FossilizedPtrVal : FossilizedVal
{
public:
    using Layout = FossilizedPtrLikeLayout;

    FossilizedVal* getTargetData() const { return _value.get(); }

    /// Determine if a value with the given `kind` should be allowed to cast to this type.
    static bool _isMatchingKind(Kind kind) { return kind == Kind::Ptr; }

private:
    FossilizedPtr<FossilizedVal> _value;
};


struct FossilizedRecordVal : FossilizedVal
{
public:
    using Layout = FossilizedRecordLayout;

    /// Determine if a value with the given `kind` should be allowed to cast to this type.
    static bool _isMatchingKind(Kind kind)
    {
        switch (kind)
        {
        default:
            return false;

        case Kind::Struct:
        case Kind::Tuple:
            return true;
        }
    }
};

//
// Some of the following subtypes of `FossilizedVal` are
// named as `Fossilized*Obj` rather than `Fossilized*Val`,
// to indicate that they will only ever be located on the
// other side of a pointer indirection.
//
// E.g., a field of a fossilized struct value should never
// have a layout claiming it to be of kind `String`; instead
// it should show as a field of kind `Ptr`, where the
// pointed-to type is `String`. The same goes for `Optional`,
// `Array`, and `Dictionary`.
//
// This distinction only matters when dealing with things like
// an *optional* string, because instead of an in-memory
// layout like `Ptr -> Optional -> Ptr -> String`, the fossilized
// data will simply store `Ptr -> Optional -> String`.
//

struct FossilizedStringObj : FossilizedVal
{
public:
    Size getSize() const;
    UnownedTerminatedStringSlice getValue() const;

    /// Determine if a value with the given `kind` should be allowed to cast to this type.
    static bool _isMatchingKind(Kind kind) { return kind == Kind::String; }

private:
    // Before the `this` address, there is a `FossilUInt`
    // with the size of the string in bytes.
    //
    // At the `this` address there is a nul-terminated
    // serquence of `getSize() + 1` bytes.
};

struct FossilizedOptionalObj : FossilizedVal
{
public:
    using Layout = FossilizedPtrLikeLayout;

    /// Determine if a value with the given `kind` should be allowed to cast to this type.
    static bool _isMatchingKind(Kind kind) { return kind == Kind::Optional; }

    FossilizedVal* getValue() { return this; }

    FossilizedVal const* getValue() const { return this; }

private:
    // An absent optional is encoded as a null pointer
    // (so `this` would be null), while a present value
    // is encoded as a pointer to that value. Thus the
    // held value is at the same address as `this`.
};

struct FossilizedContainerObj : FossilizedVal
{
public:
    using Layout = FossilizedContainerLayout;

    Count getElementCount() const;

    /// Determine if a value with the given `kind` should be allowed to cast to this type.
    static bool _isMatchingKind(Kind kind)
    {
        switch (kind)
        {
        default:
            return false;

        case Kind::Array:
        case Kind::Dictionary:
            return true;
        }
    }

private:
    // Before the `this` address, there is a `FossilUInt`
    // with the number of elements.
    //
    // At the `this` address there is a sequence of
    // `getCount()` elements. The layout of those elements
    // cannot be determined without having a `FossilizedContainerLayout`
    // for this container.
};

struct FossilizedVariantObj : FossilizedVal
{
public:
    FossilizedValLayout* getContentLayout() const;


    FossilizedVal* getContentData() { return this; }
    FossilizedVal const* getContentData() const { return this; }

    static bool _isMatchingKind(Kind kind) { return kind == Kind::Variant; }

private:
    // Before the `this` address, there is a `FossilizedPtr<FossilizedValLayout>`
    // with the layout of the content.
    //
    // The content itself starts at the `this` address, with its
    // layout determined by `getContentLayout()`.
};

/// Dynamic cast of a reference to a fossilized value.
///
template<typename T, typename U>
FossilizedValRef_<T> as(FossilizedValRef_<U> valRef)
{
    if (!valRef || !T::_isMatchingKind(valRef.getKind()))
        return FossilizedValRef_<T>();

    return FossilizedValRef_<T>(
        static_cast<T*>(valRef.getData()),
        reinterpret_cast<typename T::Layout*>(valRef.getLayout()));
}

using FossilizedInt8ValRef = FossilizedValRef_<FossilizedInt8Val>;
using FossilizedInt16ValRef = FossilizedValRef_<FossilizedInt16Val>;
using FossilizedInt32ValRef = FossilizedValRef_<FossilizedInt32Val>;
using FossilizedInt64ValRef = FossilizedValRef_<FossilizedInt64Val>;
using FossilizedUInt8ValRef = FossilizedValRef_<FossilizedUInt8Val>;
using FossilizedUInt16ValRef = FossilizedValRef_<FossilizedUInt16Val>;
using FossilizedUInt32ValRef = FossilizedValRef_<FossilizedUInt32Val>;
using FossilizedUInt64ValRef = FossilizedValRef_<FossilizedUInt64Val>;
using FossilizedFloat32ValRef = FossilizedValRef_<FossilizedFloat32Val>;
using FossilizedFloat64ValRef = FossilizedValRef_<FossilizedFloat64Val>;
using FossilizedBoolValRef = FossilizedValRef_<FossilizedBoolVal>;
using FossilizedStringObjRef = FossilizedValRef_<FossilizedStringObj>;
using FossilizedPtrValRef = FossilizedValRef_<FossilizedPtrVal>;
using FossilizedOptionalObjRef = FossilizedValRef_<FossilizedOptionalObj>;
using FossilizedContainerObjRef = FossilizedValRef_<FossilizedContainerObj>;
using FossilizedRecordValRef = FossilizedValRef_<FossilizedRecordVal>;
using FossilizedVariantObjRef = FossilizedValRef_<FossilizedVariantObj>;

FossilizedValRef getPtrTarget(FossilizedPtrValRef ptrRef);

bool hasValue(FossilizedOptionalObjRef optionalRef);
FossilizedValRef getValue(FossilizedOptionalObjRef optionalRef);

Count getElementCount(FossilizedContainerObjRef containerRef);
FossilizedValRef getElement(FossilizedContainerObjRef containerRef, Index index);

Count getFieldCount(FossilizedRecordValRef recordRef);
FossilizedValRef getField(FossilizedRecordValRef recordRef, Index index);

FossilizedValRef getVariantContent(FossilizedVariantObjRef variantRef);
FossilizedValRef getVariantContent(FossilizedVariantObj* variantPtr);


namespace Fossil
{
using RelativePtrOffset = FossilizedPtr<void>::Offset;

/// Header for a fossil-format file or blob.
///
/// A blob of fossilized data must start with a `Header`
/// that is properly formatted.
///
struct Header
{
    /// The "magic" bytes used to identify this is a fossil-format blob.
    char magic[16];

    /// The expected bytes that should appear in `magic`
    ///
    static const char kMagic[16];

    /// The total size of the fossil-format blob, including this header.
    UInt64 totalSizeIncludingHeader;

    /// Flags; reserved for future use.
    UInt32 flags;

    /// A relative pointer to the root value of the object graph.
    ///
    /// A fossil-format blob may only have one root value, and that
    /// value *must* be a variant (so that it can reference the
    /// layout information that describes itself). The *content*
    /// of the root object can be arbitrary, so applications may
    /// store multiple values using an array, struct, etc.
    ///
    FossilizedPtr<FossilizedVariantObj> rootValue;
};

/// Get the root object from a fossilized blob.
///
/// This operation performs some basic validation on the blob to
/// ensure that it doesn't seem incorrectly sized or otherwise
/// corrupted/malformed.
///
FossilizedValRef getRootValue(ISlangBlob* blob);

/// Get the root object from a fossilized blob.
///
/// This operation performs some basic validation on the blob to
/// ensure that it doesn't seem incorrectly sized or otherwise
/// corrupted/malformed.
///
FossilizedValRef getRootValue(void const* data, Size size);
} // namespace Fossil

} // namespace Slang

#endif
