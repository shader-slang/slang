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
// The term "fossil" is being used here to refer to formerly
// "live" objects that have been converted into an alternative
// form that can no longer perform their original functions,
// but that can still be inspected and dug through.
//

#include "../core/slang-relative-ptr.h"

#include <optional>
#include <type_traits>

namespace Slang
{

struct FossilizedPtrLikeLayout;
struct FossilizedRecordLayout;
struct FossilizedValLayout;

using FossilInt = int32_t;
using FossilUInt = uint32_t;

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
    StringObj,
    ArrayObj,
    OptionalObj,
    DictionaryObj,
    Tuple,
    Struct,
    Ptr,
    VariantObj,
};

// A key part of the fossil representation is the use of *relative pointers*,
// so that a fossilized object graph can be traversed dirctly in memory
// without having to deserialize any of the intermediate objects.
//
// Fossil uses 32-bit relative pointers, to keep the format compact.

template<typename T>
struct FossilizedPtr : RelativePtr32<T>
{
public:
    using RelativePtr32<T>::RelativePtr32;

    using Layout = FossilizedPtrLikeLayout;

    SLANG_FORCE_INLINE static bool isMatchingKind(FossilizedValKind kind)
    {
        return kind == FossilizedValKind::Ptr;
    }
};

static_assert(sizeof(FossilizedPtr<void>) == sizeof(uint32_t));

// Various other parts of the format need to store offsets or counts,
// and for consistency we will store them with the same number of
// bits as the relative pointers already used in the format.
//
// To make it easier for us to (potentially) change the relative
// pointer size down the line, we define type aliases for the
// general-purpose integer types that will be used in fossilized data.


static_assert(sizeof(FossilInt) == sizeof(FossilizedPtr<void>));
static_assert(sizeof(FossilUInt) == sizeof(FossilizedPtr<void>));

//
// A "live" type can declare what its fossilized representation
// is by specializing the `FossilizedTypeTraits` template.
//
// By default, a type is fossilized as an opaque `FossilizedOpaqueVal`
// if no user-defined specialization is provided.
//

template<typename T>
struct Fossilized_;

template<typename T>
struct FossilizedTypeTraits
{
    using FossilizedType = Fossilized_<T>;
};

template<typename T>
using Fossilized = FossilizedTypeTraits<T>::FossilizedType;

//
// In many cases, a new C++ type can be fossilized using
// the same representation as some existing type, so we
// allow them to conveniently declare that fact with
// a macro.
//

#define SLANG_DECLARE_FOSSILIZED_AS(TYPE, FOSSILIZED_AS)  \
    template<>                                            \
    struct FossilizedTypeTraits<TYPE>                     \
    {                                                     \
        using FossilizedType = Fossilized<FOSSILIZED_AS>; \
    }

//
// Another common pattern is when some aggregate type
// can simply be fossilized as one of its members.
//

#define SLANG_DECLARE_FOSSILIZED_AS_MEMBER(TYPE, MEMBER)           \
    template<>                                                     \
    struct FossilizedTypeTraits<TYPE>                              \
    {                                                              \
        using FossilizedType = Fossilized<decltype(TYPE::MEMBER)>; \
    }

//
// Simple scalar values are fossilized into a wrapper
// `struct` that contains the underlying value.
//
// The reason to impose the wrapper `struct` is that
// it allows us to control the alignment of the type
// in case it turns out that different targets/compilers
// don't apply the same layout to all of the underlying
// scalar types.
//

template<typename T, FossilizedValKind Kind>
struct FossilizedSimpleVal
{
public:
    using Layout = FossilizedValLayout;
    static const FossilizedValKind kKind = Kind;

    SLANG_FORCE_INLINE T const& get() const { return _value; }

    SLANG_FORCE_INLINE operator T const&() const { return _value; }

    SLANG_FORCE_INLINE static bool isMatchingKind(FossilizedValKind kind) { return kind == kKind; }

private:
    T _value;
};

#define SLANG_DECLARE_FOSSILIZED_SIMPLE_TYPE(TYPE, TAG)                           \
    template<>                                                                    \
    struct FossilizedTypeTraits<TYPE>                                             \
    {                                                                             \
        using FossilizedType = FossilizedSimpleVal<TYPE, FossilizedValKind::TAG>; \
    };

SLANG_DECLARE_FOSSILIZED_SIMPLE_TYPE(int8_t, Int8)
SLANG_DECLARE_FOSSILIZED_SIMPLE_TYPE(int16_t, Int16)
SLANG_DECLARE_FOSSILIZED_SIMPLE_TYPE(int32_t, Int32)
SLANG_DECLARE_FOSSILIZED_SIMPLE_TYPE(int64_t, Int64)

SLANG_DECLARE_FOSSILIZED_SIMPLE_TYPE(uint8_t, UInt8)
SLANG_DECLARE_FOSSILIZED_SIMPLE_TYPE(uint16_t, UInt16)
SLANG_DECLARE_FOSSILIZED_SIMPLE_TYPE(uint32_t, UInt32)
SLANG_DECLARE_FOSSILIZED_SIMPLE_TYPE(uint64_t, UInt64)

SLANG_DECLARE_FOSSILIZED_SIMPLE_TYPE(float, Float32)
SLANG_DECLARE_FOSSILIZED_SIMPLE_TYPE(double, Float64)

static_assert(sizeof(Fossilized<int8_t>) == 1);
static_assert(sizeof(Fossilized<int16_t>) == 2);
static_assert(sizeof(Fossilized<int32_t>) == 4);
static_assert(sizeof(Fossilized<int64_t>) == 8);

static_assert(sizeof(Fossilized<uint8_t>) == 1);
static_assert(sizeof(Fossilized<uint16_t>) == 2);
static_assert(sizeof(Fossilized<uint32_t>) == 4);
static_assert(sizeof(Fossilized<uint64_t>) == 8);

static_assert(sizeof(Fossilized<float>) == 4);
static_assert(sizeof(Fossilized<double>) == 8);

//
// The `bool` type shouldn't be fossilized as itself, because
// its layout is not guaranteed to be consistent across targets.
// We instead fossilize it as an underlying `uint8_t`, and convert
// on reads.
//

template<>
struct Fossilized_<bool>
{
public:
    using Layout = FossilizedValLayout;
    static const FossilizedValKind kKind = FossilizedValKind::Bool;

    SLANG_FORCE_INLINE bool get() const { return _value != 0; }

    SLANG_FORCE_INLINE operator bool() const { return get(); }

    SLANG_FORCE_INLINE static bool isMatchingKind(FossilizedValKind kind) { return kind == kKind; }

private:
    uint8_t _value;
};

static_assert(sizeof(Fossilized<bool>) == 1);

//
// Some simple types can be fossilized as one of the
// scalar types above, with explicit casts between
// the "live" type and the "fossilized" type.
//
// A common example of this is `enum` types.
//

template<typename LiveType, typename FossilizedAsType>
struct FossilizedViaCastVal
{
public:
    SLANG_FORCE_INLINE LiveType get() const { return LiveType(_value.get()); }

    SLANG_FORCE_INLINE operator LiveType() const { return get(); }


private:
    Fossilized<FossilizedAsType> _value;
};

//
// By default we assume that an `enum` type should be fossilized
// as a signed 32-bit integer.
//
#define SLANG_DECLARE_FOSSILIZED_ENUM(TYPE)                        \
    template<>                                                     \
    struct Fossilized_<TYPE> : FossilizedViaCastVal<TYPE, int32_t> \
    {                                                              \
    };

//
// For many of the other kinds of types that get fossilized,
// the in-memory encoding will typically be as a (relative)
// pointer to the actual data.
//
// Here we distinguish between the *value* type (e.g.,
// `FossilizedString`) that typically gets stored as the
// field of a record/tuple/whatever, and the *object* type
// that the value type is a (relative) pointer to (e.g.,
// `FossilizedStringObj`).
//

struct FossilizedStringObj
{
public:
    Size getSize() const;
    UnownedTerminatedStringSlice get() const;

    SLANG_FORCE_INLINE operator UnownedTerminatedStringSlice() const { return get(); }

    using Layout = FossilizedValLayout;

    SLANG_FORCE_INLINE static bool isMatchingKind(FossilizedValKind kind)
    {
        return kind == FossilizedValKind::StringObj;
    }

private:
    // Before the `this` address, there is a `FossilUInt`
    // with the size of the string in bytes.
    //
    // At the `this` address there is a nul-terminated
    // sequence of `getSize() + 1` bytes.
};

//
// The array and dictionary types are handled largely
// the same as strings, with the added detail that the
// object type is split into a base type without the
// template parameters, and a subtype that has those
// parameters. The base type enables navigating of
// these containers dynamically, based on layout.
//

struct FossilizedContainerLayout
{
    FossilizedValKind kind;
    FossilizedPtr<FossilizedValLayout> elementLayout;
    FossilUInt elementStride;
};

struct FossilizedContainerObjBase
{
public:
    using Layout = FossilizedContainerLayout;

    Count getElementCount() const;

    SLANG_FORCE_INLINE void const* getBuffer() const { return this; }

    SLANG_FORCE_INLINE static bool isMatchingKind(FossilizedValKind kind)
    {
        switch (kind)
        {
        default:
            return false;

        case FossilizedValKind::ArrayObj:
        case FossilizedValKind::DictionaryObj:
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

template<typename T>
struct FossilizedContainerObj : FossilizedContainerObjBase
{
public:
};

struct FossilizedArrayObjBase : FossilizedContainerObjBase
{
public:
    SLANG_FORCE_INLINE static bool isMatchingKind(FossilizedValKind kind)
    {
        return kind == FossilizedValKind::ArrayObj;
    }
};

template<typename T>
struct FossilizedArrayObj : FossilizedArrayObjBase
{
};

//
// While we defined the core `FossilizedPtr` type above, there is
// some subtlety involved in defining the way that a C++ pointer
// type like `T*` maps to its fossilized representation via
// `Fossilized<T*>`. The reason for this is that the binary layout
// of fossilized data avoids storing redundant pointers-to-pointers,
// so because a `Dictionary<int, float>` would already be stored
// via an indirection in the binary layout, a pointer type
// `Dictionary<int, float> *` would be stored with the exact same
// binary layout.
//
// When computing what `Fossilized<T*>` is, the result will be
// `FossilizedPtr< FossilizedPtrTarget<T> >`. The `FossilizedPtrTarget<T>`
// template uses a set of helpers defined in a `details` namespace
// to compute the correct target type.
//

namespace details
{
//
// By default, a `Fossilized<T*>` will just be a `FossilizedPtr<Fossilized<T>>`.
//
template<typename T>
T fossilizedPtrTargetType(T*, void*);
} // namespace details

template<typename T>
using FossilizedPtrTarget = decltype(details::fossilizedPtrTargetType(
    std::declval<Fossilized<T>*>(),
    std::declval<Fossilized<T>*>()));


template<typename T>
struct FossilizedTypeTraits<T*>
{
    using FossilizedType = FossilizedPtr<FossilizedPtrTarget<T>>;
};

template<typename T>
struct FossilizedTypeTraits<RefPtr<T>>
{
    using FossilizedType = FossilizedPtr<FossilizedPtrTarget<T>>;
};


//
// An optional value is effectively just a pointer, with
// the null case being used to represent the absence of
// a value.
//

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

struct FossilizedOptionalObjBase
{
public:
    SLANG_FORCE_INLINE void* getValue() { return this; }

    SLANG_FORCE_INLINE void const* getValue() const { return this; }

    using Layout = FossilizedPtrLikeLayout;

    SLANG_FORCE_INLINE static bool isMatchingKind(FossilizedValKind kind)
    {
        return kind == FossilizedValKind::OptionalObj;
    }

private:
    // An absent optional is encoded as a null pointer
    // (so `this` would be null), while a present value
    // is encoded as a pointer to that value. Thus the
    // held value is at the same address as `this`.
};

template<typename T>
struct FossilizedOptionalObj : FossilizedOptionalObjBase
{
    SLANG_FORCE_INLINE T* getValue() { return this; }

    SLANG_FORCE_INLINE T const* getValue() const { return this; }
};

template<typename T>
struct FossilizedOptional
{
public:
    SLANG_FORCE_INLINE explicit operator bool() const { return _value.get() != nullptr; }
    SLANG_FORCE_INLINE T const& operator*() const { return *_value.get(); }

private:
    FossilizedPtr<T> _value;
};

template<typename T>
struct FossilizedTypeTraits<std::optional<T>>
{
    using FossilizedType = FossilizedOptional<FossilizedPtrTarget<T>>;
};

static_assert(sizeof(Fossilized<std::optional<double>>) == sizeof(FossilUInt));

//
// With all of the various `Fossilized*Obj` cases defined above,
// we can now define the more direct versions of things that
// apply in the common case. For example, `Fossilized<String>`
// simply maps to the `FossilizedString` type, and the parallels
// are similar for arrays and dictionaries.
//

struct FossilizedString
{
public:
    SLANG_FORCE_INLINE Size getSize() const { return _obj ? _obj->getSize() : 0; }

    SLANG_FORCE_INLINE UnownedTerminatedStringSlice get() const
    {
        return _obj ? _obj->get() : UnownedTerminatedStringSlice();
    }

    SLANG_FORCE_INLINE operator UnownedTerminatedStringSlice() const { return get(); }

private:
    FossilizedPtr<FossilizedStringObj> _obj;
};

SLANG_FORCE_INLINE int compare(FossilizedString const& lhs, UnownedStringSlice const& rhs)
{
    return compare(lhs.get(), rhs);
}

SLANG_FORCE_INLINE bool operator==(FossilizedString const& left, UnownedStringSlice const& right)
{
    return left.get() == right;
}

SLANG_FORCE_INLINE bool operator!=(FossilizedString const& left, UnownedStringSlice const& right)
{
    return left.get() != right;
}

SLANG_FORCE_INLINE bool operator==(FossilizedStringObj const& left, UnownedStringSlice const& right)
{
    return left.get() == right;
}

SLANG_FORCE_INLINE bool operator!=(FossilizedStringObj const& left, UnownedStringSlice const& right)
{
    return left.get() != right;
}

#define SLANG_DECLARE_FOSSILIZED_TYPE(LIVE, FOSSILIZED) \
    template<>                                          \
    struct FossilizedTypeTraits<LIVE>                   \
    {                                                   \
        using FossilizedType = FOSSILIZED;              \
    }

SLANG_DECLARE_FOSSILIZED_TYPE(String, FossilizedString);
SLANG_DECLARE_FOSSILIZED_TYPE(UnownedStringSlice, FossilizedString);
SLANG_DECLARE_FOSSILIZED_TYPE(UnownedTerminatedStringSlice, FossilizedString);

static_assert(std::is_same_v<Fossilized<String>, FossilizedString>);
static_assert(sizeof(Fossilized<String>) == sizeof(FossilUInt));

template<typename T>
struct FossilizedContainer
{
public:
    SLANG_FORCE_INLINE Count getElementCount() const
    {
        if (!_obj)
            return 0;
        return _obj->getElementCount();
    }
    SLANG_FORCE_INLINE T const* getBuffer() const
    {
        if (!_obj)
            return nullptr;
        return (T const*)_obj.get()->getBuffer();
    }

    SLANG_FORCE_INLINE T const* begin() const { return getBuffer(); }
    SLANG_FORCE_INLINE T const* end() const { return getBuffer() + getElementCount(); }

private:
    FossilizedPtr<FossilizedContainerObj<T>> _obj;
};

template<typename T>
struct FossilizedArray : FossilizedContainer<T>
{
public:
    SLANG_FORCE_INLINE T const& operator[](Index index) const
    {
        SLANG_ASSERT(index >= 0 && index < this->getElementCount());
        return this->getBuffer()[index];
    }
};

template<typename T>
struct FossilizedTypeTraits<List<T>>
{
    using FossilizedType = FossilizedArray<Fossilized<T>>;
};

template<typename T, int N>
struct FossilizedTypeTraits<ShortList<T, N>>
{
    using FossilizedType = FossilizedArray<Fossilized<T>>;
};

template<typename T, size_t N>
struct FossilizedTypeTraits<T[N]>
{
    using FossilizedType = FossilizedArray<Fossilized<T>>;
};

static_assert(sizeof(Fossilized<List<int32_t>>) == sizeof(FossilUInt));

template<typename K, typename V>
struct FossilizedKeyValuePair
{
    using Layout = FossilizedRecordLayout;
    K key;
    V value;
};

template<typename K, typename V>
struct FossilizedTypeTraits<KeyValuePair<K, V>>
{
    using FossilizedType = FossilizedKeyValuePair<Fossilized<K>, Fossilized<V>>;
};

template<typename K, typename V>
struct FossilizedTypeTraits<std::pair<K, V>>
{
    using FossilizedType = FossilizedKeyValuePair<Fossilized<K>, Fossilized<V>>;
};

//
// In terms of the encoding, a fossilized dictionary
// is really just an array of key-value pairs, but
// we keep the types distinct to help with clarity.
//

struct FossilizedDictionaryObjBase : FossilizedContainerObjBase
{
public:
    SLANG_FORCE_INLINE static bool isMatchingKind(FossilizedValKind kind)
    {
        return kind == FossilizedValKind::DictionaryObj;
    }
};

template<typename K, typename V>
struct FossilizedDictionaryObj : FossilizedDictionaryObjBase
{
};

template<typename K, typename V>
struct FossilizedDictionary : FossilizedContainer<FossilizedKeyValuePair<K, V>>
{
public:
    using Entry = FossilizedKeyValuePair<K, V>;
};

template<typename K, typename V>
struct FossilizedTypeTraits<Dictionary<K, V>>
{
    using FossilizedType = FossilizedDictionary<Fossilized<K>, Fossilized<V>>;
};

template<typename K, typename V>
struct FossilizedTypeTraits<OrderedDictionary<K, V>>
{
    using FossilizedType = FossilizedDictionary<Fossilized<K>, Fossilized<V>>;
};

static_assert(sizeof(Fossilized<Dictionary<String, String>>) == sizeof(FossilUInt));

//
// A record (struct or tuple) is stored simply as a sequence of field
// values, and its layout gives the total number of fields as well as
// the offset and layout of each.
//

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

/// Stand-in for a fossilized record of unknown type.
///
/// Note that user-defined fossilized types should *not* try
/// to inherit from `FossilizedRecordVal`, as doing so can
/// end up breaking the correlation between the binary layout
/// of fossilized data and the matching C++ declarations.
///
struct FossilizedRecordVal
{
public:
    using Layout = FossilizedRecordLayout;

    SLANG_FORCE_INLINE static bool isMatchingKind(FossilizedValKind kind)
    {
        switch (kind)
        {
        default:
            return false;

        case FossilizedValKind::Struct:
        case FossilizedValKind::Tuple:
            return true;
        }
    }
};

//
// A *variant* is a value that can conceptually hold data of any type/layout,
// and stores a pointer to layout information so that the data it holds
// can be navigated dynamically.
//

struct FossilizedVariantObj
{
public:
    using Layout = FossilizedValLayout;
    static const FossilizedValKind kKind = FossilizedValKind::VariantObj;

    FossilizedValLayout* getContentLayout() const;

    SLANG_FORCE_INLINE void* getContentDataPtr() { return this; }
    SLANG_FORCE_INLINE void const* getContentDataPtr() const { return this; }

    SLANG_FORCE_INLINE static bool isMatchingKind(FossilizedValKind kind)
    {
        return kind == FossilizedValKind::VariantObj;
    }

private:
    // Before the `this` address, there is a `FossilizedPtr<FossilizedValLayout>`
    // with the layout of the content.
    //
    // The content itself starts at the `this` address, with its
    // layout determined by `getContentLayout()`.
};

struct FossilizedVariant
{
public:
private:
    FossilizedPtr<FossilizedVariantObj> _obj;
};

static_assert(sizeof(FossilizedVariant) == sizeof(FossilUInt));

//
// Now that all of the relevant types for fossilized data have been defined,
// we can circle back to define the specializations of `FossilizedPtrTargetType`
// for the types that need it.
//

namespace details
{
template<typename X>
FossilizedStringObj fossilizedPtrTargetType(X*, FossilizedString*);

template<typename X>
FossilizedVariantObj fossilizedPtrTargetType(X*, FossilizedVariant*);

template<typename X, typename T>
FossilizedArrayObj<T> fossilizedPtrTargetType(X*, FossilizedArray<T>*);

template<typename X, typename K, typename V>
FossilizedDictionaryObj<K, V> fossilizedPtrTargetType(X*, FossilizedDictionary<K, V>*);
} // namespace details

//
// In addition to being able to expose a statically-known
// layout through `Fossilized<T>`, the fossil format also
// allows data to be *self-describing*, by carying its layout
// with it.
//
// A `FossilizedValLayout` describes the in-memory layout of a fossilized
// value. Given a `FossilizedValLayout` and a pointer to the data
// for a particular value, it is possible to inspect the structure
// of the fossilized data.
//
// If all you have is a `void*` to a fossilzied value, then there is no way
// to access its contents without assuming it is of some particular type and
// casting it.
//
// A `FossilizedVariantObj` is a fossilized value that is self-describing;
// it stores a (relative) pointer to a layout, which can be used to inspect
// its own data/state.
//

struct FossilizedValLayout;
struct FossilizedPtrLikeLayout;
struct FossilizedContainerLayout;
struct FossilizedRecordLayout;
struct FossilizedVariantObj;

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

namespace Fossil
{
/// A reference to a fossilized value in memory, along with layout information.
///
template<typename T, typename L = typename T::Layout>
struct ValRefBase
{
public:
    using Val = T;
    using Layout = L;

    /// Construct a null reference.
    ///
    SLANG_FORCE_INLINE ValRefBase() {}

    /// Construct a reference to the given `data`, assuming it has the given `layout`.
    ///
    SLANG_FORCE_INLINE ValRefBase(T* data, Layout const* layout)
        : _data(data), _layout(layout)
    {
    }

    /// Construct a copy of `ref`.
    ///
    /// Only enabled if `U*` is convertible to `T*`.
    ///
    template<typename U>
    SLANG_FORCE_INLINE ValRefBase(
        ValRefBase<U> ref,
        std::enable_if_t<std::is_convertible_v<U*, T*>, void>* = nullptr)
        : _data(ref.getDataPtr()), _layout((Layout const*)ref.getLayout())
    {
    }

    /// Get a pointer to the value being referenced.
    ///
    SLANG_FORCE_INLINE T* getDataPtr() const { return _data; }

    /// Get a reference to the value being referenced.
    ///
    /// This accessor is disabled in the case where `T` is `void`.
    ///
    template<typename U = T>
    SLANG_FORCE_INLINE std::enable_if_t<!std::is_same_v<U, void>, T>& getDataRef() const
    {
        return *_data;
    }

    /// Get the layout of the value being referenced.
    ///
    SLANG_FORCE_INLINE Layout const* getLayout() const { return _layout; }

    /// Get the kind of value being referenced.
    ///
    /// This reference must not be null.
    ///
    SLANG_FORCE_INLINE FossilizedValKind getKind() const
    {
        SLANG_ASSERT(getLayout());
        return getLayout()->kind;
    }

protected:
    T* _data = nullptr;
    Layout const* _layout = nullptr;
};

/// A reference to a fossilized value in memory, along with layout information.
///
template<typename T>
struct ValRef : ValRefBase<T>
{
    using ValRefBase<T>::ValRefBase;
};

/// Specialization of `ValRef<T>` for the case where `T` is `void`.
///
template<>
struct ValRef<void> : ValRefBase<void, FossilizedValLayout>
{
    using ValRefBase<void, FossilizedValLayout>::ValRefBase;
};

/// A pointer to a fossilized value in memory, along with layout information.
///
template<typename T>
struct ValPtr
{
public:
    using TargetVal = T;
    using TargetLayout = typename ValRef<T>::Layout;

    /// Construct a null pointer.
    ///
    SLANG_FORCE_INLINE ValPtr() {}
    SLANG_FORCE_INLINE ValPtr(std::nullptr_t) {}

    /// Construct a pointer to the given `data`, assuming it has the given `layout`.
    ///
    SLANG_FORCE_INLINE ValPtr(T* data, TargetLayout const* layout)
        : _ref(data, layout)
    {
    }

    /// Construct a pointer to the value referenced by `ref`.
    ///
    /// This constructor is basically equivalent to the address-of operator `&`.
    /// We define it as a constructor as a slightly more preferable alternative
    /// to overloading prefix `operator&` (which is almost always a Bad Idea)
    ///
    SLANG_FORCE_INLINE explicit ValPtr(ValRef<T> ref)
        : _ref(ref)
    {
    }

    /// Construct a copy of `ptr`.
    ///
    /// Only enabled if `U*` is convertible to `T*`.
    ///
    template<typename U>
    SLANG_FORCE_INLINE ValPtr(
        ValPtr<U> ptr,
        std::enable_if_t<std::is_convertible_v<U*, T*>, void>* = nullptr)
        : _ref(*ptr)
    {
    }

    /// Get a pointer to the value being referenced.
    ///
    SLANG_FORCE_INLINE T* getDataPtr() const { return _ref.getDataPtr(); }

    /// Get the layout of the value being referenced.
    ///
    SLANG_FORCE_INLINE TargetLayout const* getLayout() const { return _ref.getLayout(); }

    SLANG_FORCE_INLINE T* get() const { return _ref.getDataPtr(); }
    SLANG_FORCE_INLINE operator T*() const { return get(); }

    /// Deference this `ValPtr` to get a `ValRef`.
    ///
    SLANG_FORCE_INLINE ValRef<T> operator*() const { return _ref; }

    /// Deference this `ValPtr` for member access.
    ///
    /// Note that an overloaded `operator->` must return either
    /// a pointer or a type that itself overloads `operator->`.
    /// Because `ValRef<T>` is not functionally a "smart pointer"
    /// to a `T`, the logical behavior here is that we want
    /// `someValPtr->foo` to be equvialent to `someValRef.foo`,
    /// where `someValRef` is a reference to the same value
    /// that `someValPtr` points to. The correct way to get
    /// that behavior is for the `operator->` on `ValPtr`
    /// to return a pointer to a `ValRef`.
    ///
    SLANG_FORCE_INLINE ValRef<T> const* operator->() const { return &_ref; }

private:
    ValRef<T> _ref;
};

/// Get a `ValPtr` pointing to the same value as the given `ref`.
///
template<typename T>
SLANG_FORCE_INLINE ValPtr<T> getAddress(ValRef<T> ref)
{
    return ValPtr<T>(ref);
}

using AnyValRef = ValRef<void>;
using AnyValPtr = ValPtr<void>;

//
// In order to make `ValRef<T>` more usable in contexts where we want
// to make use of the knowledge that it refers to a `T`, we define
// various specializations of `ValRef` for the specific types that
// are relevant for decoding serialized data.
//
// Note that we do not need to define any specializations of
// `ValPtr`, because that is ultimately just a wrapper around
// `ValRef`.
//

template<>
struct ValRef<FossilizedStringObj> : ValRefBase<FossilizedStringObj>
{
public:
    using ValRefBase<FossilizedStringObj>::ValRefBase;

    SLANG_FORCE_INLINE Size getSize() const { return getDataPtr()->getSize(); }
    SLANG_FORCE_INLINE UnownedTerminatedStringSlice get() const { return getDataPtr()->get(); }

    SLANG_FORCE_INLINE operator UnownedTerminatedStringSlice() const { return get(); }
};


template<>
struct ValRef<FossilizedContainerObjBase> : ValRefBase<FossilizedContainerObjBase>
{
public:
    using ValRefBase<FossilizedContainerObjBase>::ValRefBase;

    SLANG_FORCE_INLINE Count getElementCount() const
    {
        auto data = this->getDataPtr();
        if (!data)
            return 0;
        return data->getElementCount();
    }

    AnyValRef getElement(Index index) const;
};


template<>
struct ValRef<FossilizedArrayObjBase> : ValRefBase<FossilizedArrayObjBase>
{
public:
    using ValRefBase<FossilizedArrayObjBase>::ValRefBase;

    SLANG_FORCE_INLINE Count getElementCount() const
    {
        auto data = this->getDataPtr();
        if (!data)
            return 0;
        return data->getElementCount();
    }

    AnyValRef getElement(Index index) const;
};


template<>
struct ValRef<FossilizedDictionaryObjBase> : ValRefBase<FossilizedDictionaryObjBase>
{
public:
    using ValRefBase<FossilizedDictionaryObjBase>::ValRefBase;

    SLANG_FORCE_INLINE Count getElementCount() const
    {
        auto data = this->getDataPtr();
        if (!data)
            return 0;
        return data->getElementCount();
    }

    AnyValRef getElement(Index index) const;
};
template<>
struct ValRef<FossilizedOptionalObjBase> : ValRefBase<FossilizedOptionalObjBase>
{
public:
    using ValRefBase<FossilizedOptionalObjBase>::ValRefBase;

    SLANG_FORCE_INLINE bool hasValue() const { return this->getDataPtr() != nullptr; }

    SLANG_FORCE_INLINE AnyValRef getValue() const
    {
        SLANG_ASSERT(hasValue());
        return AnyValRef(this->getDataPtr(), this->getLayout()->elementLayout.get());
    }
};

template<>
struct ValRef<FossilizedRecordVal> : ValRefBase<FossilizedRecordVal>
{
public:
    using ValRefBase<FossilizedRecordVal>::ValRefBase;

    SLANG_FORCE_INLINE Count getFieldCount() const { return getLayout()->fieldCount; }

    AnyValRef getField(Index index) const;
};

template<typename T>
struct ValRef<FossilizedPtr<T>> : ValRefBase<FossilizedPtr<T>>
{
public:
    using ValRefBase<FossilizedPtr<T>>::ValRefBase;

    SLANG_FORCE_INLINE ValRef<T> getTargetValRef() const
    {
        auto ptrPtr = this->getDataPtr();
        return ValRef<T>(*ptrPtr, this->getLayout()->elementLayout.get());
    }

    SLANG_FORCE_INLINE ValPtr<T> getTargetValPtr() const { return ValPtr<T>(getTargetValRef()); }

    //    ValRef<T> operator*() const;
};

//
// We support both static and dynamic casting of `ValPtr`s
// to fossilized data. In the dynamic case, the layout
// information associated with the pointer is used to
// determine if the cast is allowed.
//

/// Statically cast a pointer to a fossilized value.
///
template<typename T>
SLANG_FORCE_INLINE ValPtr<T> cast(AnyValPtr valPtr)
{
    //    if (!valPtr)
    //        return ValPtr<T>();
    return ValPtr<T>(
        static_cast<T*>(valPtr.getDataPtr()),
        (typename T::Layout*)(valPtr.getLayout()));
}

/// Dynamic cast of a pointer to a fossilized value.
///
template<typename T>
SLANG_FORCE_INLINE ValPtr<T> as(AnyValPtr valPtr)
{
    if (!valPtr || !T::isMatchingKind(valPtr->getKind()))
    {
        return nullptr;
    }

    return ValPtr<T>(
        static_cast<T*>(valPtr.getDataPtr()),
        (typename T::Layout*)(valPtr->getLayout()));
}

} // namespace Fossil

/// Get a dynamically-typed pointer to the content of a fossilized variant.
///
/// This operation does not require a dynamically-typed `Fossil::ValPtr`
/// or `Fossil::ValRef` as input, because it makes use of the way that
/// a fossilized variant stores a (relative) pointer to the layout of
/// its content.
///
Fossil::AnyValPtr getVariantContentPtr(FossilizedVariantObj* variantPtr);

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

static_assert(sizeof(Header) == 32);

/// Get the root object from a fossilized blob.
///
/// This operation performs some basic validation on the blob to
/// ensure that it doesn't seem incorrectly sized or otherwise
/// corrupted/malformed.
///
Fossil::AnyValPtr getRootValue(ISlangBlob* blob);

/// Get the root object from a fossilized blob.
///
/// This operation performs some basic validation on the blob to
/// ensure that it doesn't seem incorrectly sized or otherwise
/// corrupted/malformed.
///
Fossil::AnyValPtr getRootValue(void const* data, Size size);
} // namespace Fossil

SLANG_FORCE_INLINE Size FossilizedStringObj::getSize() const
{
    auto sizePtr = (FossilUInt*)this - 1;
    return Size(*sizePtr);
}

SLANG_FORCE_INLINE UnownedTerminatedStringSlice FossilizedStringObj::get() const
{
    auto size = getSize();
    return UnownedTerminatedStringSlice((char*)this, size);
}

SLANG_FORCE_INLINE Count FossilizedContainerObjBase::getElementCount() const
{
    auto countPtr = (FossilUInt*)this - 1;
    return Size(*countPtr);
}

SLANG_FORCE_INLINE FossilizedValLayout* FossilizedVariantObj::getContentLayout() const
{
    auto layoutPtrPtr = (FossilizedPtr<FossilizedValLayout>*)this - 1;
    return (*layoutPtrPtr).get();
}

SLANG_FORCE_INLINE Fossil::AnyValPtr getVariantContentPtr(FossilizedVariantObj* variantPtr)
{
    return Fossil::AnyValPtr(variantPtr->getContentDataPtr(), variantPtr->getContentLayout());
}


} // namespace Slang

#endif
