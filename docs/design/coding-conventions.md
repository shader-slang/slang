Slang Project Coding Conventions
================================

Principles
----------

This document attempts to establish conventions to be used in the Slang codebase.
We have two goals for this convention.

The first goal is to make the code look relatively consistent so that it is easy to navigate and understand for contributors.
Having varying styles across different modules, files, functions, or lines of code makes the overall design and intention of the codebase harder to follow.

The second goal is to minimize the scope complexity of diffs when multiple maintainers work together on the codebase.
In the absence of an enforced style, developers tend to "clean up" code they encoutner to match their personal preferences, and in so doing create additional diffs that increase the chances of merge conflicts and pain down the line.

Because the Slang codebase has passed through many hands and evolved without a pre-existing convention, these two goals can come into conflict.
We encourage developers to err on the side of leaving well enough alone (favoring the second goal).
Don't rewrite or refactor code to match these conventions unless you were already going to have to touch all of those lines of code anyway.

Note that external code that is incorporated into the project is excluded from all of these conventions.

Languages
---------

### C++

Most code in the Slang project is implemented in C++.
We currently assume support for some C++11 idioms, but have explicitly avoided adding dependencies on later versions.

As a general rule, be skeptical of "modern C++" ideas unless they are clearly better to simpler alternatives.
We are not quite in the realm of "Orthodox C++", but some of the same guidelines apply:

* Don't use exceptions for non-fatal errors (and even then support a build flag to opt out of exceptions)
* Don't the built-in C++ RTTI system (home-grown is okay)
* Don't use the C++ variants of C headers (e.g., `<cstdio>` instead of `<stdio.h>`)
* Don't use the STL containers
* Don't use iostreams

The compiler implementation does not follow some of these guidelines at present; that should not be taken as an excuse to further the proliferation of stuff like `dynamic_cast`.
Do as we say, not as we do.

Some relatively recent C++ features that are okay to use:

* Rvalue references for "move semantics," but only if you are implementing performance-critical containers or other code where this really matters.

* `auto` on local variables, if the expected type is clear in context

* Lambdas are allowed, but think carefully about whether just declaring a subroutine would also work.

* Using `>>` to close multiple levels of templates, instead of `> >` (but did you really need all those templates?)

* `nullptr`

* `enum class`

* Range-based `for` loops

* `override`

* Default member initializers in `class`/`struct` bodies

Templates are suitable in cases where they improve clarity and type safety.
As a general rule, it is best when templated code is kept minimal, and forwards to a non-templated function that does the real work, to avoid code bloat.

Any use of template metaprogramming would need to prove itself exceptionally useful to pay for the increase in cognitive complexity.
We don't want to be in the business of maintaining "clever" code.

As a general rule, `const` should be used sparingly and only with things that are logically "value types."
If you find yourself having to `const`-qualify a lot of member function in type that you expect to be used as a heap-allocated object, then something has probably gone wrong.

As a general rule, default to making the implementation of a type `public`, and only encapsulate state or operations with `private` when you find that there are complex semantics or invariants that can't be provided without a heavier hand.

### Slang

The Slang project codebase also includes `.slang` files implementing the Slang standard library, as well as various test cases and examples.
The conventions described here are thus the "official" recommendations for how users should format Slang code.

To the extent possible, we will try to apply the same basic conventions to both C++ and Slang.
In places where we decide that the two languages merit different rules, we will point it out.

Files and Includes
------------------

### File Names

All files and directories that are added to codebase should have names that contain only ASCII lower-case letters, digits, dots (`.`) and dashes (`-`).
Operating systems still vary greatly in their handling of case sensitivity for file names, and non-ASCII code points are handled with even less consistency; sticking to a restricted subset of ASCII helps avoids some messy interactions between case-insensitive file systems and case-sensitive source-control systems like Git.
As with all these conventions, files from external projects are exempted from these restrictions.

### Naming of Source and Header Files

In general the C++ codebase should be organized around logical features/modules/subsystem, each of which has a single `.h` file and zero or more `.cpp` files to implement it.

If there is a single `.cpp` file, its name should match the header: e.g., `parser.h` and `parser.cpp`.

If there is more than one `.cpp` file, their names should start with the header name: e.g., `parser.h` and `parser-decls.cpp` and `parser-exprs.cpp`.
If there are declarations that need to be shared by the `.cpp` files, but shouldn't appear in the public interface, then can go in a `*-impl.h` header (e.g., `parser-impl.h`).

Use best judgement when deciding what counts as a "feature." One class per file is almost always overkill, but the codebase currently leans too far in the other direction, with some oversized source files.

### Headers

Every header file should have an include guard.
Within the implementation we can use `#pragma once`, but exported API headers (`slang.h`) should use traditional `#ifdef` style guards (and they should be consumable as both C and C++).

A header should include or forward-declare everything it needs in order to compile.
It is *not* up to the programmer who `#include`s a header to sort out the dependencies.

Avoid umbrella or "catch-all" headers.

### Source Files

Every source file should start by including the header for its feature/module, before any other includes (this helps ensure that the header correctly includes its dependencies).

Functions that are only needed within that one source file can be marked `static`, but we should avoid using the same name for functions in different files (in order to support lumped/unified builds).

### Includes

In general, includes should be grouped as follows:

* First, the correspodning feature/module header, if we are in a source file
* Next, any `<>`-enlosed includes for system/OS headers
* Next, any `""`-enclosed includes for external/third-part code that is stored in the project repository
* Finally, any includes for other features in the project

Within each group, includes should be sorted alphabetically.
If this breaks because of ordering issues for system/OS/third-party headers (e.g., `<Windows.h>` must be included before `<GL/GL.h>`), then ideally those includes should be mediated by a Slang-project-internal header that features can include.

Namespaces
----------

Favor fewer namespaces when possible.
Small programs may not need any.

All library code that a Slang user might link against should go in the `Slang` namespace for now, to avoid any possibility of clashes in a static linking scenario.
The public C API is obviously an exception to this.


Indenting, Spacing, and Braces
------------------------------

Indent by four spaces. Don't use tabs except in files that require them (e.g., `Makefile`s).

Matching opening/closing curly braces should be indented to the same column, with an exception for empty braces (`{}`).
An opening curly brace should always be on a new line.

```c++
void someFunc(int a)
{
    doThings(a);
}

void emptyFunc()
{}

struct Helper
{
    void help()
    {
        emptyFunc();
    }
}
```

Extremely short "accessor" style functions can violate these rules if the whole definition fits on a line.

```c++
struct Vec1
{
    float x;

    float getX() { return x; }
}
```

There is no hard limit on line length, but if you are going past 80-100 columns quite often, maybe think about breaking lines.

When you decide to break lines for a paramter or argument list, always break after the opening `(`, and put each argument/parameter on its own line:

```c++
float bigFunc(
    int     a,
    float   b,
    void*   c)
{
    ...
}

float gVar = bigFunc(
    0,
    1.0f,
    data);
```

You can vertically align succesive lines of code to emphasize common structure, but this is not required:

```c++
case A_AND_B: doA(); doB(); break;
case JUST_A:  doA();        break;
case JUST_B:         doB(); break;
```

Don't put space between a control-flow keyword and a following `(`.
Examples of how to format the major C++ control-flow constructs follow:

```c++
void example()
{
    for(int ii = 0; ii < N; ++ii)
    {
        if(ii == 0)
        {
        }
        else
        {
        }
    }

    int x = 0;
    while(x < 100)
    {
        x++;
    }

    int mode = 0;
    do
    {
        switch(mode)
        {
        case 0:
            x /= 2;
            /* fallthrough */
        case 1:
            x--;
            mode = 3;

        case 2:
        default:
            x++;
            mode--;
            break;
        }
    } while(x > 0);
}
```

Don't put space between a unary operator and its operand.
Always put space between an assignment (including compound assignment) operator and its operands.
For other binary operators, use your best judgement.

If you have to line break a binary expression, put the line break after the operator for an assignment, and before the operator before all others.

Intializer lists for constructors get some special-case formatting rules.
If everything fits on one line, then great:

```c++
SomeClass::SomeClass()
    : a(0), b(1), c(2)
{}
```

Otherwise, put the line break *before* the comma:

```c++
SomeClass::SomeClass()
    : a(0)
    , b(1)
    , c(2)
{}
```

When working with `static const` arrays, put the outer `{}` for the array on their own line, and then put each element on its own line and aim for vertical alignment.

```c++
struct Mapping
{
    char const* key;
    char const* val;
} kMapping[] =
{
    { "a", "aardvark" },
    { "b", "bat" },
};
```

A trailing comma should always be used for array initializer lists like this, as well as for `enum` tags.

When writing multi-line macros, the backslash escapes should align vertically.
For consistentcy, a trailing `\` can be used, followed by a comment to mark the end of the definition:

```c++
#define FOREACH_ANIMAL(X)   \
    X(Aardvark)             \
    X(Bat)                  \
    /* end */
```

Naming
------

### Casing

Types should in general use `UpperCamelCase`. This includes `struct`s, `class`es, `enum`s and `typedef`s.

Values should in general use `lowerCamelCase`. This includes functions, methods, local variables, global variables, parameters, fields, etc.

Macros should in general use `SCREAMING_SNAKE_CASE`.
It is important to prefix all macros (e.g., with `SLANG_`) to avoid collisions, since `namespace`s don't affect macros).

In names using camel case, acronyms and initialisms should appear eniterly in either upper or lower case (e.g., `D3DThing d3dThing`) and not be capitalized as if they were ordinary words (e.g., `D3dThing d3dThing`).
Note that this also applies to uses of "ID" as an abbreviation for "identifier" (e.g., use `nodeID` instead of `nodeId`).

### Prefixes

Prefixes based on types (e.g., `p` for pointers) should never be used.

Global variables should have a `g` prefix, e.g. `gCounter`.
Non-`const` `static` class members can have an `s` prefix if that suits your fancy.
Of course, both of these should be avoided, so this shouldn't come up often.

Constant data (in the sense of `static const`) should have a `k` prefix.

In contexts where "information hiding" is relevant/important, such as when a type has both `public` and `private` members, or just has certain operations/fields that are considered "implementation details" that most clients should not be using, and `_` prefix on function and field members is allowed (but not required).

In function parameter lists, an `in`, `out`, or `io` prefix can be added to a parameter name to indicate whether a pointer/reference/buffer is intended to be used for input, output, or both input and output.
For example:

```c++
void copyData(void* outBuffer, void const* inBuffer, size_t size);

Result lookupThing(Key k, Thing& outThing);

void maybeAppendExtraNames(std::vector<Name>& ioNames);
```

Public C APIs will prefix all symbol names while following the casing convention (e.g. `SlangModule`, `slangLoadModule`, etc.).

### Enums

C-style `enum` should use the following convention:

```c++
enum Color
{
    Color_Red,
    Color_Green,
    Color_Blue,

    ColorCount,
};
```

When using `enum class`, drop the type name as prefix, but retain the `UpperCamelCase` tag names:

```c++
enum class Color
{
    Red,
    Green,
    Blue,

    Count,
};
```

When defining a set of flags, separate the type definition from the `enum`:

```c++
typedef unsigned int Axes;
enum
{
    Axes_None = 0,

    Axis_X = 1 << 0,
    Axis_Y = 1 << 1,
    Axis_Z = 1 << 2,

    Axes_All = kAxis_X | kAxis_Y | kAxis_Z,
};
```

Note that the type name reflects the plural case, while the cases that represent individual bits are named with a singular prefix.

In public APIs, all `enum`s should use the style of separating the type defintion from the `enum`, and all cases should use `SCREAMING_SNAKE_CASE`:

```c++
typedef unsigned int SlangAxes;
enum
{
    SLANG_AXES_NONE = 0,

    SLANG_AXIS_X = 1 << 0,
    SLANG_AXIS_Y = 1 << 1,
    SLANG_AXIS_Z = 1 << 2,

    SLANG_AXES_ALL = SLANG_AXIS_X | SLANG_AXIS_Y | SLANG_AXIS_Z,
};
```

### General

Names should default to the English language and US spellings, to match the dominant conventiosn of contemporary open-source projects.

Function names should either be named with action verbs (`get`, `set`, `create`, `emit`, `parse`, etc.) or read as questions (`isEnabled`, `shouldEmit`, etc.).

Whenever possible, compiler concepts should be named using the most widely-understood term available: e.g., we use `Token` over `Lexeme`, and `Lexer` over `Scanner` simply because they appear to be the more common names.

Avoid abbreviations and initialisms unless they are already widely established across the codebase; a longer name may be cumbersome to write in the moment, but the code will probably be read many more times than it is written, so clarity should be preferred.
An important exception to this is common compiler concepts or techqniues which may have laboriously long names: e.g., Static Single Assignment (SSA), Sparse Conditional Copy Propagation (SCCP), etc.

One gotcha particular to compiler front-ends is that almost every synonym for "type" has some kind of established technical meaning; most notably the term "kind" has a precise meaning that is relevant in our domain.
It is common practice in C and C++ to define tagged union types with a selector field called a "type" or "kind," which does not usually match this technical definition.
If a developer wants to avoid confusion, they are encouraged to use the term "flavor" instead of "type" or "kind" since this term (while a bit silly) is less commonly used in the literature.

Comments and Documentation
--------------------------

You probably know the drill: comments are good, but an out-of-date comment can be worse than no comment at all.
Try to write comments that explain the "why" of your code more than the "what."
When implementing a textbook algorithm or technique, it may help to imagine giving the reviewer of your code a brief tutorial on the topic.

In cases where comments would benefit from formatting, use Markdown syntax.
We do not currently have a setup for extracting documentation from comments, but if we add one we will ensure that it works with Markdown.

When writing comments, please be aware that your words could be read by many people, from a variety of cultures and backgrounds.
Default to a plain-spoken and professional tone and avoid using slang, idiom, profanity, etc.
