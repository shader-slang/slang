# Organizing the Implementation by Concern

The source hierarchy should help a contributor find the code responsible for a particular task and understand how that task fits into the compiler.
We organize code around _concerns_: coherent responsibilities such as parsing a program, representing its IR, or emitting code for a target.
A concern provides services that we can describe without first listing the C++ types that implement them.

Our intended organization gives each concern a directory and an architecture description beside its implementation.
For example, the parser lives in `source/slang/parser/`, with its architecture described in `source/slang/parser/parser.md`.
The directory may contain several implementation files, a single header and implementation pair, or child concerns when a further decomposition becomes useful.
The amount of code is less important than whether the responsibility has enough substance to benefit from being understood and developed separately.

## Architecture Descriptions

Each concern has a primary Markdown document named after the concern.
The document explains the architecture at the scale of that concern, with links to detailed design documents and child descriptions rather than copies of their explanations.
Its main topics are:

- **Services and responsibilities.** What the concern does for the rest of the implementation, and which neighboring responsibilities belong elsewhere.
- **Concepts.** The important terms needed to understand those services, with definitions for concepts introduced by the concern and links to definitions owned elsewhere.
- **Dependencies.** The concerns and shared infrastructure used by the implementation, with an explanation of the service each dependency supplies.
- **Decomposition.** The child concerns and their primary documents, or the main operations and file groups when there are no child concerns.
- **Ownership notes.** Where the implementation lives, which files have mixed responsibilities, and any important limits of the current directory boundary.

These topics are a common set of questions, not a requirement to reproduce the same amount of prose in every document.
A small concern can have a brief description.
A dependency description should explain an architectural relationship; it need not reproduce every transitive include or common utility dependency.
Terms that already have a useful definition elsewhere should link to that definition so that neighboring descriptions use the same vocabulary.

The primary document describes the implementation that a contributor will encounter.
When a preferred future organization differs from the current code, the description distinguishes that aspiration from the current boundary.
In particular, a directory does not imply a separate library, a C++ namespace, a restricted public interface, or an acyclic dependency graph.
Any such boundary needs its own design and implementation before contributors can rely on it.

## Incremental Adoption

We introduce concerns where the existing responsibilities are clear enough to improve navigation by moving whole files.
The first application is the [compiler](../../source/slang/compiler.md) in `source/slang/`.
Its children cover a useful part of the compiler without assigning every remaining file to a newly invented category.
Code can remain directly in a parent directory until we have identified a worthwhile concern that owns it.

For a directory reorganization, we classify every file in the selected directory by its primary responsibility.
Files with an obvious home move together with their headers and supporting definitions or generator inputs.
Files that do not primarily belong to one of the selected concerns remain in the parent directory.
An architecture description records a mixed responsibility when splitting the file would require a separate design decision.
A small, straightforward split can be appropriate, but complete separation is not a prerequisite for introducing the directory.

The compiler's initial decomposition adds one level of child directories.
It preserves the existing C++ namespaces, declaration names, code organization within files, and build artifacts.
Implementation edits support the moves: include paths, generator paths, build descriptions, and other references must continue to find the relocated files.
This organization work does not require an audit or rewrite of the contracts and comments inside every moved file.

As the hierarchy develops, the same approach can describe more concerns and make their dependencies easier to understand.
That understanding may support later work on narrower interfaces, dependency cycles, namespaces, or build boundaries.
Those changes should be justified on their own terms rather than being prerequisites for making the existing implementation easier to navigate.

## Maintaining the Descriptions

When a change adds a concern or changes ownership between concerns, we update the affected primary documents and their parent links.
A change to an implementation detail usually belongs in the code or a detailed design document instead.
Keeping the architecture descriptions focused on services, relationships, and ownership makes them useful without turning them into a second inventory of every declaration.
