Slang Documentation
===================

This directory contains documentation for the Slang system.
Some of the documentation is intended for users of the language and compiler, while other documentation is intended for developers contributing to the project.

* historical

Language
--------

The Slang language [User's Guide](https://shader-slang.github.io/slang/user-guide/) provides an introduction to the Slang language and its major features, as well as the compilation and reflection API.

Work is in progress on a more detailed [reference](language/reference) for the Slang language, but that documentation is not ready for general consumption.

Compiler
--------

The command-line Slang compiler, `slangc`, has its own separate [user's guide](compiler/command-line/guide) and [reference](compiler/command-line/reference).

Slang Standard Library
----------------------

A [reference](stdlib/reference) for all of the built-in types, functions, etc. supported by Slang is generated from the content of the Slang standard library.
The quality of this documentation is currently lacking.

Supported GPU Features
----------------------

Documentation of individual GPU hardware and API features supported by Slang is organized under [`gpu-feature`](gpu-feature/).
The documents there typically describe how a feature is exposed in the Slang standard library, how those constructs map to each of the targets that supports a given feature, and any caveats that users should be aware of.

Supported Targets
-----------------

When compiling for multiple targets, the [Feature Stability Table](target/feature-stability.md) gives an overview of which targets support which GPU features.

Detailed guides for individual targets are collected under [`target/`](target/).

Proposals
---------

Proposals for changes to the Slang language, standard library, API, or tools are organized under [`proposals/`](proposals/).
Proposals are used to iterate on the design of new features before they are finalized.

For Contributors
----------------

For contributors to the Slang project, the information under the [`contributor/`](contributor/) directory may be relevant to building the project, interacting with our CI process, etc.

In addition, the documents under [`compiler/implementation/`](compiler/implementation/) are intended to provide developers working on the project with explanations of how certain subsystems of the compiler have been designed and implemented.
Proposals for implementation improvements and changes may be checked in under [`compiler/implementation/proposals`](compiler/implementation/proposals); these documents do *not* reflect the current status of the codebase, but may guide future development.

The `gfx` GPU Library
---------------------

The Slang system provides a (completely optional) GPU API abstraction layer, called `gfx`.
The `gfx` library has its own [documentation](gfx), separate from the Slang language and compiler.