// slang-fossil.h
#ifndef SLANG_FOSSIL_H
#define SLANG_FOSSIL_H

#include <stdarg.h>
#include <stdio.h>

static inline void _tessTrace(char const* message, ...)
{
    va_list args;
    va_start(args, message);

    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), message, args);

    fprintf(stderr, "TESS: %s\n", buffer);
}

#define TESS_TRACE(...) \
    do                          \
    {                           \
        _tessTrace(__VA_ARGS__);\
    } while (0)



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

// A key part of the fossil representation is the use of *relative pointers*,
// so that a fossilized object graph can be traversed dirctly in memory
// without having to deserialize any of the intermediate objects.
//
// Fossil uses 32-bit relative pointers, to keep the format compact.

namespace Fossil
{
template<typename T>
using RelativePtr = RelativePtr32<T>;
}

// Various other parts of the format need to store offsets or counts,
// and for consistency we will store them with the same number of
// bits as the relative pointers already used in the format.
//
// To make it easier for us to (potentially) change the relative
// pointer size down the line, we define type aliases for the
// general-purpose integer types that will be used in fossilized data.

using FossilInt = Fossil::RelativePtr<void>::Offset;
using FossilUInt = Fossil::RelativePtr<void>::UOffset;

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
struct FossilizedPtrLikeLayout;
struct FossilizedContainerLayout;
struct FossilizedRecordLayout;
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
    StringObj,
    ArrayObj,
    OptionalObj,
    DictionaryObj,
    Tuple,
    Struct,
    Ptr,
    VariantObj,
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
    using Layout = FossilizedValLayout;

protected:
    FossilizedVal() = default;
    FossilizedVal(FossilizedVal const&) = default;
    FossilizedVal(FossilizedVal&&) = default;
    ~FossilizedVal() = default;

    FossilizedVal& operator=(FossilizedVal const&) = default;
    FossilizedVal& operator=(FossilizedVal&&) = default;
};

template<typename T>
struct FossilizedPtr : FossilizedVal, Fossil::RelativePtr<T>
{
public:
    using Layout = FossilizedPtrLikeLayout;

    using Fossil::RelativePtr<T>::RelativePtr;
};

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

template<typename T>
struct FossilizedSimpleRef;

template<typename T, FossilizedValKind Kind>
struct FossilizedSimpleVal : FossilizedVal
{
public:
    static const FossilizedValKind kKind = Kind;

    T const& get() const { return _value; }

    operator T const&() const { return _value; }

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

//
// The `bool` type shouldn't be fossilized as itself, because
// its layout is not guaranteed to be consistent across targets.
// We instead fossilize it as an underlying `uint8_t`, and convert
// on reads.
//

template<>
struct Fossilized_<bool> : FossilizedVal
{
public:
public:
    static const FossilizedValKind kKind = FossilizedValKind::Bool;

    bool get() const { return _value != 0; }

    operator bool() const { return get(); }

private:
    uint8_t _value;
};

//
// Some simple types can be fossilized as one of the
// scalar types above, with explicit casts between
// the "live" type and the "fossilized" type.
//
// A common example of this is `enum` types.
//

template<typename LiveType, typename FossilizedAsType>
struct FossilizedViaCastVal : FossilizedVal
{
public:
    LiveType get() const { return LiveType(_value.get()); }

    operator LiveType() const { return get(); }


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

struct FossilizedStringObj : FossilizedVal
{
public:
    Size getSize() const;
    UnownedTerminatedStringSlice get() const;

    operator UnownedTerminatedStringSlice() const { return get(); }

private:
    // Before the `this` address, there is a `FossilUInt`
    // with the size of the string in bytes.
    //
    // At the `this` address there is a nul-terminated
    // sequence of `getSize() + 1` bytes.
};

struct FossilizedString : FossilizedVal
{
public:
    Size getSize() const { return _obj ? _obj->getSize() : 0; }

    UnownedTerminatedStringSlice get() const
    {
        return _obj ? _obj->get() : UnownedTerminatedStringSlice();
    }

    operator UnownedTerminatedStringSlice() const { return get(); }

private:
    FossilizedPtr<FossilizedStringObj> _obj;
};

inline int compare(FossilizedString const& lhs, UnownedStringSlice const& rhs)
{
    return compare(lhs.get(), rhs);
}

inline bool operator==(FossilizedString const& left, UnownedStringSlice const& right)
{
    return left.get() == right;
}

inline bool operator!=(FossilizedString const& left, UnownedStringSlice const& right)
{
    return left.get() != right;
}

inline bool operator==(FossilizedStringObj const& left, UnownedStringSlice const& right)
{
    return left.get() == right;
}

inline bool operator!=(FossilizedStringObj const& left, UnownedStringSlice const& right)
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

//
// The array and dictionary types are handled largely
// the same as strings, with the added detail that the
// object type is split into a base type without the
// template parameters, and a subtype that has those
// parameters. The base type enables navigating of
// these containers dynamically, based on layout.
//

struct FossilizedContainerObjBase : FossilizedVal
{
public:
    using Layout = FossilizedContainerLayout;

    Count getElementCount() const;

    void const* getBuffer() const { return this; }

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

template<typename T>
struct FossilizedContainer : FossilizedVal
{
public:
    Count getElementCount() const
    {
        if (!_obj)
            return 0;
        return _obj->getElementCount();
    }
    T const* getBuffer() const
    {
        if (!_obj)
            return nullptr;
        return (T const*)_obj.get()->getBuffer();
    }

    T const* begin() const { return getBuffer(); }
    T const* end() const { return getBuffer() + getElementCount(); }

private:
    FossilizedPtr<FossilizedContainerObj<T>> _obj;
};


struct FossilizedArrayObjBase : FossilizedContainerObjBase
{
};

template<typename T>
struct FossilizedArrayObj : FossilizedArrayObjBase
{
};

template<typename T>
struct FossilizedArray : FossilizedContainer<T>
{
public:
    T const& operator[](Index index) const
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


//
// TODO: need to handle pointers here, with some clever logic...
//

namespace details
{
template<typename T>
T fossilizedPointerTargetType(T*, void*);

template<typename T>
FossilizedStringObj fossilizedPointerTargetType(T*, FossilizedString*);
} // namespace details

template<typename T>
using FossilizedPointerTarget = decltype(details::fossilizedPointerTargetType(
    std::declval<Fossilized<T>*>(),
    std::declval<Fossilized<T>*>()));


template<typename T>
struct FossilizedTypeTraits<T*>
{
    using FossilizedType = FossilizedPtr<FossilizedPointerTarget<T>>;
};

template<typename T>
struct FossilizedTypeTraits<RefPtr<T>>
{
    using FossilizedType = FossilizedPtr<FossilizedPointerTarget<T>>;
};


//
// An optional value is effectively just a pointer, with
// the null case being used to represent the absence of
// a value.
//

struct FossilizedOptionalObjBase : FossilizedVal
{
public:
    using Layout = FossilizedPtrLikeLayout;

    FossilizedVal* getValue() { return this; }

    FossilizedVal const* getValue() const { return this; }

private:
    // An absent optional is encoded as a null pointer
    // (so `this` would be null), while a present value
    // is encoded as a pointer to that value. Thus the
    // held value is at the same address as `this`.
};

template<typename T = FossilizedVal>
struct FossilizedOptionalObj : FossilizedOptionalObjBase
{
};

template<typename T>
struct FossilizedOptional : FossilizedVal
{
public:
    explicit operator bool() const { return _value.get() != nullptr; }
    T const& operator*() const { return *_value.get(); }

private:
    FossilizedPtr<T> _value;
};

template<typename T>
struct FossilizedTypeTraits<std::optional<T>>
{
    using FossilizedType = FossilizedOptional<FossilizedPointerTarget<T>>;
};

//
// Many user-defined types will be fossilized as some kind
// of "record" (either a tuple or struct).
//

struct FossilizedRecordVal : FossilizedVal
{
public:
    using Layout = FossilizedRecordLayout;
};

struct FossilizedTupleVal : FossilizedRecordVal
{
};

struct FossilizedStructVal : FossilizedRecordVal
{
};

//
// A *variant* is a value that can conceptually hold data of any type/layout,
// and stores a pointer to layout information so that the data it holds
// can be navigated dynamically.
//

struct FossilizedVariantObj : FossilizedVal
{
public:
    static const FossilizedValKind kKind = FossilizedValKind::VariantObj;

    FossilizedValLayout* getContentLayout() const;

    FossilizedVal* getContentDataPtr() { return this; }
    FossilizedVal const* getContentDataPtr() const { return this; }

private:
    // Before the `this` address, there is a `FossilizedPtr<FossilizedValLayout>`
    // with the layout of the content.
    //
    // The content itself starts at the `this` address, with its
    // layout determined by `getContentLayout()`.
};

struct FossilizedVariant : FossilizedVal
{
public:
private:
    FossilizedPtr<FossilizedVariantObj> _obj;
};

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

/// A reference to a fossilized value in memory, and its layout.
///
template<typename T>
struct DynRefBase
{
public:
    using Layout = typename T::Layout;

    /// Construct a null reference.
    ///
    DynRefBase() {}

    /// Construct a reference to the given `dataPtr`, assuming it has the given `layout`.
    ///
    DynRefBase(T* dataPtr, Layout* layout)
        : _dataPtr(dataPtr), _layout(layout)
    {
    }

    template<typename U>
    DynRefBase(
        DynRefBase<U> const& other,
        std::enable_if_t<std::is_convertible_v<U*, T*>, void*> = nullptr)
        : _dataPtr(other.getDataPtr()), _layout((Layout*)other.getLayout())
    {
    }

    /// Get the kind of value being referenced.
    ///
    /// This reference must not be null.
    ///
    FossilizedValKind getKind() const
    {
        TESS_TRACE("DynRefBase::getKind()");
        TESS_TRACE("    this:%p", this);
        TESS_TRACE("    _dataPtr:%p", _dataPtr);
        TESS_TRACE("    _layout:%p", _layout);

        SLANG_ASSERT(getLayout());
        return getLayout()->kind;
    }

    /// Get the layout of the value being referenced.
    ///
    Layout* getLayout() const { return _layout; }

    /// Get a pointer to the value being referenced.
    ///
    T* getDataPtr() const { return _dataPtr; }

    bool isNull() const { return getDataPtr() == nullptr; }

    operator T const&() const { return *_dataPtr; }

    //    T const& get() { return *_data; }

    //    operator FossilizedVal*() const { return _data; }

    //    FossilizedVal* operator->() const { return _data; }

private:
    T* _dataPtr = nullptr;
    Layout* _layout = nullptr;
};

template<typename T>
struct DynRef : DynRefBase<T>
{
public:
    using DynRefBase<T>::DynRefBase;
};

template<typename T>
struct DynPtr
{
public:
    DynPtr() = default;
    DynPtr(DynPtr<T> const&) = default;

    DynPtr(T* ptr, typename T::Layout* layout)
        : _ref(ptr, layout)
    {
    }

    DynPtr(DynRef<FossilizedPtr<T>> const& ptrRef)
        : _ref(ptrRef.getTarget())
    {
    }

    explicit DynPtr(DynRefBase<T> const& ref)
        : _ref(ref.getDataPtr(), ref.getLayout())
    {
    }

    template<typename U>
    DynPtr(DynPtr<U> const& ptr, std::enable_if_t<std::is_convertible_v<U*, T*>, void*> = nullptr)
        : _ref(*ptr)
    {
    }


    DynPtr<T>& operator=(DynPtr<T> const&) = default;

    T* get() const { return _ref.getDataPtr(); }

    DynRef<T> operator*() const { return _ref; }

    DynRef<T> const* operator->() const { return &_ref; }
    operator DynRef<T> const*() const { return &_ref; }
    operator T*() const { return _ref.getDataPtr(); }

    explicit operator bool() { return !_ref.isNull(); }

private:
    DynRef<T> _ref;
};


//
// We support dynamic casting of `DynRef`s to the various important
// sub-types of `FossilizedVal`.
//

namespace detail
{
template<typename T>
struct DynamicCastHelper
{
    static bool isMatchingKind(FossilizedValKind kind) { return kind == T::kKind; }
};

template<>
struct DynamicCastHelper<FossilizedVal>
{
    static bool isMatchingKind(FossilizedValKind) { return true; }
};

template<>
struct DynamicCastHelper<FossilizedStringObj>
{
    static bool isMatchingKind(FossilizedValKind kind)
    {
        return kind == FossilizedValKind::StringObj;
    }
};

template<>
struct DynamicCastHelper<FossilizedArrayObjBase>
{
    static bool isMatchingKind(FossilizedValKind kind)
    {
        return kind == FossilizedValKind::ArrayObj;
    }
};

template<>
struct DynamicCastHelper<FossilizedDictionaryObjBase>
{
    static bool isMatchingKind(FossilizedValKind kind)
    {
        return kind == FossilizedValKind::DictionaryObj;
    }
};

template<>
struct DynamicCastHelper<FossilizedContainerObjBase>
{
    static bool isMatchingKind(FossilizedValKind kind)
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
};

template<>
struct DynamicCastHelper<FossilizedOptionalObj<FossilizedVal>>
{
    static bool isMatchingKind(FossilizedValKind kind)
    {
        return kind == FossilizedValKind::OptionalObj;
    }
};

template<>
struct DynamicCastHelper<FossilizedPtr<FossilizedVal>>
{
    static bool isMatchingKind(FossilizedValKind kind) { return kind == FossilizedValKind::Ptr; }
};

template<>
struct DynamicCastHelper<FossilizedStructVal>
{
    static bool isMatchingKind(FossilizedValKind kind) { return kind == FossilizedValKind::Struct; }
};

template<>
struct DynamicCastHelper<FossilizedTupleVal>
{
    static bool isMatchingKind(FossilizedValKind kind) { return kind == FossilizedValKind::Tuple; }
};

template<>
struct DynamicCastHelper<FossilizedRecordVal>
{
    static bool isMatchingKind(FossilizedValKind kind)
    {
        switch (kind)
        {
        default:
            return false;

        case FossilizedValKind::Tuple:
        case FossilizedValKind::Struct:
            return true;
        }
    }
};
} // namespace detail

/// Statically cast a reference to a fossilized value.
///
template<typename T, typename U>
DynRef<T> cast(DynRef<U> valRef)
{
    if (valRef.isNull())
        return DynRef<T>();
    return DynRef<T>(
        static_cast<T*>(valRef.getDataPtr()),
        reinterpret_cast<typename T::Layout*>(valRef.getLayout()));
}

/// Statically cast a pointer to a fossilized value.
///
template<typename T, typename U>
DynPtr<T> cast(DynPtr<U> valPtr)
{
    if (!valPtr)
        return DynPtr<T>();
    return DynPtr<T>(
        static_cast<T*>(valPtr.get()),
        reinterpret_cast<typename T::Layout*>(valPtr->getLayout()));
}

/// Dynamic cast of a reference to a fossilized value.
///
template<typename T, typename U>
DynPtr<T> as(DynPtr<U> valPtr)
{
    TESS_TRACE("if (!valPtr || !detail::DynamicCastHelper<T>::isMatchingKind(valPtr->getKind()))");
    if (!valPtr || !detail::DynamicCastHelper<T>::isMatchingKind(valPtr->getKind()))
    {
        TESS_TRACE("return DynPtr<T>();");
        return DynPtr<T>();
    }

    return DynPtr<T>(
        static_cast<T*>(valPtr->getDataPtr()),
        reinterpret_cast<typename T::Layout*>(valPtr->getLayout()));
}


template<>
struct DynRef<FossilizedVal> : DynRefBase<FossilizedVal>
{
public:
    using DynRefBase<FossilizedVal>::DynRefBase;

    DynRef(FossilizedVariantObj* valPtr)
        : DynRefBase<FossilizedVal>(valPtr->getContentDataPtr(), valPtr->getContentLayout())
    {
    }
};

using FossilizedValRef = DynRef<FossilizedVal>;

template<typename T, FossilizedValKind K>
struct DynRef<FossilizedSimpleVal<T, K>> : DynRefBase<FossilizedSimpleVal<T, K>>
{
public:
    using DynRefBase<FossilizedSimpleVal<T, K>>::DynRefBase;

    T const& get() const { return *(this->getDataPtr()); }
};

template<>
struct DynRef<Fossilized<bool>> : DynRefBase<Fossilized<bool>>
{
public:
    using DynRefBase<Fossilized<bool>>::DynRefBase;

    bool get() const { return *(this->getDataPtr()); }
};

template<>
struct DynRef<FossilizedRecordVal> : DynRefBase<FossilizedRecordVal>
{
public:
    using DynRefBase<FossilizedRecordVal>::DynRefBase;

    Count getFieldCount() const { return getLayout()->fieldCount; }

    DynRef<FossilizedVal> getField(Index index) const;
};

template<>
struct DynRef<FossilizedStringObj> : DynRefBase<FossilizedStringObj>
{
public:
    using DynRefBase<FossilizedStringObj>::DynRefBase;

    UnownedTerminatedStringSlice get() const { return getDataPtr()->get(); }
};

template<typename T>
struct DynRef<FossilizedOptionalObj<T>> : DynRefBase<FossilizedOptionalObj<T>>
{
public:
    using DynRefBase<FossilizedOptionalObj<T>>::DynRefBase;

    bool hasValue() const { return this->getDataPtr() != nullptr; }

    DynRef<T> getValue() const
    {
        SLANG_ASSERT(hasValue());
        return DynRef<T>(this->getDataPtr(), this->getLayout()->elementLayout.get());
    }
};

template<typename T>
struct DynRef<FossilizedPtr<T>> : DynRefBase<FossilizedPtr<T>>
{
public:
    using DynRefBase<FossilizedPtr<T>>::DynRefBase;

    DynRef<T> getTarget() const
    {
        auto ptrPtr = this->getDataPtr();
        return DynRef<T>(*ptrPtr, this->getLayout()->elementLayout.get());
    }

    operator T*() const { return this->getTarget().getDataPtr(); }
};

template<>
struct DynRef<FossilizedContainerObjBase> : DynRefBase<FossilizedContainerObjBase>
{
public:
    using DynRefBase<FossilizedContainerObjBase>::DynRefBase;

    Count getElementCount() const
    {
        auto data = this->getDataPtr();
        if (!data)
            return 0;
        return data->getElementCount();
    }

    DynRef<FossilizedVal> getElement(Index index) const;

    struct Iterator
    {
    public:
        Iterator(DynRef<FossilizedContainerObjBase> const& container, Index index)
            : _container(container), _index(index)
        {
        }
        bool operator!=(Iterator const& other) const { return _index != other._index; }
        void operator++() { _index++; }
        DynRef<FossilizedVal> operator*() const { return _container.getElement(_index); }

    private:
        DynRef<FossilizedContainerObjBase> const& _container;
        Index _index;
    };

    Iterator begin() const { return Iterator(*this, 0); }
    Iterator end() const { return Iterator(*this, getElementCount()); }
};

template<>
struct DynRef<FossilizedArrayObjBase> : DynRef<FossilizedContainerObjBase>
{
public:
    using DynRef<FossilizedContainerObjBase>::DynRef;
};

template<>
struct DynRef<FossilizedDictionaryObjBase> : DynRef<FossilizedContainerObjBase>
{
public:
    using DynRef<FossilizedContainerObjBase>::DynRef;
};

template<typename T>
struct DynRef<FossilizedContainer<T>> : DynRefBase<FossilizedContainer<T>>
{
public:
    using DynRefBase<FossilizedContainer<T>>::DynRefBase;

    Count getElementCount() const { return this->getDataPtr()->getElementCount(); }

    DynRef<T> getElement(Index index) const;

    struct Iterator
    {
    public:
        Iterator(DynRef<FossilizedContainer<T>> const& container, Index index)
            : _container(container), _index(index)
        {
        }
        bool operator!=(Iterator const& other) const { return _index != other._index; }
        void operator++() { _index++; }
        DynRef<T> operator*() const { return _container.getElement(_index); }

    private:
        DynRef<FossilizedContainer<T>> const& _container;
        Index _index;
    };

    Iterator begin() const { return Iterator(*this, 0); }
    Iterator end() const { return Iterator(*this, getElementCount()); }
};

template<typename T>
struct DynRef<FossilizedArray<T>> : DynRefBase<FossilizedArray<T>>
{
public:
    using DynRefBase<FossilizedArray<T>>::DynRefBase;
};

template<typename K, typename V>
struct DynRef<FossilizedKeyValuePair<K, V>> : DynRef<FossilizedRecordVal>
{
public:
    using DynRef<FossilizedRecordVal>::DynRef;

    DynRef<K> getKey() const { return cast<K>(this->getField(0)); }

    DynRef<V> getValue() const { return cast<V>(this->getField(1)); }
};

template<typename K, typename V>
struct DynRef<FossilizedDictionary<K, V>>
    : DynRef<FossilizedContainer<FossilizedKeyValuePair<K, V>>>
{
public:
    using DynRef<FossilizedContainer<FossilizedKeyValuePair<K, V>>>::DynRef;
};


#if 0
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
#endif

#if 0
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
#endif


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


using FossilizedValPtr = DynPtr<FossilizedVal>;


#if 0
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
#endif

#if 0
FossilizedValRef getPtrTarget(FossilizedPtrValRef ptrRef);

bool hasValue(FossilizedOptionalObjRef optionalRef);
FossilizedValRef getValue(FossilizedOptionalObjRef optionalRef);

Count getElementCount(FossilizedContainerObjRef containerRef);
FossilizedValRef getElement(FossilizedContainerObjRef containerRef, Index index);

Count getFieldCount(FossilizedRecordValRef recordRef);
FossilizedValRef getField(FossilizedRecordValRef recordRef, Index index);

FossilizedValRef getVariantContent(FossilizedVariantObjRef variantRef);
#endif
FossilizedValPtr getVariantContentPtr(FossilizedVariantObj* variantPtr);

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
FossilizedValPtr getRootValue(ISlangBlob* blob);

/// Get the root object from a fossilized blob.
///
/// This operation performs some basic validation on the blob to
/// ensure that it doesn't seem incorrectly sized or otherwise
/// corrupted/malformed.
///
FossilizedValPtr getRootValue(void const* data, Size size);
} // namespace Fossil

} // namespace Slang

#endif
