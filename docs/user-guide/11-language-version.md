# Language Version

Like many programming languages, Slang experiences a tension between the desire for rapid innovation/evolution and stability. One of the benefits that users of Slang have so far enjoyed has been the rapid pace of innovation in the language and its standard library. However, as developers start to have larger bodies of Slang code, they may become concerned that changes to the language could break existing code. There is no magical way to keep innovating while also keeping the language static.

Slang supports using the `#language` preprocessor directive, as well as the `-std` compiler option (`CompilerOptionName::LanguageVersion`) to specify the language version that a source file is written against. The source file will then be parsed and checked by the compiler using the rules from the specified language version.

Users are advised to provide a `#language` directive as the first non-whitespace line in their source file, such as:

```
#language slang 2026
```

The following version strings are allowed:
- `latest`: use the latest language version supported by the current compiler.
- `legacy`: use the legacy Slang language.
- `2018`: equivalent to `legacy`.
- `2025`: Slang language version 2025.
- `2026`: Slang language version 2026.

If no `#language` line exists and no version is specified via compiler options, the default setting is `legacy`.

## The Legacy Slang Language

When the language version is set to `legacy`, the compiler behavior will be consistent with the Slang language as in 2018-2023. Specifically:

- All declarations have `public` visibility.
- A `module` declaration is not required at the start of each module.

## Slang 2025

Slang language version 2025 brings these changes on top of the legacy language:

- All declarations have `internal` visibility.
- A `module` declaration is required at the start of each module.
- Modifier `volatile` has been deprecated in Slang. See GitHub issue
  [#10614](https://github.com/shader-slang/slang/issues/10614) for details. The modifier is still accepted
  in GLSL source code for compatibility reasons.

## Slang 2026

Slang language version 2026 brings these changes on top of Slang 2025:

- Comma expression is removed when it is used inside a parenthesis. The expression `(a, b)` no longer evaluates to have `b`'s type as in C/C++. Instead, `(a,b)` means `makeTuple(a,b)` and returns a tuple consisting of `a` and `b`. See [SP#027](https://github.com/shader-slang/spec/blob/main/proposals/027-tuple-syntax.md) for details.
- Users must explicitly opt in to enable dynamic dispatch with the `dyn` keyword. More rigorous validations are enabled to make sure dynamic dispatch is not triggered accidentally. See [SP#024](https://github.com/shader-slang/spec/blob/main/proposals/024-any-dyn-types.md) for details.
- Modifier `volatile` has been removed from Slang. See GitHub issue
  [#10614](https://github.com/shader-slang/slang/issues/10614) for details. The modifier is still accepted
  in GLSL source code for compatibility reasons.
