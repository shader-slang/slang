Serialization
=============

Slang has a collection of serialization components. This document will be used to discuss serialization around IR/AST and modules as it currently exists. A separate document will describe the future serialization plans.

All of the serialization aspects here focus on binary serialization. 

The major components are

* IR Serialization
* AST/Generalized Serialization
* SourceLoc Serialization
* Riff container
* C++ Extractor

Generalized Binary Serialization
================================

Generalized serializtion is the mechanism used to save 'arbitray' C++ structures. It is currently used for serializing the AST. Although not necessary, generalized serialization is typically helped out by the `C++ extractor`, which can rudamentarily parse C++ source, and extract class-like types and their fields. The extraction then produces header files that contain macros that can then be used to drive serialization. 

It's worth discussing briefly what the philosphy is behind the generalized serialization system. To talk about this design it is worth talking a little about serialization in general and the issues involved. Lets say we have a collection of C++ class instances that contain fields. Some of those fields might be pointers. Others of the fields might be a templated container type like a Dictionary<K,V>. We want to take all of these instances, write them to a file, such that when we read the file back we will have the equivalent objects with equivalent relationships. 

We could imagine a mechanism that saved off each instance, by writing off the address of the object, and then the in memory representation for all the instances that can be reached. When reading back the objects would be at different locations in memory. If we knew where the pointers were, we could use a map of old pointers to the new instances and fix them up. Problems with this simple mechanism occur because...

* If we try to read back on a different machine, with a different pointer size, the object layout will be incompatible
* If we try to read back on the same machine where the source is compiled by a different compiler, the object layout might be incompatible (say bool or enum are different size)
* Endianess might be different
* Knowing where all the pointers are and what they point to and therefore what to serialize is far from simple. 
* The alignment of types might be different across different processors and different compilers 

The implementation makes a distinction between the 'native' types, the regular C++ in memory types and 'serial' types. Each serializable C++ type has an associated 'serial' type - with the distinction that it can be written out and (with perhaps some other data) read back in to recreate the C++ type. The serial type can be a C++ type, but is such it can be written and read from disk and still represent the same data. 

The approach taken in Slang is to have each 'native' type (ie the C++ type) that is being serialized have a serializable 'dual' type. The serial type can be an explicit C++ type, or it might implicit (ie not have a C++ type) and calculated at Slang startup. 

The important point here is that the Serial type must writable on one target/process and readable correctly on another. 

The easy cases are types that have an alignment and representation that will work over all targets. These would be most built in types - integrals 8,16,32 and float32. Note that int64 and double are *not* so trivial, because on some targets that require 8 byte alignment - so they must be specially defined to have 8 byte alignment. 

Another odd case is bool - it has been on some compilers 32 bits, and on others 8 bits. Thus we need to potentially convert.

For this and other types it is therefore necessary to have function that can convert to and from the serialized dual representation.

## AST serialization

The AST is currently serialized via the Generalized serialization. 

The AST node types are generally types derived from the NodeBase. The C++ extractor is used to associate an ASTNodeType with every NodeBase type, such that casting is fast and simple and we have a simple integer to uniquely identify those types. The extractor also performs another task of associating with the type name all of the fields held in just that type. The definition of the fields is stored in an 'x macro' which is in the slang-ast-generated-macro.h file, for example

## Generalized Field Conversion

For types that contain fields, it would be somewhat laborious to have to write all of the conversion functions by hand. To avoid this we use the macro output of the C++ extractor to automatically generate the appropriate functions. 

Take DeclRefExpr from the AST hierarchy - the extractor produces a macro something like...

```
#define SLANG_FIELDS_ASTNode_DeclRefExpr(_x_, _param_)\
    _x_(scope, (RefPtr<Scope>), _param_)\
    _x_(declRef, (DeclRef<Decl>), _param_)\
    _x_(name, (Name*), _param_)
``` 

DeclRefExpr derives from Expr and this might hold other fields and so forth. 

The macros can generate the appropriate conversion functions *if* we have the conversion functions for the field types. Field type conversions can be specified via a special macro that describes how the conversion to and from the type work. To make the association between the native and serial type, as well as provide the functions to convert, we use the template

```
template <typename T>
struct SerialTypeInfo;
```
and specialize it for each native type. The specialization holds

* SerialType - The type that will be used to represent the native type
* NativeType - The native type
* SerialAlignment - A value that holds what kind of alignment the SerialType needs to be serializable (it may be different from SLANG_ALIGN_OF(SerialType)!)
* toSerial - A function that with the help of SerialWriter convert the NativeType into the SerialType
* toNative - A function that with the help of SerialReader convert the SerialType into the NativeType

It is useful to have a structure that can hold the type information, so it can be stored. That is achieved with

```
template <typename T>
struct SerialGetType;
```

This template can be specialized for a specific native types - but all it holds is just a function getType, which returns a `SerialType*`, which just holds the information held in the SerialTypeInfo template, but additionally including the size of the SerialType.

So we need to define a specialized SerialTypeInfo for each type that can be a field in a NodeBase/RefObject derived type. We don't need to define anything explicitly for the NodeBase derived types, as we will just generate the layout from the fields. How do we know the fields? We just used the macros generated from the C++ extractor.

So first a few things to observe...

1) Some types don't need any conversion to be serializable - int8_t, or float the bits can just be written out and read in (1)
2) Some types need a conversion but it's very simple - for example an enum without explicit size, being written as an explicit size
3) Some types can be written out but would not be directly readable or usable with different targets/processors, so need converting
4) Some types require complex conversions that require programmer code - like Dictionary/List

For types that need no conversion (1), we can just use the template SerialIdentityTypeInfo

```
template <>
struct SerialTypeInfo<SomeType> : public SerialIdentityTypeInfo<SomeType> {};
```

This specialization means that SomeType can be written out and read in across targets/compilers without problems.

For (2) we have another template that will do the conversion for us

```
template <typename NATIVE_T, typename SERIAL_T>
struct SerialConvertTypeInfo;
```

That we can use as above, and specify the native and serial types.

For (3) there are a few scenarios. For any field in a serial type we must store in the serialized type such that the representation will work across all processors/compilers. So one problematic type is `bool`. It's not specified how it's laid out in memory - and some compiles have stored it as a word. Most recently it's been stored as a byte. To make sure bool is ok for serialization therefore we store as a uint8_t.

Another example would be double. It's 64 bits, but on some arches/compilers it's SLANG_ALIGN_OF is 4 and on others it's 8. On some architectures a non aligned read will lead to a fault, on others it might be very slow. To work around this problem therefore we have to ensure double has the alignment that will work across all targets - and that alignment is 8. In that specific case that issue is handled via SerialBasicTypeInfo, which makes the SerialAlignment the sizeof the type.

For (4) there are a few things to say. First a type can always implement a custom version of how to do a conversion by specializing `SerialTypeInfo`. But there remains another nagging issue - types which allocate/use other memory that changes at runtime. Clearly we cannot define 'any size of memory' in a fixed SerialType defined in a specialization of SerialTypeInfo. The mechanism to work around this is to allow arbitrary arrays to be stored, that can be accessed via an SerialIndex. This will be discussed more once we discuss a little more about the file system, and SerialIndex. 

## Generalized Serialization Format

The serialization format used is 'stream-like' with each 'object' stored in order. Each object is given an index starting from 1. 0 is used to be in effect nullptr. The stream looks like

```
SerialInfo::Entry (for index 1)
Payload for type in entry

SerialInfo::Entry (for index 2)
Payload for type in entry

... 
... 
```

That when writing we have an array that maps each index to a pointer to the associated header. We also have a map that maps native pointers to their indices. The Payload *is* the SerialType for thing saved. The payload directly follows the Entry data. Each object in this list can only be a few types of things

* NodeBase derived type
* RefObject derived type
* String
* Array

The actual Entry followed by the payloads are allocated and stored when writing in a MemoryArena. When we want to write into a stream, we can just iterate over each entry in order and write it out.

You may have spotted a problem here - that some Entry types can be stored without alignment (for example a string - which stores the length VarInt encoded followed by the characters). Others require an alignment - for example an NodeBase derived type that contains a int64_t will *require* 8 byte alignment. That as a feature of the serialization format we want to be able to just map the data into memory, and be able to access all the SerialType as is on the CPU. For that to work we *require* that the payload for each entry has the right alignment for the associated SerialType.

To achieve this we store in the Entry it's alignment requirement *AND* the next entries alignment. With this when we read, as we as stepping through the entries we can find where the next Entry starts. Because the payload comes directly after the Entry - the Entrys size must be a modulo of the largest alignment the payload can have.

For the code that does the conversion between native and serial types it uses either the SerialWriter or SerialReader. This provides the mechanism to turn a pointer into a serializable `SerialIndex` and vice versa. There are some special functions for turning string like types to and forth.

The final mechanism is that of 'Arrays'. An array allows reading or writing a chunk of data associated with a `SerialIndex`. The chunk of data *must* hold data that is serializable. If the array holds pointers - then the serialized array must hold an array of `SerialIndex` values that represent those pointers. When reading back in `SerialIndex` is converted back to a pointer.

Arrays are the escape hatch that allows for more complex types to serialize. Dictionaries for example are saved as a serial type that is two SerialIndices one to a keys array and one to a values array.

Note that writing has two phases, serializing out into an SerialWriter, and then secondly writing out to a stream. 

## Object/Reference Types

When talking about Object/Reference types this means types that can be referenced natively as pointers. Currently that means NodeBase and some RefObject derived types. 

The SerialTypeInfo mechanism is generally for *fields* of object types. That for derived types we use the C++ extractors field list to work out the native fields offsets and types. With this we can then calculate the layout for NodeBase types such that they follow the requirements for serialization - such as alignment and so forth.

This information is held in the SerialClasses, which for a given TypeKind/SubType gives a SerialClassInfo, that specifies fields for just that type. 

## Reading

Due to the care in writing reading is relatively simple. We can just take the contents of the file and put in memory, as long as in memory it has an alignment of at least MAX_ALIGNMENT. Then we can build up an entries table by stepping through the data and writing the pointer.

The toNative functions take an SerialReader - this allows the implementation to ask for pointers and arrays from other parts of the serialized data. It also allows for types to be lazily reconstructed if necessary.

Lazy reconstruction may be useful in the future to partially reconstruct a sub part of the serialized data. In the current implementation, lazy evaluation is used on Strings. The m_objects array holds all of the recreated native 'objects'. Since the objects can be derived from different base classes the associated Entry will describe what it really is.

For the String type, we initially store the object pointer as null. If a string is requested from that index, we see if the object pointer is null, if it is we have to construct the StringRepresentation that will be used. An extra wrinkle is that we allow accessing of a serialized String as a Name or a string or a UnownedSubString. Fortunately a Name just holds a string, and a Name remains in scope as long as it's NamePool does which is passed in.

## Identifying Types


IR Serialization
================

SourceLoc Serialization
=======================

Riff Container
==============

C++ Extractor
=============

The C++ Extractor is a tool `slang-cpp-extractor` that can be used to example C++ files to extract class definitions and associated fields. It can then optionally output two files. These files contain in the form of macros information about each class as well as reflected fields. These generated files can then be used to implement serialization without having to explicitly specify fields in C++ source code.




