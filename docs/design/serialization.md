Serialization
=============

Slang's infrastructure for serialization is currently in flux, so there exist a mixture of different subsystems, using a mixture of different techniques.

This document is curently minimal, and primarily serves to provide a replacement for an older draft that no longer reflects the state of the codebase.

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
The current implementation only includes a RIFF-based output format matching what had previously been in use for the AST, but the infrastructure should also be able to support a JSON implementation, or binary formats.

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
