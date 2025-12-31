# Structures

A `struct` is a type consisting of an ordered sequence of members.

A struct member is declared in the struct body and is one of the following:
- Data member (*aka.* field); declared as a variable
- Member function; declared as a function
- Nested type; declared as type or type alias

A data member and a member function can be declared with the `static` keyword.

- The storage for a static data member is allocated from the global storage. A static member function may:
  - Access static data members of the struct.
  - Invoke other static member functions of the struct.
- The storage for a non-static data member is allocated as part of the struct. A non-static member function may:
  - Access both the static and the non-static data members.
  - Invoke both the static and the non-static member functions.

The non-static data members are allocated sequentially within the `struct` when a variable of this type is
allocated.

A nested type is a regular type enclosed within the scope of the outer `struct`.

A structure may conform to one or more [interface](04-types-interface.md) types.

A structure may be extended with a [type extension](04-types-extension.md).

> Remark: Structure inheriting from another structure is deprecated.


## Objects

An object is an *instance* of a `struct`. An instance consists of all non-static data members defined in a
`struct`. The data members may be initialized using an initializer list or a constructor. For details, see
[variable declarations](07-declarations.md).


## Non-static Member Functions

A non-static member function has a hidden parameter `this` that refers to an object. The hidden parameter
is used to reference the object data members and to invoke other non-static member functions.

In the function body, other members may be referenced using `this.`, although it is optional.

By default, only a read access to the object members is allowed by a member function. If write access is
required, the member function must be declared with the `[mutating]` attribute.

Non-static member functions cannot be accessed without an object.

> Remark: In C++ terminology, a member function is `const` by default. Attribute `[mutating]` makes it
> a non-`const` member function.


## Accessing Members and Nested Types

The static `struct` members and nested types are accessed using either `.` or `::`. Their semantics in this
context is identical.

Non-static object members are accessed using the member of object operator `.`.

### Example:

```hlsl
// struct type declaration
struct TestStruct
{
    // data member
    int a;

    // static data member, initial value 5
    static int b = 5;

    // static constant data member, initial value 6
    static const int c = 6;

     // nested type
    struct NestedStruct
    {
        static int c = 6;
        int d;
    }

    // member function with read-only access
    // to non-static data members
    int getA()
    {
        // also just plain "return a" would do
        return this.a;
    }

    // member function with read/write access
    // to non-static data members
    [mutating] int incrementAndReturnA()
    {
        // modification of data member
        // requires [mutating]
        a = a + 1;

        return a;
    }

    // static member function
    static int getB()
    {
        return b;
    }

    static int incrementAndReturnB()
    {
        // [mutating] not needed for
        // modifying static data member
        b = b + 1;

        return b;
    }
}

// instantiate an object of type TestStruct using defaults
TestStruct obj = { };

// instantiate an object of type NestedStruct
TestStruct::NestedStruct obj2 = { };

// access an object data member directly
obj.a = 42;

// access a static data member directly
int tmp0 = TestStruct::b + TestStruct::NestedStruct::c;

// invoke object member functions
int tmp1 = obj.getA();
int tmp2 = obj.incrementAndReturnA();

// invoke static members functions

// '.' can be used to resolve scope
int tmp3 = TestStruct.getB();

// '::' is equivalent to '.' for static member access
int tmp4 = TestStruct::incrementAndReturnB();
```

# Memory Layout

## Base Layout

The *base layout* for a structure type uses the following rules:

- The alignment of a structure is the maximum of 1, alignment of any member, and alignment of any parent type.
- The data is laid out in order of:
  - Parent types
  - Non-static data members
- Offset of the data items:
  - The offset of the first data item is 0
  - The offset of the *Nth* data item is the offset+size of the previous item rounded up to the alignment of
    the item
- The size of the struct is offset+size of the last item. That is, the struct is not tail-padded and rounded
  up to the alignment of the struct.

The following algorithm may be used:

1. Initialize variables `size` and `alignment` to zero and one, respectively
2. For each field `f` of the structure type:
   1. Update `alignment` to be the maximum of `alignment` and the alignment of `f`
   2. Set `size` to the smallest multiple of `alignment` not less than `size`
   3. Set the offset of field `f` to `size`
   4. Add the size of `f` to `size`

When this algorithm completes, `size` and `alignment` will be the size and alignment of the structure type.

> Remark: Most target platforms do not use the base layout directly, but it provides a baseline for defining
> other layouts. Any layout for a structure type must guarantee an alignment at least as large as the standard
> layout.

## C-Style Layout

The C-style layout of a structure type differs from the base layout in that the structure size is rounded up
to the structure alignment. This mirrors the layout rules used by typical C/C++ compilers.

## D3D Constant Buffer Layout

D3D constant buffer layout is similar to the base layout with two differences:

- The minimum alignment is 16.
- If a data member crosses a 16-byte boundary and its offset is not aligned by 16, the offset is rounded up to the
  next multiple of 16.
  - In HLSL, this is called an _improper straddle_.

This Type
---------

Within the body of a structure or interface declaration, the keyword `This` may be used to refer to the
enclosing type. Inside of a structure type declaration, `This` refers to the structure type itself.  Inside
of an interface declaration, `This` refers to the concrete type that is conforming to the interface (that is,
the type of `this`).
