# The Language Server

The language server answers editor requests about Slang programs and manages the workspace information used to answer them.
Its services include completion, navigation, hover information, document symbols, semantic tokens, inlay hints, formatting, and diagnostic updates.

## Concepts

A _document version_ records a document's text and the position mappings needed to relate editor requests to compiler source locations.
A _workspace_ maintains documents and compilation state for the files being edited.
The _Language Server Protocol_ defines the requests and responses exchanged with editors; its shared representation lives in `source/compiler-core/`.

## Dependencies

- Compiler sessions, linkage, modules, and diagnostics provide compilation services for the documents in a workspace.
- [AST](../ast/ast.md) and [semantic checking](../semantic-checking/semantic-checking.md) provide program information used for queries such as completion and navigation.
- `source/compiler-core/` supplies protocol types, JSON-RPC infrastructure, source locations, and shared compiler utilities.
- Documentation extraction services in the compiler directory provide declaration documentation shown to users.
- `source/core/` supplies text, files, paths, collections, and synchronization support.

## Decomposition and Ownership

`slang-language-server.*` implements request handling and the language server core.
The other `slang-language-server-*` files implement individual features and AST lookup used by those features.
`slang-workspace-version.*` manages document versions and workspace compilation state.
`slang-content-assist-info.h` defines the information exchanged with preprocessing and checking to support completion and other editor services.

Shared protocol and JSON-RPC code remains in `source/compiler-core/`, and the `slangd` executable entry point remains in `tools/slangd/`.
The preprocessor and semantic checker fill in content-assist records declared here, so this concern supplies types used by those compiler phases as well as depending on their services.
The directory groups editor services without extracting a new independently built library or changing how those services access compiler internals.
