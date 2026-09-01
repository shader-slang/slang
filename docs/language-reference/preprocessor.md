> Note: This document is a work in progress. It is both incomplete and, in many cases, inaccurate.

# Preprocessor

Slang supports a C-style preprocessor with the following directives:

* `#include`
* `#define`
* `#undef`
* `#if`, `#ifdef`, `#ifndef`
* `#else`, `#elif`
* `#endif`
* `#error`
* `#warning`
* `#line`
* `#pragma`
* `#language`, `#lang`
* `#version`

## Language Directive (Slang)

> *`LanguageDirective`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`'#lang'`** | **`'#language'`**)<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[*`SourceLanguage`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`SourceLanguageVersion`*<br>
>
> *`SourceLanguage`* = **`'slang'`**
>
> *`SourceLanguageVersion`* = **`<[[:alnum:]]+>`**

## Version Directive (GLSL)

> *`VersionDirective`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'#version'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`GlslLanguageVersion`*<br>
>
> *`GlslLanguageVersion`* = **`<[[:digit:]]+>`**

> Note: This section is not yet complete.
