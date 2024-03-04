---
layout: user-guide
permalink: /user-guide/convenience-features
---

# Basic Convenience Features

This topic covers a series of nice-to-have language features in Slang. These features are not supported by HLSL but are introduced to Slang to simplify code development. Many of these features are added to Slang per request of our users. 

## Type Inference in Variable Definitions
Slang supports automatic variable type inference:
```csharp
var a = 1; // OK, `a` is an `int`.
var b = float3(0, 1, 2); // OK, `b` is a `float3`.
```
Automatic type inference require an initialization expression to present. Without an initial value, the compiler is not able to infer the type of the variable. The following code will result a compiler error:
```csharp
var a; // Error, cannot infer the type of `a`.
```

You may use the `var` keyword to define a variable in a modern syntax:
```csharp
var a : int = 1; // OK.
var b : int; // OK.
```

## Immutable Values
The `var` syntax and the traditional C-style variable definition introduces a _mutable_ variable whose value can be changed after its definition. If you wish to introduce an immutable or constant value, you may use the `let` keyword:
```rust
let a = 5; // OK, `a` is `int`.
let b : int = 5; // OK.
```
Attempting to change an immutable value will result in a compiler error:
```rust
let a = 5;
a = 6; // Error, `a` is immutable.
```


## Namespaces

You can use the `namespace` syntax to define symbols in a namespace:
```csharp
namespace ns
{
    int f();
}
```

Slang also supports the abbreviated syntax for defining nested namespaces:
```csharp
namespace ns1.ns2
{
    int f();
}
// equivalent to:
namespace ns1::ns2
{
    int f();
}
// equivalent to:
namespace ns1
{
    namespace ns2
    {
        int f();
    }
}
```

To access symbols defined in a namespace, you can use its qualified name with namespace prefixes:
```csharp
void test()
{
    ns1.ns2.f();
    ns1::ns2::f(); // equivalent syntax.
}
```

Symbols defined in the same namespace can access each other without a qualified name, this is true even if the referenced symbol is defined in a different file or module:
```csharp
namespace ns
{
    int f();
    int g() { f(); } // OK.
}
```

You can also use the `using` keyword to pull symbols defined in a different namespace to
the current scope, removing the requirement for using fully qualified names.
```cpp
namespace ns1.ns2
{
    int f();
}

using ns1.ns2;
// or:
using namespace ns1.ns2; // alternative syntax.

void test() { f(); } // OK.
```

## Member functions

Slang supports defining member functions in `struct`s. For example, it is allowed to write:

```hlsl
struct Foo
{
    int compute(int a, int b)
    {
        return a + b;
    }
}
```

You can use the `.` syntax to invoke member functions:

```hlsl
Foo foo;
int rs = foo.compute(1,2);
```

Slang also supports static member functions, For example:
```
struct Foo
{
    static int staticMethod(int a, int b)
    {
        return a + b;
    }
}
```

Static member functions are accessed the same way as other static members, via either the type name or an instance of the type:

```hlsl
int rs = Foo.staticMethod(a, b);
```

or

```hlsl
Foo foo;
...
int rs = foo.staticMethod(a,b);
```

### Mutability of member function

For GPU performance considerations, the `this` argument in a member function is immutable by default. If you modify the content in `this` argument, the modification will be discarded after the call and does not affect the input object. If you intend to define a member function that mutates the object, use `[mutating]` attribute on the member function as shown in the following example.

```hlsl
struct Foo
{
    int count;
    
    [mutating]
    void setCount(int x) { count = x; }

    void setCount2(int x) { count = x; }
}

void test()
{
    Foo f;
    f.setCount(1); // f.count is 1 after the call.
    f.setCount2(2); // f.count is still 1 after the call.
}
```

## Properties

Properties provide a convenient way to access values exposed by a type, where the logic behind accessing the value is defined in `getter` and `setter` function pairs. Slang's `property` feature is similar to C# and Swift. 
```csharp
struct MyType
{
    uint flag;

    property uint highBits
    {
        get { return flag >> 16; }
        set { flag = (flag & 0xFF) + (newValue << 16); }
    }
};
```

Or equivalently in a "modern" syntax:

```csharp
struct MyType
{
    uint flag;

    property highBits : uint
    {
        get { return flag >> 16; }
        set { flag = (flag & 0xFF) + (newValue << 16); }
    }
};
```

You may also use an explicit parameter for the setter method:
```csharp
property uint highBits
{
    set(uint x) { flag = (flag & 0xFF) + (x << 16);  }
}
```

> #### Note ####
> Slang currently does not support automatically synthesized `getter` and `setter` methods. For example,
> the following code is not supported:
> ```
> property uint highBits {get;set;} // Not supported yet.
> ```

## Initializers
> #### Note ####
> The syntax for defining initializers are subject to future change.


Slang supports defining initializers in `struct` types. You can write:
```csharp
struct MyType
{
    int myVal;
    __init(int a, int b)
    {
        myVal = a + b;
    }
}
```

You can use an initializer to construct a new instance by using the type name in a function call expression:
```csharp
MyType instance = MyType(1,2);  // instance.myVal is 3.
```

You may also use C++ style initializer list to invoke an initializer:
```csharp
MyType instance = {1, 2};
```

If an initializer does not define any parameters, it will be recognized as *default* initializer that will be automatically called at the definition of a variable:

```csharp
struct MyType
{
    int myVal;
    __init()
    {
        myVal = 10;
    }
};

int test()
{
    MyType test;
    return test.myVal; // returns 10.
}
```

## Operator Overloading

Slang allows defining operator overloads as global methods:
```csharp
struct MyType
{
    int val;
    __init(int x) { val = x; }
}

MyType operator+(MyType a, MyType b)
{
    return MyType(a.val + b.val);
}

int test()
{
    MyType rs = MyType(1) + MyType(2);
    return rs.val; // returns 3.
}
```
Slang currently supports overloading the following operators: `+`, `-`, `*`, `/`, `%`, `&`, `|`, `<`, `>`, `<=`, `>=`, `==`, `!=`, unary `-`, `~` and `!`. Please note that the `&&` and `||` operators are not supported.


## Subscript Operator

Slang allows overriding `operator[]` with `__subscript` syntax:
```csharp
struct MyType
{
    int val[12];
    __subscript(int x, int y) -> int
    {
        get { return val[x*3 + y]; }
        set { val[x*3+y] = newValue; }
    }
}
int test()
{
    MyType rs;
    rs[0, 0] = 1;
    rs[1, 0] = rs[0, 0] + 1
    return rs[1, 0]; // returns 2.
}
```

## `Optional<T>` type

Slang supports the `Optional<T>` type to represent a value that may not exist.
The dedicated `none` value can be used for any `Optional<T>` to represent no value.
`Optional<T>::value` property can be used to retrieve the value.

```csharp
struct MyType
{
    int val;
}

int useVal(Optional<MyType> p)
{
    if (p == none)        // Equivalent to `p.hasValue`
        return 0;
    return p.value.val;
}

int caller()
{
    MyType v;
    v.val = 0;
    useVal(v);  // OK to pass `MyType` to `Optional<MyType>`.
    useVal(none);  // OK to pass `none` to `Optional<MyType>`.
    return 0;
}
```

## `reinterpret<T>` operation

Sometimes it is useful to reinterpret the bits of one type as another type, for example:
```csharp
struct MyType
{
    int a;
    float2 b;
    uint c;
}

MyType myVal;
float4 myPackedVector = packMyTypeToFloat4(myVal);
```

The `packMyTypeToFloat4` function is usually implemented by bit casting each field in the source type and assign it into the corresponding field in the target type,
by calling `intAsFloat`, `floatAsInt` and using bit operations to shift things in the right place.
Instead of writing `packMyTypeToFloat4` function yourself, you can use Slang's builtin `reinterpret<T>` to do just that for you:
```
float4 myPackedVector = reinterpret<float4>(myVal);
```

`reinterpret` can pack any type into any other type as long as the target type is no smaller than the source type.

## Pointers

Slang supports pointers when generating code for SPIRV, C++ and CUDA targets. The syntax for pointers is similar to C, with the exception that operator `.` can also be used to dereference a member. Slang currently does not support the `->` operator.
For example:
```csharp
int test(MyType* pObj)
{
    MyType* pNext = pObj + 1;
    MyType* pNext = &pNext[1];
    return pNext.a + (*pNext).a + pNext[0].a;
}
```

Pointer types can also be specified using the generic syntax: `Ptr<MyType>` is equivalent to `MyType*`.

## `struct` inheritance (limited)

Slang supports a limited form of inheritance. A derived `struct` type has all the members defined in the base type it is inherited from:

```csharp
struct Base
{
    int a;
    void method() {}
} 

struct Derived : Base { int b; }

void test()
{
    Derived c;
    c.a = 1;  // OK, a is inherited from `Base`.
    c.b = 2;
    c.method(); // OK, `method` is inherited from `Base`.
}
```

A derived type can be implicitly casted to its base type:
```csharp
void acceptBase(Base b) { ... }
void test()
{
    Derived c;
    acceptBase(c); // OK, c is implicitly casted to `Base`.    
}
```

Slang supports controlling whether a type can be inherited from with `[sealed]` and `[open]` attributes on types.
If a base type is marked as `[sealed]`, then inheritance from the type is not allowed anywhere outside the same module (file) that is defining the base type. If a base type is marked as `[open]`, then inheritance is allowed regardless of the location of the derived type. By default, a type is `[sealed]` if no attributes are declared, which means the type can only be inherited by other types in the same module.

### Limitations

Please note that the support for inheritance is currently very limited. Common features that comes with inheritance, such as `virtual` functions and multiple inheritance are not supported by the Slang compiler. Implicit down-casting to the base type and use the result as a `mutable` argument in a function call is also not supported.

Extensions
--------------------
Slang allows defining additional methods for a type outside its initial definition. For example, suppose we already have a type defined:

```csharp
struct MyType
{
    int field;
    int get() { return field; }
}
```

You can extend `MyType` with new method members:
```csharp
extension MyType
{
    float getNewField() { return newField; }
}
```

All locations that sees the definition of the `extension` can access the new members:

```csharp
void test()
{
    MyType t;
    float val = t.getNewField();
}
```

This feature is similar to extensions in Swift and partial classes in C#.

> #### Note:
> You can only extend a type with additional methods. Extending with additiional data fields is not allowed.

Multi-level break
-------------------

Slang allows `break` statements with a label to jump into any ancestor control flow break points, and not just the immediate parent.
Example:
```
outer:
for (int i = 0; i < 5; i++)
{
    inner:
    for (int j = 0; j < 10; j++)
    {
        if (someCondition)
            break outer;
    }
}
```

Force inlining
-----------------
Most of the downstream shader compilers will inline all the function calls. However you can instruct Slang compiler to do the inlining
by using the `[ForceInline]` decoration:
```
[ForceInline]
int f(int x) { return x + 1; }
```


Special Scoping Syntax
-------------------
Slang supports three special scoping syntax to allow users to mix in custom decorators and content in the shader code. These constructs allow a rendering engine to define custom meta-data in the shader, or map engine-specific block syntax to a meaningful block that is understood by the compiler via proper `#define`s.

### `__ignored_block`
An ignored block will be parsed and ignored by the compiler:
```
__ignored_block
{
    arbitrary content in the source file,
    will be ignored by the compiler as if it is a comment.
    Can have nested {} here.
}
```

### `__transparent_block`
Symbols defined in a transparent block will be treated as if they are defined
in the parent scope:
```csharp
struct MyType
{
    __transparent_block
    {
        int myFunc() { return 0; }
    }
}
```
Is equivalent to:
```csharp
struct MyType
{
    int myFunc() { return 0; }
}
```

### `__file_decl`
Symbols defined in a `__file_decl` will be treated as if they are defined in
the global scope. However, symbols defined in different `__file_decl`s is not visible
to each other. For example:
```csharp
__file_decl
{
    void f1()
    {
    }
}
__file_decl
{
    void f2()
    {
        f1(); // error: f1 is not visible from here.
    }
}
```
