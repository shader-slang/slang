# Basic Convenience Features

This topic covers a series of nice-to-have language features in Slang. These features are not supported by HLSL but are introduced to Slang to simplify code development. Many of these features are added to Slang per request of our users. 

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

Properties provide a convienient way to access values exposed by a type, where the logic behind accessing the value is defined in `getter` and `setter` function pairs. Slang's `property` feature is similar to C# and Swift. 
```C#
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

```C#
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
```C#
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
```C#
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
```C#
MyType instance = MyType(1,2);  // instance.myVal is 3.
```

You may also use C++ style initializer list to invoke an initializer:
```C#
MyType instance = {1, 2};
```

If an initializer does not define any parameters, it will be recognized as *default* initializer that will be automatically called at the definition of a variable:

```C#
struct MyType
{
    int myVal;
    __init(i)
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
```C#
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

Slang currently supports overloading the following operators: `+`, `-`, `*`, `/`, `%`, `&`, `&&`, `|`, `||`, `<`, `>`, `<=`, `>=`, `==`, `!=`, unary `-`, `~` and `!`.

## `struct` inheritance (limited)

Slang supports a limited form of inheritance. A derived `struct` type has all the members defined in the base type it is inherited from:

```C#
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
```C#
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
