#!/usr/bin/env bash
# Filename patterns to exclude for slangc compiler-only coverage.
# These are files compiled into libslang but not part of the compiler pipeline:
#   - Generated code (prelude, capability defs, FIDDLE, lookup tables, core module meta)
#   - External/third-party code, public API headers
#   - Core module embedding, GLSLANG bridge
#   - Record-replay instrumentation
#   - Language server, documentation generator, unmaintained debug features
#
# Shared by run-coverage.sh and ci-slang-coverage.yml to keep the filter
# definition in a single place.

SLANGC_IGNORE_ARGS=(
  -ignore-filename-regex='build/prelude/'
  -ignore-filename-regex='build/source/slang/(capability|fiddle|slang-lookup-tables)/'
  -ignore-filename-regex='build/source/slang-core-module/'
  -ignore-filename-regex='external/'
  -ignore-filename-regex='include/'
  -ignore-filename-regex='source/slang-core-module/'
  -ignore-filename-regex='source/slang-glslang/'
  -ignore-filename-regex='source/slang-record-replay/'
  -ignore-filename-regex='tools/'
  -ignore-filename-regex='source/slang/slang-(language-server|doc-markdown-writer|doc-ast|ast-dump|repro|workspace-version)[.\-]'

  # Language-server / doc-only files that live in compiler-core
  # (LSP protocol structs, JSON-RPC framing, doc-comment extraction).
  -ignore-filename-regex='source/compiler-core/slang-(language-server-protocol|json-rpc|doc-extractor)[.\-]'

  # FIDDLE-generated AST declaration headers (no executable code)
  -ignore-filename-regex='source/slang/slang-ast-(expr|modifier|stmt)\.h$'
)
