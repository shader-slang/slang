SP #012: Introduce a `#language` Directive
=========================================

Status: Design Review
Implementation: -
Author: Theresa Foley <tfoley@nvidia.com>
Reviewer: -

Introduction
------------

We propose to add a preprocessor directive, `#language` to allow a Slang source file to specify the version of the Slang language that is used in that file.
The basic form is something like:

    // MyModule.slang

    #language slang 2024.1

    ...

In the above example, the programmer has declared that their file is written using version 2024.1 of the Slang language and standard library.
Slang toolsets with versions below 2024.1 will refuse to compile this file, since it might use features that they do not support.
Slang toolsets with versions greater than 2024.1 *may* refuse to compile the file, if they have removed support for that language version.

Putting a language version directly in source files allows the Slang language and standard library to evolve (including in ways that remove existing constructs) without breaking existing code.
A single release of the Slang toolchain may support a range of language versions, with different supported language/library constructs, and select the correct features to enable/disable based on the `#language` directive.

When a release of the Slang toolchain doesn't support the language version requested by a programmer's code, the diagnostics produced can clearly state the problem and possible solutions, such as switching to a different toolchain version, or migrating code.

Background
----------

Like many programming languages, Slang experiences a tension between the desire for rapid innovation/evolution and stability.
One of the benefits that users of Slang have so far enjoyed has been the rapid pace of innovation in the language and its standard library.
However, as developers start to have larger bodies of Slang code, they may become concerned that changes to the language could break existing code.
There is no magical way to keep innovating while also keeping the language static.

This proposal is an attempt to find a middle road between the extremes of unconstrained evolution and ongoing stasis.

Related Work
------------

### GLSL ###

The most obvious precedent for the feature proposed here is the [`#version` directive](https://www.khronos.org/opengl/wiki/Core_Language_(GLSL)#Version) in GLSL, which can be used to specify a version of the GLSL language being used and, optionally, a profile name:

    #version 460 core

There are some key lessons from the history of GLSL that are worth paying attention to:

* When OpenGL ES was introduced, the OpenGL ES Shading Language also used an identical `#version` directive, but the meaning of a given version number was different between GLSL and GLSL ES (that is, different language features/capabilities were implied by the same `#version`, depending on whether one was compiling with a GLSL or GLSL ES compiler). The use of the optional profile name is highly encouraged when there might be differences in capability not encoded by just the version number.

* Initially, the version numbers for OpenGL and GLSL were not aligned. For example, OpenGL 2.0 used GLSL 1.10 by default. This led to confusion for developers, who needed to keep track of what API version coresponded to what language version. The version numbers for OpenGL and GLSL became aligned starting with OpenGL 3.3 and GLSL 3.30.

* A common, but minor, gotcha for developers is that the GLSL `#version` directive can only be preceded by trivia (whitespace and comments) and, importantly, cannot be preceded by any other preprocessor directives. This limitation has created problems when applications want to, e.g., prepend a sequence of `#define`s to an existing shader that starts with a `#version`.

When a GLSL file does not include a `#version` directive, it implicitly indicates version 1.10.
This is a safe o

### Racket ###

While it is a very different sort of language than Slang, it is valuable to make note of the Racket programming language's [`#lang` notation](https://docs.racket-lang.org/guide/Module_Syntax.html#%28part._hash-lang%29).

A `#lang` line like:

    #lang scribble/base

indicates that the rest of the file should be read using the language implementation in the module named `scribble/base`.
Different modules can implement vastly different languages (e.g., `scribble/base` is a LaTeX-like document-preparation language).

This construct in Racket is extremely flexible, allowing for entirely different languages (e.g., custom DSLs) to be processed by the Racket toolchain, but it could also trivially be used to support things like versioning:

    #lang slang 2024.1

While we do not necessarily need or want the same degree of flexibility for the Slang language itself, it is worth noting that the Slang project, and its toolchain, is in the situation of supporting multiple distinct languages/dialects (Slang, a GLSL-flavored dialect, and an HLSL-flavored dialect), and has extensive logic for inferring the right language to use on a per-file basis from things like file extensions.

The Racket toolchain treats files without a `#lang` line as using the ordinary Racket language, and provide whatever language and library features were current at the time that toolchain was built.

### Other Languages ###

Most other language implementations do not embed versioning information in source files themselves, and instead make the language version be something that is passed in via compiler options:

* gcc and clang use the `-std` option to select both a language and a version of that language: e.g., `c99` vs `c++14`.

* dxc uses the `-HV` option to specify the version of the HLSL language to use, typically named by a year: e.g., `2016` or `2021`.

* Rust developers typically use configuration files for the Cargo package manager, which allows specifying the Rust language and compiler version to use with syntax like `rust-version = "1.56"`.

When language versions are not specified via these options, most toolchains select a default, but that default may change between releases of the toolchain (e.g., recent versions of clang will use C++17 by default, even if older releases of the toolchain defaulted to C++14 or lower).


Proposed Approach
-----------------

### Language and Compiler Versions ###

We will differentiate between two kinds of versions, which will have aligned numbering:

* The *language version* determines what language features (keywords, attributes, etc.) and standard library declarations (types, functions, etc.) are available, and what their semantic guarantees are.

* The *compiler version* or *toolset version* refers to the version of a release of the actual Slang tools such as `slangc`, `slang.dll`, etc.

This proposal doesn't intend to dictate the format used for version numbers, since that is tied into the release process for the Slang toolset.
We expect that version numbers will start with a year, so that, e.g., `2025.0`  would be the first release in the year 2025.

A given version of the Slang toolset (e.g, `2024.10`) should always support the matching language verson.

If this proposal is accepted, we expect releases of the Slang toolset to support a *range* of language versions, ideally covering a full year or more of backwards compatibility.
This proposal does not seek to make any guarantees about the level of backwards compatiblity, leaving that the Slang project team to determine in collaboration with users.

### `#language` Directives ###

Any file that is being processed as Slang code (as opposed to HLSL or GLSL) may have a `#language` directive and, if it does, that directive determines the language version required by that file.

A `#language` directive specifying Slang version 2025.7 would look like:

    #language slang 2025.7

If the toolset being used supports the requested version, it will process that file with only the capabilities of that language version.
If the requested version is out of the range supported by the toolset (either too old or too new) compilation will fail with an appropriate diagnostic.

If a file has *no* version directive, then the toolset will process that file as if it requested the version corresponding to the toolset release (e.g., a 2025.1 toolset release would compile such a file using language version 2025.1).

Detailed Explanation
--------------------

* A `#language` directive must only be preceded by trivia (whitespace and comments).

* The directive must always be of the form `#language slang <version>`; it is not valid to only list a version number without the language name `slang`.

* The version number follows the syntactic form of a floating-point literal, and might be lexed as one for simplicity, but internally each of the components of the version should be treated as an integer. For example, a version `2026.10` is *not* equivalent to `2026.1`, and is a higher version number than `2026.9`.

* We are not proposing strict adherence to [Semantic Versioning](https://semver.org/) at this time.

* The `#language` directive will not support specifying a version in the form `MAJOR.MINOR.PATCH` - only `MAJOR` and `MAJOR.MINOR` are allowed. The assumption is that patch releases should always be backwards-compatible, and a given toolset can always safely use the highest patch number that matches the requested version.

* If the version number is given as just the form `MAJOR` instead of `MAJOR.MINOR`, then the toolset will use the highest language version it supports that has that major version. That is, `#language slang 2026` is not an alias for `2026.0`, but instead acts as a kind of wildcard, matching any `2026.*` version.

* The directive is allowed to be given as just `#language slang`, in which case the toolset will use the highest supported language version, as it would when the directive is absent.

* If an explicit compiler option was used to select a language other than Slang (e.g., via `-lang hlsl` to explicitly select HLSL), then the `#language` directive described in this proposal will result in an error being diagnosed.

* When the toolset version and the language version are not the same (e.g., a `2026.1` compiler is applied to `2025.3` code), the request language version *only* affects what language and library constructs are supported, and their semantics. Things like performance optimizations, supported targets, etc. are still determined by the toolset.

Alternatives Considered
-----------------------

### Compiler Options ###

The main alternative here is to allow the language version to be specified via compiler options.
The exising `-lang` option for `slangc` could be extended to include a language version: e.g., `slang2025.1`.

This proposal is motivated by extensive experience with the pain points that arise when semantically-significant options, flags, and capabilities required by a project are encoded not in its source code, but only in its build scripts or other configuration files.
Anybody who has been handed a single `.hlsl` file and asked to simply compile it (e.g., to reproduce a bug or performance issue) likely knows the litany of questions that need to be answered before that file is usable: what is the entry point name? What stage? What shader model?

The addition of the `[shader(...)]` attribute to HLSL represents a significant improvement to quality-of-life for developers, in part because it encodes the answers to two of the above question (the entry point name and stage) into the source code itself.
We believe this example should be followed, to enable as much information as possible that is relevant to compilation to be embedded in the source itself.

### Leaving Out the Language Name ###

It is tempting to support a directive with *just* the language version, e.g. something like:

    #version 2025.3

We strongly believe that including an explicit language name is valuable for future-proofing, especially given the lessons from GLSL and GLSL ES mentioned earlier.

The requirement of an explicit language name has the following benefits:

* It avoids any possible confusion with the GLSL `#version` directive, which serves a similar purpose but has its own incompatible version numbering. If we supported using just a bare version number, then there would be a strong push to use the name `#version` for our directive instead of `#language`.

* It leaves the syntax open to future extensions, such that the Slang toolset could recognize other languages/dialects (e.g., supporting a `#language hlsl 2021` directive). Further down that particular rabbit-hole would be support for Racket-style DSLs.

* It could potentially encourage other languages with overlapping communities (such as HLSL, WGSL, etc.) to adopt a matching/compatible directives, thus increasing the capability for tooling to automatically recognize and work with multiple languages.

### Naming ###

The name for this directive could easily lead to a lot of bikeshedding, with major alternatives being:

* We could use `#version` to match GLSL, *especially* if we decide to change the approach and allow the language name `slang` to be elided. However, as discussed above, we think that this is likely to cause confusion between Slang's directive and the GLSL directive, especially when the Slang toolchain supports both languages.

* We could hew closely to the precedent of Racket and use `#lang` instead of `#language`. There is no strong reason to pursue actual *compatibility* between the two (i.e., so that Slang code can be fed to the Racket toolchain, or vice versa), so the benefits of using the exact same spelling are minimal. This proposal favors being more humane and spelling things out fully over using contractions.

Future Directions
-----------------

Some of the more likely future directions include:

* Allow the version part of `#language` to support a patch number, so that code that requires a particular bug fix to compile can be annotated with that fact and thus fail compilation cleanly when processed with a buggy toolset version.

* Extend support for versioning to modules other than the standard library. The `#language` directive effectively introduces a versioning scheme for the Slang standard library and allows a user-defined module to specify the version(s) it is compatible with. This system could be extended to allow all modules (including user-defined modules) to define their own version numbers, and for `import` declarations to identify the version of a dependency that is required.

  * Taking such a direction should only be done with a careful survey of existing approaches to versioning used by package managers for popular languages, to ensure that we do not overlook important features that make management of dependency versions practical.

Some less likely or less practical directions include:

* Add other supported language names. Supporting something like `#language hlsl 2021` would allow for our HLSL-flavored dialect to recognize different versions of HLSL without resorting to command-line flags.

* If we did the above, we would probably need to consider allowing a "safe" subset of preprocessor directives to appear before a `#language` line - most importantly, `#if` and `#ifdef`, so that one could conditionally compile a `#language` line depending on whether HLSL-flavored code is being processed with the Slang toolchain (which would support `#language`) or other compilers like dxc (which might not).

* In the far-flung future, we could consider the Racket-like ability to have a `#language` directive support looking up a language implementation module matching the language name, and then parsing the rest of the file as that language, for DSL support, etc.

