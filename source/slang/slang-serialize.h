// slang-serialize.h
#ifndef SLANG_SERIALIZE_H
#define SLANG_SERIALIZE_H

// This file defines an API for serialization.
//
// The API is intended to support multiple serialization formats,
// and to work with complicated object graphs that may include
// shared pointers, circular references, and so on.
//
// For anybody who don't want to dig into the details, the short
// version is that if you have a user-defined type like:
//
//      // my-thing.h
//      ...
//
//      struct MyThing
//      {
//          float a;
//          List<OtherThing> otherThings;
//          SomeObject* object;
//      };
//
// then you can implement serialization support for your type
// with something like:
//
//      template<typename S>
//      void serialize(S const& serializer, MyThing& value)
//      {
//          SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
//          serialize(serializer, value.a);
//          serialize(serializer, value.otherThings);
//          serialize(serializer, value.object);
//      }
//
// That's it. So long as the `OtherThing` and `SomeObject` types used
// in the declaration of `MyType` already implemented serialization
// support, your new type should be fully serializable.
//

#include "../core/slang-basic.h"

#include <optional>

namespace Slang
{

//
// A central design choice of this serialization system is that
// both reading and writing of serialized data for a type are
// implemented using a single function. This choice makes it
// easier for a developer to be certain that the reading and
// writing code for a type are consistent with one another.
//
// In some cases, however, a serialization function may need
// to know whether it is reading or writing serialized data.
// For that reason, we define a simple `enum` to represent
// the different modes of operation.
//

/// Whether serialized data is being read or written.
enum class SerializationMode
{
    Read,
    Write,
};

//
// Complex pointer-based object graphs can be challenging for
// a serialization system. Naively written recursive serialization
// logic can lead to extremely deep call graphs at runtime
// (potentially overflowing the stack), and when deserialization
// needs to interact with systems for caching and/or deduplicating
// objects, the ordering constraints imposed by those systems
// must be respected.
//
// This serialization system relies on callback functions in
// a few places to allow serialization actions to be queued up or
// otherwise rescheduled to avoid these problems.
//

/// Callback type used by the serialization infrastructure.
///
/// Users typically will never define callbacks manually; the
/// system creates and manages them behind the scenes, based
/// on user-defined types.
///
typedef void (*SerializerCallback)(void* valuePtr, void* impl, void* context);

//
// In order to achieve the best possible performance,
// serialization functions need to be statically specialized
// to the particular data format being used. To that
// end, concrete serialization formats are expected to
// model the right concept (provide the right methods,
// type declarations, etc.), but there is not a single
// base class or interface that they all *must* inherit from.
//
// However, to help illustrate the requirements for a
// complete serializer implementation, we will declare an
// interface that helps illustrate the outline of what
// a serializer implementation needs to provide.
//
// Note: this interface is somewhat user-unfriendly. It
// is *not* intended to be something that user code would
// usually interact with directly, and instead is intended
// to define only the operations that a serialization
// back-end must support.
//

/// Base interface for serialization.
///
/// Can be used for both reading and writing of serialized data.
///
/// Note that implementations of serialization back-ends do
/// *not* need to inherit from this type; it currently serves
/// only to define the requirements.
///
struct ISerializerImpl
{
    /// Get the mode that this serializer is operating in (reading or writing).
    virtual SerializationMode getMode() = 0;

    /// Handle a boolean value.
    ///
    /// If the serializer is writing, then `value` will be
    /// written to the serialized format.
    ///
    /// If the serializer is reading, then `value` will be
    /// set to the value read from the serialized format.
    ///
    virtual void handleBool(bool& value) = 0;

    /// Handle an integer value.
    ///
    /// If the serializer is writing, then `value` will be
    /// written to the serialized format.
    ///
    /// If the serializer is reading, then `value` will be
    /// set to the value read from the serialized format.
    ///
    virtual void handleInt8(int8_t& value) = 0;

    /// Handle an integer value.
    ///
    /// If the serializer is writing, then `value` will be
    /// written to the serialized format.
    ///
    /// If the serializer is reading, then `value` will be
    /// set to the value read from the serialized format.
    ///
    virtual void handleInt16(int16_t& value) = 0;

    /// Handle an integer value.
    ///
    /// If the serializer is writing, then `value` will be
    /// written to the serialized format.
    ///
    /// If the serializer is reading, then `value` will be
    /// set to the value read from the serialized format.
    ///
    virtual void handleInt32(Int32& value) = 0;

    /// Handle an integer value.
    ///
    /// If the serializer is writing, then `value` will be
    /// written to the serialized format.
    ///
    /// If the serializer is reading, then `value` will be
    /// set to the value read from the serialized format.
    ///
    virtual void handleInt64(Int64& value) = 0;

    /// Handle an integer value.
    ///
    /// If the serializer is writing, then `value` will be
    /// written to the serialized format.
    ///
    /// If the serializer is reading, then `value` will be
    /// set to the value read from the serialized format.
    ///
    virtual void handleUInt8(uint8_t& value) = 0;

    /// Handle an integer value.
    ///
    /// If the serializer is writing, then `value` will be
    /// written to the serialized format.
    ///
    /// If the serializer is reading, then `value` will be
    /// set to the value read from the serialized format.
    ///
    virtual void handleUInt16(uint16_t& value) = 0;

    /// Handle an integer value.
    ///
    /// If the serializer is writing, then `value` will be
    /// written to the serialized format.
    ///
    /// If the serializer is reading, then `value` will be
    /// set to the value read from the serialized format.
    ///
    virtual void handleUInt32(UInt32& value) = 0;

    /// Handle an integer value.
    ///
    /// If the serializer is writing, then `value` will be
    /// written to the serialized format.
    ///
    /// If the serializer is reading, then `value` will be
    /// set to the value read from the serialized format.
    ///
    virtual void handleUInt64(UInt64& value) = 0;

    /// Handle a floating-point value.
    ///
    /// If the serializer is writing, then `value` will be
    /// written to the serialized format.
    ///
    /// If the serializer is reading, then `value` will be
    /// set to the value read from the serialized format.
    ///
    virtual void handleFloat32(float& value) = 0;

    /// Handle a floating-point value.
    ///
    /// If the serializer is writing, then `value` will be
    /// written to the serialized format.
    ///
    /// If the serializer is reading, then `value` will be
    /// set to the value read from the serialized format.
    ///
    virtual void handleFloat64(double& value) = 0;

    /// Handle a string value.
    ///
    /// If the serializer is writing, then `value` will be
    /// written to the serialized format.
    ///
    /// If the serializer is reading, then `value` will be
    /// set to the value read from the serialized format.
    ///
    virtual void handleString(String& value) = 0;

    //
    // Every concrete serializer implementation should define
    // its own `Scope` type, which will be passed as the
    // operand to the various paired `begin`/`end` operations
    // below. Code that is calling into a serializer takes
    // responsibility for maintaining the `Scope` for the
    // duration of the `begin`/`end` pair, and passing the
    // same data in to `end` that was passed into `begin`,
    // without making any modifications of its own.
    //
    // An implementation may be able to use the `Scope` to
    // store information related to the hierarchy of `begin`
    // and `end` operations, and thus avoid needing to
    // maintain its own heap-allocated stack structure.
    //
    struct Scope
    {
        // The implementation of `Scope` in this placeholder
        // interface is intended to define a kind of upper
        // bound on the size of reasonable scope representations.
        //
        // The intention is that any concrete implementation
        // could be wrapped up as an `ISerializerImpl`, so
        // long as its `Scope` fits within the storage limit
        // defined here.
        //
        void* storage[8];
    };

    /// Begin serializing an array value.
    ///
    /// An array should be used to serialize an
    /// unkeyed homogeneous collection of a varying
    /// number of elements.
    ///
    /// This operation must be properly paired with a
    /// call to `endArray()`.
    ///
    /// When writing, the values serialized between `beginArray()`
    /// and `endArray()` will be written as the elements of a
    /// serialized array.
    ///
    /// When reading, the user should call `hasElements()` to
    /// test whether there are elements remaining to be read,
    /// and serialize values in a loop until `hasElements()`
    /// returns `false`.
    ///
    virtual void beginArray(Scope&) = 0;

    /// End serializing an array value.
    virtual void endArray(Scope&) = 0;

    /// Begin serializing an optional value.
    ///
    /// An optional should be used to serialize a
    /// collection that logically has either zero
    /// or one element.
    ///
    /// This operation must be properly paired with a
    /// call to `endOptional()`.
    ///
    /// When writing, a value serialized between `beginOptional()`
    /// and `endOptional()` will be written as the value of
    /// the serialized optional. If no value is serialized,
    /// then the optional will be empty.
    ///
    /// When reading, the user should call `hasElements()` to
    /// test whether the serialized optional has a value and,
    /// if it does, read the value before calling `endOptional()`.
    ///
    virtual void beginOptional(Scope&) = 0;

    /// End serializing an optional value.
    virtual void endOptional(Scope&) = 0;

    /// Begin serializing a dictionary value.
    ///
    /// A dictionary should be used to serialize a
    /// keyed homogeneous collection of a varying
    /// number of elements. The elements of a dictioanry
    /// are key-value pairs (that is, two-element tuples).
    ///
    /// Formats are required to support dictionaries with
    /// any serializable type as the key, not just strings.
    ///
    /// This operation must be properly paired with a
    /// call to `endDictionary()`.
    ///
    /// When writing, the values serialized between `beginDictionary()`
    /// and `endDictionary()` will be written as the elements of a
    /// serialized dictionary.
    ///
    /// When reading, the user should call `hasElements()` to
    /// test whether there are elements remaining to be read,
    /// and serialize values in a loop until `hasElements()`
    /// returns `false`.
    ///
    virtual void beginDictionary(Scope&) = 0;

    /// End serializing a dictionary value.
    virtual void endDictionary(Scope&) = 0;

    /// Check whether there are elements remaining to be read
    /// from a serialized container.
    ///
    /// It is invalid to call this function except between paired
    /// `beginArray()`/`endArray()`, beginDictionary()`/`endDictionary()`,
    /// or `beginOptional()`/`endOptional()` calls.
    ///
    virtual bool hasElements() = 0;

    /// Begin serializing a tuple value.
    ///
    /// A tuple should be used to serialize an
    /// unkeyed heterogeneous collection of a fixed
    /// number of elements.
    ///
    /// It is up to the concrete implementation whether calls
    /// to `hasElements()` are allowed between `beginTuple()`
    /// and `endTuple()`.
    ///
    virtual void beginTuple(Scope&) = 0;

    /// End serializing a tuple value.
    virtual void endTuple(Scope&) = 0;


    /// Begin serializing a struct value.
    ///
    /// A struct should be used to serialize an
    /// keyed heterogeneous collection of a fixed
    /// number of elements.
    ///
    /// The value of each struct field should be
    /// preceded by a call to `handleFieldKey()`,
    /// which specifies the field being serialized.
    ///
    /// It is up to the concrete implementation whether
    /// a fields can be read in a different order than
    /// they were written, and how to handle attempts
    /// to read a field that was not written.
    ///
    virtual void beginStruct(Scope&) = 0;

    /// End serializing a struct value.
    virtual void endStruct(Scope&) = 0;

    /// Begin serializing a variant value.
    ///
    /// A variant should be used to serialize any type
    /// that behaves like a "tagged union," where different
    /// instances may have different sequences of members,
    /// of different types.
    ///
    /// User code reading from a variant must be able to
    /// use the members read so far to determine what
    /// members it should read next (e.g., by serializing
    /// a tag enumerant first, followed by the tag-dependent
    /// members).
    ///
    /// A variant is otherwise like a struct. Some serializer
    /// implementations may treat variants just like structs,
    /// while others may rely on any type serialized as a
    /// struct always including the same members in the same
    /// order.
    ///
    virtual void beginVariant(Scope&) = 0;

    /// End serializing a variant value.
    virtual void endVariant(Scope&) = 0;

    /// Set the key for the next struct field to be serialized.
    ///
    /// If no name is available for the field, `name` may be `nullptr`.
    ///
    /// If no index is available for the field, `index` may be `-1`.
    ///
    /// A user must pass either a valid `name` or `index.
    ///
    virtual void handleFieldKey(char const* name, Int index) = 0;

    /// A callback function used to handle serialization of pointers.
    typedef void (*Callback)(void* valuePtr, void* impl, void* context);

    /// Handle a pointer value that is expected to be unique.
    ///
    /// A unique pointer is logically similar to an optional value.
    ///
    /// If the pointer value being read/written is null, then
    /// the function returns without invoking `callback`.
    ///
    /// When reading, if the serialized value is non-null,
    /// then the callback will be invoked as `callback(&value, userData)`.
    /// The callback is expected to read the members of the pointed-to
    /// type and set `value` to some object (whether newly constructed
    /// or looked up).
    ///
    /// When writing, if the `value` is non-null, then the callback
    /// will be invoked, either immediately or at some later point,
    /// as `callback(&ptr, this, context)` where `ptr` is a variable
    /// holding a copy of the `value` that was passed in. The callback
    /// is expected to write the members of the pointed-to type.
    ///
    /// If the `callback` is invoked at some later point, rather than
    /// immediately, the concrete serializer implementation is responsible
    /// for ensuring that its internal state has been restored to
    /// be compatible with what it was when `handleUniquePtr` was called.
    ///
    virtual void handleUniquePtr(void*& value, Callback callback, void* context) = 0;

    /// Handle a pointer value that may have multiple references.
    ///
    /// This operation is similar to `handleUniquePtr` with the following
    /// differences:
    ///
    /// * When writing, if the same pointer value has been seen before,
    ///   the `callback` will not be invoked, and instead an additional
    ///   reference to the previously-serialized value will be written.
    ///
    /// * When reading, if the serialized value has been read before,
    ///   the `callback` will not be invoked, and instead `value` will
    ///   be set to the pointer that was previously read.
    ///
    virtual void handleSharedPtr(void*& value, Callback callback, void* context) = 0;

    /// Defer serialization of the contents of an object.
    ///
    /// Used to delay serialization of members of an object that
    /// could cause infinite recursion if serialized eagerly.
    ///
    /// This operation should only be used in the body of a callback
    /// passed to `handleUniquePtr()` or `handleSharedPtr()`.
    ///
    /// This operation schedules the given `callback` to be called
    /// at some later point a `callback(value, this, context)`, with
    /// the state of the serializer implementation restored to what
    /// it was when `handleDeferredObjectContents()` was called.
    ///
    /// Some concrete serializer implementations might implement
    /// this operation by invoking `callback` immediately.
    ///
    virtual void handleDeferredObjectContents(void* value, Callback callback, void* context) = 0;
};

//
// Rather than interface with serialization back-ends directly,
// most client code will use a wrapper type that amounts to a kind
// of smart pointer.
//
// While serialization back-ends providing operations comparable to
// `ISerializerImpl` above can cover a wide range of
// types that need to be serialized, it is common for types to require
// more specific *context* to be available in order to perform serialization.
// For example, code might need access to a factory object in order
// to construct objects of a type being read.
//
// To support more specialized serializer implementations, the smart
// pointer type used for a serializer actually wraps *two* pointers:
// one for an `ISerializerImpl`-derived type, and one for a context
// type. The smart pointer is templated on both of these types.
//

/// Base type for serialization contexts.
///
/// The type parameter `I` should be the serialization back-end
/// implementation (e.g., `ISerializerImpl`), while the `C` type
/// parameter can be any type that carries additional context
/// information needed.
///
template<typename I, typename C>
struct Serializer
{
public:
    using Impl = I;
    using Context = C;

    SLANG_FORCE_INLINE Serializer() = default;
    SLANG_FORCE_INLINE Serializer(Impl* impl, Context* context = nullptr)
        : _impl(impl), _context(context)
    {
    }

    template<typename SourceImpl, typename SourceContext>
    SLANG_FORCE_INLINE Serializer(
        Serializer<SourceImpl, SourceContext> const& serializer,
        std::enable_if_t<
            std::is_convertible_v<SourceImpl*, Impl*> &&
                std::is_convertible_v<SourceContext*, Context*>,
            void>* = nullptr)
        : _impl(serializer.getImpl()), _context(serializer.getContext())
    {
    }

    SLANG_FORCE_INLINE Impl* getImpl() const { return _impl; }
    SLANG_FORCE_INLINE Context* getContext() const { return _context; }

    SLANG_FORCE_INLINE Impl* get() const { return _impl; }
    SLANG_FORCE_INLINE Impl* operator->() const { return get(); }


private:
    Impl* _impl = nullptr;
    Context* _context = nullptr;
};

//
// We define namespace-scope functions that mirror some
// of the operations of `ISerializerImpl`, to insulate
// user-defined `serialize()` function implementations
// a bit from the details of the serializer concept/interface.
//

/// Get the mode of `serializer`.
template<typename S>
SLANG_FORCE_INLINE SerializationMode getMode(S const& serializer)
{
    return serializer->getMode();
}

/// Check if `serializer` is reading serialized data.
template<typename S>
SLANG_FORCE_INLINE bool isReading(S const& serializer)
{
    return getMode(serializer) == SerializationMode::Read;
}

/// Check if `serializer` is writing serialized data.
template<typename S>
SLANG_FORCE_INLINE bool isWriting(S const& serializer)
{
    return getMode(serializer) == SerializationMode::Write;
}

/// Check if `serializer` has more container elements.
template<typename S>
SLANG_FORCE_INLINE bool hasElements(S const& serializer)
{
    return serializer->hasElements();
}

template<typename S>
SLANG_FORCE_INLINE void serialize(S const& serializer, bool& value)
{
    serializer->handleBool(value);
}

template<typename S>
SLANG_FORCE_INLINE void serialize(S const& serializer, int8_t& value)
{
    serializer->handleInt8(value);
}

template<typename S>
SLANG_FORCE_INLINE void serialize(S const& serializer, int16_t& value)
{
    serializer->handleInt16(value);
}

template<typename S>
SLANG_FORCE_INLINE void serialize(S const& serializer, Int32& value)
{
    serializer->handleInt32(value);
}

template<typename S>
SLANG_FORCE_INLINE void serialize(S const& serializer, Int64& value)
{
    serializer->handleInt64(value);
}

template<typename S>
SLANG_FORCE_INLINE void serialize(S const& serializer, uint8_t& value)
{
    serializer->handleUInt8(value);
}

template<typename S>
SLANG_FORCE_INLINE void serialize(S const& serializer, uint16_t& value)
{
    serializer->handleUInt16(value);
}

template<typename S>
SLANG_FORCE_INLINE void serialize(S const& serializer, UInt32& value)
{
    serializer->handleUInt32(value);
}

template<typename S>
SLANG_FORCE_INLINE void serialize(S const& serializer, UInt64& value)
{
    serializer->handleUInt64(value);
}

template<typename S>
SLANG_FORCE_INLINE void serialize(S const& serializer, float& value)
{
    serializer->handleFloat32(value);
}

template<typename S>
SLANG_FORCE_INLINE void serialize(S const& serializer, double& value)
{
    serializer->handleFloat64(value);
}

template<typename S>
SLANG_FORCE_INLINE void serialize(S const& serializer, String& value)
{
    serializer->handleString(value);
}

/// Serialize an `enum` value via an intermediate integer type.
///
/// This function serializes a value of `EnumType`, by
/// converting it to/from the given `RawType` for storage
/// in the serialized format.
///
template<typename RawType = Int32, typename S, typename EnumType>
SLANG_FORCE_INLINE void serializeEnum(S const& serializer, EnumType& value)
{
    auto raw = RawType(value);
    serialize(serializer, raw);
    value = EnumType(raw);
}

//
// We define a suite of simple RAII types to help users
// maintain the proper pairing of begin/end operations
// when interacting with an `ISerializerImpl`, and for
// each of those types we define a macro to simplify
// introducing a coresponding scope.
//
// These types handle the details of properly allocating
// and retaining a `Scope` value appropriate to the
// serializer implementation, and passing it into both
// the `begin` and `end` operations.
//

template<typename S>
struct ScopedSerializerArray
{
public:
    SLANG_FORCE_INLINE ScopedSerializerArray(S const& serializer)
        : _serializer(serializer)
    {
        serializer->beginArray(_scope);
    }

    SLANG_FORCE_INLINE ~ScopedSerializerArray() { _serializer->endArray(_scope); }

private:
    S _serializer;
    typename S::Impl::Scope _scope;
};

template<typename S>
struct ScopedSerializerDictionary
{
public:
    SLANG_FORCE_INLINE ScopedSerializerDictionary(S const& serializer)
        : _serializer(serializer)
    {
        serializer->beginDictionary(_scope);
    }

    SLANG_FORCE_INLINE ~ScopedSerializerDictionary() { _serializer->endDictionary(_scope); }

private:
    S _serializer;
    typename S::Impl::Scope _scope;
};

template<typename S>
struct ScopedSerializerStruct
{
public:
    SLANG_FORCE_INLINE ScopedSerializerStruct(S const& serializer)
        : _serializer(serializer)
    {
        serializer->beginStruct(_scope);
    }

    SLANG_FORCE_INLINE ~ScopedSerializerStruct() { _serializer->endStruct(_scope); }

private:
    S _serializer;
    typename S::Impl::Scope _scope;
};

template<typename S>
struct ScopedSerializerVariant
{
public:
    SLANG_FORCE_INLINE ScopedSerializerVariant(S const& serializer)
        : _serializer(serializer)
    {
        serializer->beginVariant(_scope);
    }

    SLANG_FORCE_INLINE ~ScopedSerializerVariant() { _serializer->endVariant(_scope); }

private:
    S _serializer;
    typename S::Impl::Scope _scope;
};

template<typename S>
struct ScopedSerializerTuple
{
public:
    SLANG_FORCE_INLINE ScopedSerializerTuple(S const& serializer)
        : _serializer(serializer)
    {
        serializer->beginTuple(_scope);
    }

    SLANG_FORCE_INLINE ~ScopedSerializerTuple() { _serializer->endTuple(_scope); }

private:
    S _serializer;
    typename S::Impl::Scope _scope;
};

template<typename S>
struct ScopedSerializerOptional
{
public:
    SLANG_FORCE_INLINE ScopedSerializerOptional(S const& serializer)
        : _serializer(serializer)
    {
        serializer->beginOptional(_scope);
    }

    SLANG_FORCE_INLINE ~ScopedSerializerOptional() { _serializer->endOptional(_scope); }

private:
    S _serializer;
    typename S::Impl::Scope _scope;
};


#define SLANG_SCOPED_SERIALIZER_ARRAY(SERIALIZER)                                               \
    ::Slang::ScopedSerializerArray<std::remove_reference_t<decltype(SERIALIZER)>> SLANG_CONCAT( \
        _scopedSerializerArray,                                                                 \
        __LINE__)(SERIALIZER)

#define SLANG_SCOPED_SERIALIZER_DICTIONARY(SERIALIZER)                                 \
    ::Slang::ScopedSerializerDictionary<std::remove_reference_t<decltype(SERIALIZER)>> \
    SLANG_CONCAT(_scopedSerializerDictionary, __LINE__)(SERIALIZER)

#define SLANG_SCOPED_SERIALIZER_OPTIONAL(SERIALIZER)                                               \
    ::Slang::ScopedSerializerOptional<std::remove_reference_t<decltype(SERIALIZER)>> SLANG_CONCAT( \
        _scopedSerializerOptional,                                                                 \
        __LINE__)(SERIALIZER)

#define SLANG_SCOPED_SERIALIZER_STRUCT(SERIALIZER)                                               \
    ::Slang::ScopedSerializerStruct<std::remove_reference_t<decltype(SERIALIZER)>> SLANG_CONCAT( \
        _scopedSerializerStruct,                                                                 \
        __LINE__)(SERIALIZER)

#define SLANG_SCOPED_SERIALIZER_VARIANT(SERIALIZER)                                               \
    ::Slang::ScopedSerializerVariant<std::remove_reference_t<decltype(SERIALIZER)>> SLANG_CONCAT( \
        _scopedSerializerVariant,                                                                 \
        __LINE__)(SERIALIZER)

#define SLANG_SCOPED_SERIALIZER_TUPLE(SERIALIZER)                                               \
    ::Slang::ScopedSerializerTuple<std::remove_reference_t<decltype(SERIALIZER)>> SLANG_CONCAT( \
        _scopedSerializerTuple,                                                                 \
        __LINE__)(SERIALIZER)

//
// Containers like arrays and dictionaries are more
// difficult to serialize than typical user-defined
// types because they typically need to have distinct
// code paths for reading and writing, so they don't
// benefit much from having a unified read/write abstraction.
///
// That said, the definitions themselves
// are fairly straightforward. All we have to do is
// branch on whether we are reading or writing and
// either iterate over the serialized data to fill
// the collection (when reading), or iterate over
// the collection to serialize its elements (when
// writing).
//

template<typename S, typename T>
void serialize(S const& serializer, List<T>& value)
{
    SLANG_SCOPED_SERIALIZER_ARRAY(serializer);
    if (isWriting(serializer))
    {
        for (auto element : value)
            serialize(serializer, element);
    }
    else
    {
        value.clear();
        while (hasElements(serializer))
        {
            T element;
            serialize(serializer, element);
            value.add(element);
        }
    }
}

template<typename S, typename T, size_t N>
void serialize(S const& serializer, T (&value)[N])
{
    SLANG_SCOPED_SERIALIZER_ARRAY(serializer);
    if (isWriting(serializer))
    {
        for (auto element : value)
            serialize(serializer, element);
    }
    else
    {
        size_t index = 0;
        while (hasElements(serializer))
        {
            T element;
            serialize(serializer, element);

            if (index >= N)
            {
                SLANG_UNEXPECTED("serialized array too large");
            }
            value[index++] = element;
        }
    }
}

template<typename S, typename T, int N>
void serialize(S const& serializer, ShortList<T, N>& value)
{
    SLANG_SCOPED_SERIALIZER_ARRAY(serializer);
    if (isWriting(serializer))
    {
        for (auto element : value)
            serialize(serializer, element);
    }
    else
    {
        value.clear();
        while (hasElements(serializer))
        {
            T element;
            serialize(serializer, element);
            value.add(element);
        }
    }
}

template<typename S, typename T>
void serialize(S const& serializer, std::optional<T>& value)
{
    SLANG_SCOPED_SERIALIZER_OPTIONAL(serializer);
    if (isWriting(serializer))
    {
        if (value.has_value())
        {
            serialize(serializer, *value);
        }
    }
    else
    {
        value.reset();
        if (hasElements(serializer))
        {
            value.emplace();
            serialize(serializer, *value);
        }
    }
}

template<typename S, typename K, typename V>
void serialize(S const& serializer, KeyValuePair<K, V>& value)
{
    SLANG_SCOPED_SERIALIZER_TUPLE(serializer);
    serialize(serializer, value.key);
    serialize(serializer, value.value);
}

template<typename S, typename K, typename V>
SLANG_FORCE_INLINE void serialize(S const& serializer, std::pair<K, V>& value)
{
    SLANG_SCOPED_SERIALIZER_TUPLE(serializer);
    serialize(serializer, value.first);
    serialize(serializer, value.second);
}

template<typename S, typename K, typename V>
SLANG_FORCE_INLINE void serialize(S const& serializer, Dictionary<K, V>& value)
{
    SLANG_SCOPED_SERIALIZER_DICTIONARY(serializer);
    if (isWriting(serializer))
    {
        for (auto pair : value)
            serialize(serializer, pair);
    }
    else
    {
        value.clear();
        while (hasElements(serializer))
        {
            KeyValuePair<K, V> pair{K(), V()};
            serialize(serializer, pair);
            value.add(pair.key, pair.value);
        }
    }
}

template<typename S, typename K, typename V>
void serialize(S const& serializer, OrderedDictionary<K, V>& value)
{
    SLANG_SCOPED_SERIALIZER_DICTIONARY(serializer);
    if (isWriting(serializer))
    {
        for (auto pair : value)
            serialize(serializer, pair);
    }
    else
    {
        value.clear();
        while (hasElements(serializer))
        {
            KeyValuePair<K, V> pair{K(), V()};
            serialize(serializer, pair);
            value.add(pair.key, pair.value);
        }
    }
}

//
// Serialization of pointers is the most complicated part of
// the whole system. Dealing with pointers means contending with:
//
// * Multiply-referenced objects, or even cycles in the object graph.
//
// * Polymoprhic types, where a `Derived*` might get serialized
//   through a `Base*` pointer.
//
// * Types that require going through a factory function of
//   some kind as part of their creation (perhaps to implement
//   deduplication/caching).
//
// Our handling of pointers is thus broken down into several
// different steps/layers:
//
// * An ordinary overload of `serialize(s,v)` is used to intercept
//   pointer types `T*` and dispatched out to `serializePtr(s,v,(T*)nullptr)`.
//   Passing the additional `T*` argument allows different overloads
//   of `serializePtr` to intercept entire type hierarchies, while
//   still allowing for a fallback case.
//
// * Implementations of `serializePtr` are typically expected to
//   invoke either `serializeUniquePtr` or `serializeSharedPtr`, which
//   handle calling into the `ISerializerImpl` methods with appropriate
//   callbacks.
//
// * The `handleUniquePtr()` or `handleSharedPtr()` operation on
//   `ISerializerImpl` is expected to handle null pointers, or previously-
//   encountered pointers in the shared case, and then invoke the
//   callback to handle things when it can't early-out.
//
// * The callbacks will end up calling `serializeObject(s,v,(T*)nullptr)`,
//   which is another customization point. The default implementation
//   will call `new T()` when reading, so types that need more complicated
//   creation logic should intercept this specialization point.
//
// * An implementation of `serializeObject()` should strive to serialize
//   the bare minimum of members required to actually allocate the object
//   (in the case where serialized data is being read), and then call
//   `deferSerializeObjectContents()` to schedule the remainder of
//   the data to be serialized. Maintaining that policy helps ensure
//   that cycles in the object graph don't create problems.
//
// * `serializeObjectContents()` is the final customization point. By
//   default it simply takes a `T* value` and does `serialize(..., *value)`
//   to serialize the pointed-to `T` value. A custom implementation
//   should serialize whatever members of the object weren't handled
//   as part of the corresponding `serializeObject()` implementation.
//

template<typename S, typename T>
SLANG_FORCE_INLINE void serializeObjectContents(S const& serializer, T* value, void*)
{
    serialize(serializer, *value);
}

template<typename S, typename T>
void _serializeObjectContentsCallback(void* valuePtr, void* impl, void* context)
{
    S serializer((typename S::Impl*)impl, (typename S::Context*)context);
    auto value = (T*)valuePtr;
    serializeObjectContents(serializer, value, (T*)nullptr);
}

template<typename S, typename T>
SLANG_FORCE_INLINE void deferSerializeObjectContents(S const& serializer, T* value)
{
    serializer->handleDeferredObjectContents(
        value,
        _serializeObjectContentsCallback<S, T>,
        serializer.getContext());
}

template<typename S, typename T>
void serializeObject(S const& serializer, T*& value, void*)
{
    if (isReading(serializer))
    {
        value = new T();
    }
    deferSerializeObjectContents(serializer, value);
}

template<typename S, typename T>
void _serializeObjectCallback(void* valuePtr, void* impl, void* context)
{
    S serializer((typename S::Impl*)impl, (typename S::Context*)context);
    auto& value = *(T**)valuePtr;
    serializeObject(serializer, value, (T*)nullptr);
}

template<typename S, typename T>
SLANG_FORCE_INLINE void serializeSharedPtr(S const& serializer, T*& value)
{
    serializer->handleSharedPtr(
        *(void**)&value,
        _serializeObjectCallback<S, T>,
        serializer.getContext());
}

template<typename S, typename T>
SLANG_FORCE_INLINE void serializeUniquePtr(S const& serializer, T*& value)
{
    serializer->handleUniquePtr(
        *(void**)&value,
        _serializeObjectCallback<S, T>,
        serializer.getContext());
}

template<typename S, typename T>
SLANG_FORCE_INLINE void serializePtr(S const& serializer, T*& value, void*)
{
    serializeSharedPtr(serializer, value);
}

template<typename S, typename T>
SLANG_FORCE_INLINE void serialize(S const& serializer, T*& value)
{
    serializePtr(serializer, value, (T*)nullptr);
}

template<typename S, typename T>
SLANG_FORCE_INLINE void serialize(S const& serializer, RefPtr<T>& value)
{
    T* raw = value;
    serialize(serializer, raw);
    value = raw;
}

} // namespace Slang

#endif
