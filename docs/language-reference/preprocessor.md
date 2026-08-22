> Note: This document is a work in progress. It is both incomplete and, in many cases, inaccurate.

# Preprocessor

Slang supports a C-style preprocessor with the following directives:

- `#include`
- `#define`
- `#undef`
- `#if`, `#ifdef`, `#ifndef`
- `#else`, `#elif`
- `#endif`
- `#error`
- `#warning`
- `#line`
- `#pragma`
- `#language`, `#lang`
- `#version`

## Language Directive (Slang)

> _`LanguageDirective`_ =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`'#lang'`** | **`'#language'`**)<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[_`SourceLanguage`_]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;_`SourceLanguageVersion`_<br>
>
> _`SourceLanguage`_ = **`'slang'`**
>
> _`SourceLanguageVersion`_ = **`<[[:alnum:]]+>`**

## Version Directive (GLSL)

> _`VersionDirective`_ =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'#version'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;_`GlslLanguageVersion`_<br>
>
> _`GlslLanguageVersion`_ = **`<[[:digit:]]+>`**

> Note: This section is not yet complete.
