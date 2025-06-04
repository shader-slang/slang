Serialization
=============

Slang's infrastructure for serialization is currently in flux, so there exist a mixture of different subsystems, using a mixture of different techniques.

This document is curently minimal, and primarily serves to provide a replacement for an older draft that no longer reflects the state of the codebase.

The Fossil Format
=================

The "fossil" format is a memory-mappable binary format for general-purpose serialization.

Goals
-----

The main goals of the fossil format are:

* Data can be read from memory as-is.

  * Basic types are stored at offsets that are naturally aligned (e.g., a 4-byte integer is 4-byte aligned)

  * Pointers are encoded as relative offsets, and can be traversed without any "relocation" step after data is loaded.

* Supports general-purpose data, including complicated object graphs.

* Data can include embedded layout information, allowing code to traverse it without statically knowing the structure.

  * Embedded layout information should support versioning; new code should be able to load old data by notcing what has/hasn't been encoded.

* Layout information is *optional*, and data can be traversed with minimal overhead by code that knows/assumes the layout

Top-Level Structure
-------------------

A serialized blob in fossil format starts with a header (see `Slang::Fossil::Header`), which in turn points to the *root value*.
All other data in the blob should be reachable from the root value, and an application can choose to make the root value whatever type they want (an array, structure, etc.).

Encoding
--------

### Endian

All data is read/written in the endianness of the host machine.
There is currently no automatic support for encoding endian-ness as part of the format; a byte-order mark should be added if we ever need to support big-endian platforms.

### Fixed-Size Types

#### Basic Types

Basic types like fixed-width integers and floating-point numbers are encoded as-is.
That is, an N-byte value is stored directly as N bytes of data with N-byte alignment.

A Boolean value is encoded as an 8-bit unsigned integer holding either zero or one.

#### Pointers

A pointer is encoded as a 4-byte signed integer, representing a relative offset.

If the relative offset value is zero, then the pointer is null.
Otehrwise, the relative offset value should be added to the offset of the pointer itself, to get the offset of the target.

#### Optionals

An optional value of some type `T` (e.g., the equivalent of a `std::optional<T>`) is encoded as a pointer to a `T`.
If the pointer is null, the optional has no value; otherwise the value is stored at the offset being pointed to.

Note that when encoding a pointer to an optional (`std::optional<T> *`) or an optional pointer (`std::optional<T*>`), there will be two indirections.

#### Records

Things that are conceptually like a `struct` or tuple are encoded as *records*, which are simply a sequence of *fields*.

The alignment of a record is the maximum alignment of its fields.

Fields in a record are laid out sequentially, where each field gets the next suitably-aligned offset after the preceding field.
No effort is made to fill in "gaps" left by preceding fields.

Note: currently the size of a record is *not* rounded up to be a multiple of its alignment, so it is possible for one field to be laid out in the "tail padding" of the field before it.
This behavior should probably be changed, so that the fossilized layout better matches what C/C++ compilers tend to do.

### Variable-Size Types

Types where different instances may consume a different number of bytes may be encoded either *inline* or *indirectly*.

If a variable-size type `V` is being referred to by a pointer or optional (e.g., `V*` or `std::optional<V>`), then it will be encoded inline as the target address of that pointer/optional.

In all other contexts, including when a `V` is used as a field or a record, it will be encoded indirectly (conceptually, as if the field was actually a `V*`).
When a variable-size type is encoded indirectly, a null pointer should be interpreted as an empty instance of the type `V`.

#### Arrays

An array of `T` is encoded as a sequence of `T` values, separated by the *stride* of `T` (the size of `T` rounded up to the alignment of `T`).
The offset of the array is the offset of its first element.

The number of elements in the array is encoded as a 4-byte unsigned integer stored immediately *before* the offset of the array itself.

#### Strings

A string is encoded in the same way that an array of 8-bit bytes would be (including the count stored before the first element).
The only additional detail is that the serialized data *must* include an additional nul byte after the last element of the string.

The data of a string is assumed to be in UTF-8 encoding, but there is nothing about the format that validates or enforces this.

#### Dictionaries

A dictionary with keys of type `K` and values of type `V` is encoded in the same way as an array of `P`, where `P` is a two-element tuple of a `K` and a `V`.

There is currently no provision made for efficient lookup of elements of a fossilized dictionary.

#### Variants

A *variant* is a fossilized value that can describe its own layout.

The content of variant holding a value of type `T` is encoded exactly as a record with one field of type `T` would be, starting at the offset of the variant itself.

The four bytes immediately preceding a variant store a relative pointer to the fossilized layout for the type `T` of the content.

### Layouts

Every layout starts with a 4-byte unsigned integer that holds a tag representing the kind of layout (see `Slang::FossilizedValKind`).
The value of the tag determines what, if any, information appears after the tag.

In any place where a relative pointer to a layout is expected, a null pointer may be used to indicate that the relevant layout information is either unknown, or was elided from the fossilized data.

#### Pointer-Like Types

For pointers (`T*`) and optionals (`Optional<T>`), the tag is followed by a relative pointer to a layout for `T`.

#### Container Types

For arrays and dictionaries, the tag is followed by:

* A relative pointer to a layout for the element type

* A 4-byte unsigned integer holding the stride between elements

#### Record Types

For records, the tag is followed by:

* A 4-byte unsigned integer holding the number of fields, `N`

* `N` 8-byte values representing the fields, each comprising:

    * A relative pointer to the type of the field

    * A 4-byte unsigned integer holding the offset of that field within the record

The RIFF Support Code
=====================

There is code in `source/core/slang-riff.{h,cpp}` that implements abastractions for reading and writing RIFF-structured files.

The current RIFF implementation is trying to be "correct" for the RIFF format as used elsewhere (e.g., for `.wav` files), but it is unclear if this choice is actually helping us rather than hurting us.
It is likely that we will want to customize the format if we keep using (e.g., at the very least increase the minimum alignment of chunks).

RIFF is a simple chunk-based file format that is used by things like WAV files, and has inspired many similar container formats used in media/games.

The RIFF structures are currently being used for a few things:

* The top-level structure of serialized files for slang modules, "module libraries". This design choice is being utilized so that the compiler can navigate the relevant structures and extract the parts it needs (e.g., just the digest of a module, but not the AST or IR).

* Repro files are using a top-level RIFF container, but it is just to encapsulate a single blob of raw data (with internal offset-based pointers)

* The structure of the IR and `SourceLoc` serialization formats uses RIFF chunks for their top-level structure, but doesn't really make use of the ability to navigate them in memory or perform random access.

* The actual serialized AST format is currently a deep hierarchy of RIFF chunks.

* There is also code for a RIFF-based hierarchical virtual file-system format, and that format is being used for the serialized core module (seemingly just because it includes support for LZ4; the actual "file system" that gets serialized seems to only have a single file in it).

General-Purpose Hierarchical Data Serialization
===============================================

The code in `source/slang/slang-serialize.{h,cpp}` implements a framework for serialization that is intended to be lightweight for users to adopt, while also scaling to more complicated cases like our AST serialization.

In the simplest cases, all a programmer needs to know is that if they have declared a type like:

    struct MyThing
    {
        float f;
        List<OtherThing> others;
        SomeObject* obj;
    };

then they can add serialization support for their type by writing a function like:

    void serialize(Serializer const& serializer, MyThing& value)
    {
        SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
        serialize(serializer, value.f);
        serialize(serializer, value.others);
        serialize(serializer, value.obj);
    }

If the `OtherThing` and `SomeObject` types were already set up with their own serialization support, then that should be all that's needed.
Of course there's a lot more to it in once you get into the details and the difficult cases.
For now, looking at `source/slang/slang-serialize.h` is probably the best way to learn more about the approach.

One key goal of this serialization system is that it allows the serialized format to be swapped in and out without affecting the per-type `serialize` functions.
Currently there are only a small number of implementations.

RIFF Serialization
------------------

The files `slang-serialize-riff.{h,cpp}` provide an implementation of the general-purpose serialization framework that reads/writes RIFF files with a particular kind of structure, based on what had previously been hard-coded for use in serializing the AST to RIFF.

In practice this representation is kind of like an encoding of JSON as RIFF chunks, with leaf/data chunks for what would be leaf values in JSON, and container chunks for arrays and dictionaries (plus other aggregates that would translate into arrays or dictionaries in JSON).

Fossil Serialization
--------------------

The files `slang-serialize-fossil.{h,cpp}` provide an implementation of the generla-purpose serialization framwork that reads/writes the "fossil" format, which is described earlier in this document.

AST Serialization
=================

AST serialization is implementation as an application of the general-purpose framework described above.
There is an `ASTSerializer` type that expands on `Serializer` to include the additional context that is needed for handling AST-related types like `SourceLoc`, `Name`, and the `NodeBase` hierarchy.

The Old Serialization System
============================

The old serialization system has largely been removed, but some vestiges of it are still noticeable.

There was an older serialization system in place that made use of an extensive RTTI system that types had to be registered with, plus a set of boilerplate macros for interfacing with that system that were generated from the C++ declarations of the AST node types.
That system was also predicated on the idea that to serialize a user C++ type `Foo`, one would also hand-author a matching C++ type `SerialFooData`, and then write code to translate a `Foo` to/from a `SerialFooData` plus code to read/write a `SerialFooData` from the actual serialized data format.

The IR and `SourceLoc` serialization approaches are currently still heavily influenced by the old serialization system, and there are still vestigates of the RTTI infrastructure that was introduced to support it.
The hope is that as more subsystems are ported to use newer approaches to serialization, this code can all be eliminated.

The following sections are older text that describes some of the formats that have not yet been revisited.

IR Serialization
----------------

This mechanism is *much* simpler than generali serialization, because by design the IR types are very homogeneous in style. There are a few special cases, but in general an instruction consists of

* Its type
* A SourceLoc
* 0 or more operands.
* 0 or more children. 

Within the IR instructions are pointers to IRInst derived types. As previously discussed serializing pointers directly is generally not a good idea. To work around this the pointers are turned into 32 bit indices. Additionally we know that an instruction can belong to at most one other instruction. 

When serializing out special handling is made for child instructions - their indices are made to be a contiguous range of indices for all instructions that belong to each parent. The indices are ordered into the same order as the children are held in the parent. By using this mechanism it is not necessary to directly save off the indices that belong to a parent, only the range of indices. 

The actual serialization mechanism is similar to the generalized mechanism - referenced objects are saved off in order of their indices. What is different is that the encoding fixes the size of the Inst to `IRSerialData`. That this can hold up to two operands, if the instruction has more than two operands then one of the UInt32 is the operand count and the other is an offset to a list of operands. It probably makes sense to alter this in the future to stream the instructions payload directly. 

IR serialization allows a simple compression mechanism, that works because much of the IR serialized data is UInt32 data, that can use a variable byte encoding.

SourceLoc Serialization
-----------------------

SourceLoc serialization presents several problems. Firstly we have two distinct serialization mechanisms that need to use it - IR serialization and generalized serialization. That being the case it cannot be saved directly in either, even though it may be referenced by either. 

To keep things simple for now we build up SourceLoc information for both IR and general serialization via their writers adding their information into a SerialSourceLocWriter. Then we can save this information into a RIFF section, that can be loaded before either general or IR deserialization is used.  

When reading the SourceLoc information has to be located and deserialized before any AST or IR deserialization. The SourceLoc data can then be turned into a SerialSourceLocReader, which is then either set on the `SerialReaders` `SerialExtraObjects`. Or passed to the `IRSerialReader`.
