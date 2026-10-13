---
layout: user-guide
permalink: /user-guide/hlsl-dialect
---

# The HLSL-Flavored Dialect

The Slang toolset supports an HLSL-flavored source-language dialect.
This appendix documents how that dialect differs from both the Slang language and full HLSL.

## Selecting the HLSL Dialect

The `slangc` command selects the HLSL dialect for source files whose names end in `.hlsl` or `.fx`.
To compile a file as HLSL, even if it has another file extension,
use the `-lang hlsl` option to explicitly request the HLSL dialect.
For the complete rules that assign source files to translation units,
see [Source Files and Translation Units](08-compiling.md#source-files-and-translation-units).

## Member Function Receivers

Non-static methods on HLSL value types and non-static HLSL interface requirements
have an implicit `this` parameter for the receiver of a call.
In the HLSL dialect,
this parameter uses the `inout` parameter-passing mode by default,
so the member function can modify its receiver:

```hlsl
struct Counter
{
    int value;

    void increment()
    {
        ++value;
    }
};
```

This mutating default applies to both value-type methods and interface requirements
whenever the HLSL dialect is selected.
There is no compatibility option that restores the read-only default used by the Slang dialect.

The HLSL dialect accepts a trailing `const`
on both value-type methods and interface requirements.
It appears immediately after the parameter list and before any `where` clause.
Trailing `const` selects a read-only `this` parameter,
which prevents the member function from modifying its receiver.
A copyable receiver uses the `in` parameter-passing mode,
while a non-copyable receiver uses the borrowing `constref` mode:

```hlsl
struct Counter
{
    int value;

    int getValue() const
    {
        return value;
    }
};
```

Receiver mode is not part of overload identity.
Consequently, `f()` and `f() const` are conflicting redeclarations rather than two overloads:

```hlsl
struct Counter
{
    int getValue();
    int getValue() const; // Error: redeclaration with a different receiver mode.
};
```

Outside the HLSL dialect, trailing `const` is diagnosed as an error.
For a method whose receiver mode is configurable,
the compiler still treats the receiver as read-only while recovering from that error.
Methods declared on a Slang `class` instead retain their fixed `in` receiver mode
and reject trailing `const` along with the other receiver-mode specifiers.
See [Receiver mutability](03-convenience-features.md#receiver-mutability).
