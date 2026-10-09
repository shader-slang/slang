---
layout: user-guide
---

# The HLSL-Flavored Dialect

The Slang toolset supports an HLSL-flavored source-language dialect.
The `slangc` command selects this dialect for source files whose names end in `.hlsl` or `.fx`.
Pass `-lang hlsl` for another filename extension.

This appendix describes places where the HLSL-flavored dialect differs from the Slang language.

## Receiver Mutability

A non-`static` HLSL member function declared on a value type or interface has an implicit `this` parameter.
By default, that parameter uses the `inout` parameter-passing mode, so the member function can modify the receiver and those changes are visible to the caller:

```hlsl
struct Counter
{
    int value;

    void increment()
    {
        value++;
    }
};
```

Putting `const` after the parameter list changes the mode of `this` to `in` and makes the receiver immutable in the member-function body:

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

Trailing `const` is the HLSL spelling for a non-mutating receiver.
It is rejected outside the HLSL dialect; Slang source uses `[nonmutating]` instead.

The trailing `const` controls the parameter-passing mode of `this`.
It does not make an otherwise identical member function a distinct overload.
