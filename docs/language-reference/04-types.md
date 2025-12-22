> Note: This document is a work in progress. It is both incomplete and, in many cases, inaccurate.

Types
=====

This section defines the kinds of types supported by Slang.

Types in Slang do not necessarily prescribe a single _layout_ in memory.
The discussion of each type will specify any guarantees about layout it provides; any details of layout not specified here may depend on the target platform, compiler options, and context in which a type is used.

* [4a - Fundamental Types](04-types-fundamental.md)
* [4b - Vector and Matrix Types](04-types-vector.md)
* [4c - Structures and Interfaces](04-types-struct-and-interface.md)
* [4d - Array types](04-types-array.md)
* [4e - Opaque types](04-types-opaque.md)


Known and Unknown Size
----------------------

Every type has either known or unknown size.
Types with unknown size arise in a few ways:

* An unknown-size array type has unknown size

* A structure type has unknown size if any field type has unknown size

The use of types with unknown size is restricted as follows:

* A type with unknown size cannot be used as the element type of an array

* A type with unknown size can only be used as the last field of a structure type

* A type with unknown size cannot be used as a generic argument to specialize a user-defined type, function, etc. Specific built-in generic types/functions may support unknown-size types, and this will be documented on the specific type/function.
