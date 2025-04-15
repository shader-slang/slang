SP #013: Introduce a `#feature` Directive
=========================================

Status: Design Review

Implementation: -

Author: Theresa Foley <tfoley@nvidia.com>

Reviewer: -

Introduction
------------

We propose to introduce a `#feature` directive, which can be used to explicitly opt in to not-yet-stabilized features:

    #language slang 2025
    #feature quantumTextures

In the above hypothetical example, a developer has chosen to code against the Slang 2025 language version (as introduced in SP#012), which does not support a hypothetical feature `quantumTextures`, because it is not yet stabilized.
By using the directive `#feature quantumTextures`, the developer opts in to support for this feature, accepting that their code may break as the design and implementation of that feature evolves through experimentation.


Background
----------

One of Slang's strengths is the rapid pace at which language ideas and innovations become available, but this strength can also be a source of concerns for developers who need a stable language definition that they can rely on for production software development.

SP#012 aims to address one part of the challenge, with a preprocessor directive in source code to specify a *language version* that a particular file is authored against.
In order for language versions to be a useful tool, the set of features associated with a given version must not change and, even more vitally, the interface and semantics of those features must not change.

This proposal aims to address a different challenge: how can some developers access features that are not yet stable, without *all* developers being exposed to that instability?


Related Work
------------

### Rust ###

The Rust [Unstable Book](https://doc.rust-lang.org/unstable-book/index.html) shows how a Rust developer can opt in to unstable features by their names:

    #![feature(coroutines, coroutine_trait, stmt_expr_attributes)]

    ...

This syntax uses a Rust "inner attribute" (`#![...]` syntax), which applies to the surrounding scope (in this case the module being compiled).

Unstable features in Rust are given names (e.g., `coroutines` above), and code in a module only has access to the features it explicitly opts into.

Note that the `#![feature(...)]` attribute in Rust is documented as being used for "unstable or experimental compiler features".
Once features are stabilized, they are on-by-default in a given Rust language version.

### GLSL ###

GLSL has an `#extension` directive, which allows a source file to explicitly opt-in to extensions to the GLSL language and library by name, e.g.:

    #extension GL_ARB_arrays_of_arrays : require

The above example indicates that the shader source requires whatever language and library features are added by the `GL_ARB_arrays_of_arrays` extension.

In general, *published* GLSL extensions are expected to be stable; a developer opting in to use of an extension does not expect the definition of the extension to change.
Many GLSL extensions are used in production codebases, even when stability is a requirement.

Sometimes an unpublished GLSL extension might be exposed through compilers even when it is still in an experiemental or unstable state; it is expected that developers will typically know whether an extension they are using has been stabilized or not.
There is no indication in the source code, however, of whether a given extension represents a stable or unstable feature; the same `#extension` directive is used for both cases.

### Python ###

Python supports an idiomatic form of the `import` construct, which are called [future statements](https://docs.python.org/3/library/__future__.html).
For example:

    from __future__ import generators

These future statements allow a program to opt in to new language constructs or semantics that are not yet on-by-default.

The intention of future statements in Python is *not* to support experimental or unstable features.
Instead, they are used to allow the language designers to introduce new features that could break existing code (e.g., by adding a new reserved word) in a staged fashion.

When a new language feature (with a possibility for breaking existing code) is defined, implemented *and stabilized*, it is initially off-by-default to avoid breaking existing code, but can be explicited opted into via a future statement.
Then, after a specific Python version, that language feature will be changed to be on-by-default--ideally after developers have had enough time to adapt their code to the new semantics.

A future statement continues to work without error even when the feature it enables is already on-by-default in the current Python version.

Given that future statements only apply to fully *stabilized* language features, they are akin to the `#language` directive proposed in SP#012.

Proposed Approach
-----------------

The basic idea is simple: the compiler will process directives of the form:

    #feature <feature-name>

A `#feature` directive will enable the experimental feature with the matching name, if that is possible.

Because newer language versions might affect which features are on or off by default (or indeed might *remove* certain feature), a `#language` directive, if any, must precede any `#feature` directives.

Because a feature being enabled or disabled might affect how the rest of a file is parsed, or even preprocessed, `#feature` directives must precede all other code in a file (except for a possible `#language` directive).

Detailed Explanation
--------------------

### Feature Names ###

When experimental development of a new feature begins, via the Slang Proposal process, it should be given a *feature name*: an identifier that will distinguish it from all other features (even features that have been previously stabilized).

The template for proposals should be updated to include an entry 
for the feature name associated with a proposal in the header.

### Feature Status ###

A Slang compiler will need to maintain a mapping from feature names to the *status* of that feature, for the purposes of compilation of a given source file.
The status will be one of:

* Unknown (for features the compiler release does not know about)
* Unstable and disabled
* Unstable but enabled
* Stable (and therefore enabled)
* Deprecated
* Removed

A feature is inaccessible unless its status is "unstable but enabled," "stable," or "deprecated."

The compiler may try to diagnose attempts to access a feature that is inaccessible *if and only if* the code in question would be rejected with an error without that feature.
Put another way: if code can compile without error without the inaccessible feature, the compiler should do just that.

### Interaction With Language Versions ###

The language version selected by a programmer (e.g., via the `#language` directive proposed in SP#012) should determine for each feature known to the compiler whether its initial status is "unstable but disabled," "stable," "deprecated," or "removed."

### Feature Directive ###

A `#feature` directive always takes the form:

    #feature <feature-name>

Feature directives must appear at the start of a source file, and can only be preceded by trivia (whitespace and comments) and an optional `#language` directive.

The `<feature-name>` must be a single identifier.

When the preprocessor encounters a `#feature` directive, it will look up the current status of the feature with the given name (which will be determined by the language version in effect), and act accordingly:

* If the feature is unstabled and disabled, its status will be changed to unstable but enabled.

* If the feature is unknown or removed, an appropriate error diagnostic will be emitted

* Otherise, an appropriate warning diagnostic will be emitted

Alternatives Considered
-----------------------

### Compiler Options ###

The main alternative, as with SP#012, would be to include compiler options (e.g., command-line flags) to enable experimental features.

Our rationale for why we prefer to embed this information in the source file, rather than pass it via options, is the same as for SP#012, so we won't restate it here.

### Different Compiler Releases ###

The Slang project could produce distinct releases of the Slang compiler and toolchain with support for experimental features included (and enabled by default) vs. excluded (with no way to enable them).

This kind of all-or-nothing approach is undesirable because it is both too restrictive and not restrictive enough.

It is too restrictive in that developers who have been using a "stable" version of the Slang toolchain need to change which release(s) they rely on in order to even *try out* an experimental feature; the resulting barrier to entry could push developers away from features that might otherwise benefit them.

It is not restrictive enough in that as soon as a developer wants to access *any* experimental feature they end up using a compiler build that supports *all* experimental features.
This puts the onus on a developer to make sure that they don't accidentally start relying on unstable features they didn't mean to.

Resolving either of these problems seemingly requires a more granular approach to enabling and disabling features than all-or-nothing... which is precisely what the current proposal aims to provide.

### Naming ###

The name `#feature` is not ideal:

* It is just a simple noun, and does not make it clear that the given feature is being *enabled* or made *required* by the directive. In contrast, the GLSL `#extension` expects an explicit `: require`.

* It does not make it clear that the feature(s) that the directive applies to are *unstable* or *experimental*, and that this directive's purpose is unlike the superficially-similar GLSL `#extension` directive in this way.

Various alternative syntaxes could be experimented with, in order to identify what resonates best with developers:

    // add `enable` or `require` along
    //  the lines of GLSL `#extension`?
    //
    #feature XYZ : enable
    #feature XYZ : require

    // rename the directive be more fluent?
    //
    #enable feature XYZ
    #require feature XYZ

    // include an explicit word to note the
    // experimental or unstable nature of the feature?
    //
    #enableExperimentalFeature XYZ
    #enable experimental feature XYZ
