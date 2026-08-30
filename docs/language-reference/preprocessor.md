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

`#language` selects the Slang language and the Slang language version associated with that source unit.
The language version is a per-source-unit property even when multiple Slang source files belong to one translation unit.
See [Language Versioning](../user-guide/11-language-version.md) for the supported version names and their compatibility rules.

## Version Directive (GLSL)

> *`VersionDirective`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'#version'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`GlslLanguageVersion`*<br>
>
> *`GlslLanguageVersion`* = **`<[[:digit:]]+>`**

`#version` selects GLSL and declares the GLSL version used by that source unit.

All primary source units in one translation unit must use one source language.
The source language explicitly requested through the compilation API or `-lang` takes precedence over the language inferred from file-name extensions, and source directives are expected to agree with that selection.
For backward compatibility, a conflicting `#language` or `#version` currently produces a warning and overrides the request-level selection before parsing begins.
Conflicting source directives within one translation unit are an error.

The deprecated `-allow-glsl` option is equivalent to explicitly requesting GLSL for every translation unit in a compilation request.
It has no independent effect on preprocessing, parsing, semantic checking, or code generation after that request-level normalization.

> Note: This section is not yet complete.
