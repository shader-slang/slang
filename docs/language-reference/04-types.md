# Types

The types in the Slang language consists of the following categories:

* [4a - Fundamental Types](04-types-fundamental.md)
* [4b - Vector and Matrix Types](04-types-vector-and-matrix.md)
* [4c - Structures](04-types-struct.md)
* [4d - Interfaces](04-types-interface.md)
* [4e - Extensions](04-types-extension.md)
* [4f - Array types](04-types-array.md)
* [4g - Opaque types](04-types-opaque.md)

Types in Slang do not generally have identical memory layouts in different targets. The provided guarantees
are discussed with each type. Any details of layout not specified may depend on the target language, target
device, compiler options, and the context in which a type is used.


## Known and Unknown Size

Every type has either a known or an unknown size. Types with unknown size generally stem from unknown-length
arrays:
* An unknown-length array type has an unknown size.
* The size of a structure type is unknown if it has a non-static data member with unknown size.

The use of types with unknown size is restricted as follows:
* A type with unknown size cannot be used as the element type of an array
* A type with unknown size can only be used as the last field of a structure type
* A type with unknown size cannot be used as a generic argument to specialize a user-defined type, function,
  etc. Specific built-in generic types/functions may support unknown-size types, and this will be documented
  on the specific type/function.
* A type with unknown size cannot be instantiated as a variable.

> Remark: Unknown size is different to unspecified or target-specified size. [Opaque types](04-types-opaque.md)
> have target-specified sizes, sizes of [structures](04-types-struct.md) and [arrays](04-types-array.md) are subject to
> target-specific alignment rules, and certain [4a - Fundamental Types](04-types-fundamental.md) such as
> `bool` have target-specified size. Unspecified-sized types are not subject to the restrictions of
> unknown-sized types.
