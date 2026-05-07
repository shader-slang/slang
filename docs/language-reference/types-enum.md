# Enumerations

## Syntax {#syntax}

Enumeration declaration:
> *`enum-decl`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[*`modifier-list`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'enum'`** [**`'class'`**] [*`enum-identifier`*] <br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[**`':'`** *`tag-type`*]<br>
> **`'{'`** [*`enum-case-decl`* (**`','`** *`enum-case-decl`*)* ] **`'}'`**

Enumeration case declaration:
> *`enum-case-decl`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`enum-const-identifier`* [**`'='`** *`expr`*]

> 📝 **Remark:** The parser currently accepts generic parameters for an enumeration declaration,
> but the parameters are not usable. See GitHub issue
> [#10078](https://github.com/shader-slang/slang/issues/10078) for details.

### Parameters {#parameters}

- *`modifier-list`* is an optional list of modifiers. (TODO: link)
- **`'class'`** is a compatibility feature that allows the same enumeration declarations to be shared between
  C/C++ and Slang.
- *`enum-identifier`* is the identifier for the declared enumeration type. If omitted, the enumeration
  is *anonymous*.
- *`tag-type`* specifies the underlying type of the enumeration. If omitted, the default is `int`.
- *`enum-const-identifier`* is an identifier for an enumerator, i.e., an enumerated constant.
- *`expr`* is a [link-time constant](expressions-evaluation-classes.md), specifying the
  numeric value for the enumerator. If omitted, the value is the previous enumerator's value incremented
  by 1. If the value for the first enumerator is unspecified, the default is 0.

### Description {#description}

An enumeration is a scalar type that holds a value and may contain named constants, called
*enumerators*. An enumeration has an underlying type that serves as both its storage type and the type of its
enumerators.

The underlying type of an enumeration must be a [Boolean](types-fundamental.md) or an
[integer](types-fundamental.md) type. If no underlying type is specified, the default is `int`.

Enumerations can be either *scoped* or *unscoped*. The named constants of a scoped enumeration are accessed
within the enumeration namespace using the `EnumType.ENUM_CONST` form. If the enumeration is *unscoped*,
the named constants are defined in the same namespace as the enumeration type. The enumerators of an unscoped
enumeration can also be accessed using the scoped form.

Named enumerations are scoped by default; anonymous enumerations are always unscoped. The `slangc`
command-line option `-unscoped-enum` changes the default for named enumerations to unscoped. The
[\[UnscopedEnum\]](../../../core-module-reference/attributes/unscopedenum-08.html) attribute in the
modifier list explicitly declares an unscoped enumeration, while the **`'class'`** keyword explicitly
declares a scoped enumeration. It is an error to apply the \[UnscopedEnum\] attribute to an enum class
declaration, or to declare an anonymous enum class.

Multiple enumerators may share the same numeric value.

An enumeration may be extended using the [extension](types-extension.md#enum) syntax. An extension can be used
to add member functions, constructors, interface conformances, and similar features to the enumeration.

> 📝 **Remark:** Scoped enumerations are generally recommended to avoid namespace pollution.


## Examples

```hlsl
enum TestEnum
{
    Zero,                    // value 0
    One,                     // value 1
    AnotherOne = One,        // value 1
    Three = One + One + One, // value 3
    Max = 2147483647,
}

[UnscopedEnum]
enum MyUnscopedEnum
{
    SOME_CONSTANT = 123,
    ANOTHER_CONSTANT = 234,
}

RWStructuredBuffer<TestEnum> output1;
RWStructuredBuffer<MyUnscopedEnum> output2;

[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    output1[0] = TestEnum.Zero;       // 0
    output1[1] = TestEnum.One;        // 1
    output1[2] = TestEnum.AnotherOne; // 1
    output1[3] = TestEnum.Three;      // 3
    output1[4] = TestEnum.Max;        // 2147483647

    // The enumerators of an unscoped enum can be
    // used directly...
    output2[0] = SOME_CONSTANT;       // 123

    // ... or with the enumeration type prefix
    output2[1] = MyUnscopedEnum.ANOTHER_CONSTANT; // 234
}
```
