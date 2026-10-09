---
layout: user-guide
permalink: /user-guide/hlsl-dialect
---

# The HLSL-Flavored Dialect

The Slang toolset supports an HLSL-flavored source-language dialect.
This appendix documents how that dialect differs from both the Slang language and full HLSL.

## Selecting the HLSL Dialect

The `slangc` command selects the HLSL dialect for source files whose names end in `.hlsl` or `.fx`.
To compile a file as HLSL when it has another filename extension, use the `-lang hlsl` option to explicitly request the HLSL dialect.
For the complete rules that assign source files to translation units, see [Source Files and Translation Units](08-compiling.md#source-files-and-translation-units).

## Member Function Receivers

A non-static HLSL member function has an implicit `this` parameter for the receiver of the member-function call.
By default, the `this` parameter uses the `inout` parameter-passing mode, so the member function can modify the receiver:

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

Write `const` after the parameter list to make the `this` parameter read-only in the member-function body:

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

A copyable receiver then uses the `in` parameter-passing mode.
A non-copyable receiver instead uses a read-only borrow because it cannot be copied into the member function.

The mutating default applies whenever the HLSL dialect is selected; there is no compatibility option that restores a read-only default.

The parameter-passing mode of `this` does not distinguish overloads.
Declarations with otherwise identical signatures belong to the same redeclaration family even when that mode differs.
The compiler then compares their effective `this` parameter modes: equivalent spellings are compatible, while different modes produce a receiver-mismatch diagnostic.
For example, these declarations conflict instead of defining two overloads:

```hlsl
struct Counter
{
    int getValue();
    int getValue() const; // Error: redeclaration with a different receiver mode.
};
```
