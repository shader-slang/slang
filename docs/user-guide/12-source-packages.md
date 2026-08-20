# Slang Source Packages

The `slang package` command manages source dependencies stored in Git repositories. The short form
`slang pkg` accepts the same commands. Package management does not change Slang's `import` syntax
and does not build or distribute `.slang-module` files.

## Package layout

A package uses these conventional paths:

```text
slang-package.json
src/
tests/
docs/
```

`src/` contains importable Slang modules. `tests/` and `docs/` are reserved for package tests and
documentation. The initial package tool creates these directories but does not run tests or build
documentation.

The manifest declares the package and its source dependencies:

```json
{
  "name": "my-shaders",
  "version": "0.1.0",
  "exports": ["src"],
  "dependencies": {
    "noise": {
      "git": "https://github.com/example/slang-noise.git",
      "tag": ">=v1.2.0 <v2.0.0"
    }
  }
}
```

Dependency versions come from Git tags named `vMAJOR.MINOR.PATCH`, which package publishers must
treat as immutable. Fetching fails if a locked tag no longer identifies its locked commit. A
dependency can name one exact tag or a space-separated intersection of `>`, `>=`, `<`, and `<=`
comparisons. The resolver combines constraints from direct and transitive dependencies, chooses
the highest satisfying version, and reports an error when no version satisfies the full graph.

## Locking and fetching

`slang package fetch` resolves a project without a lockfile, writes `slang-package.lock`, and checks
out every direct and transitive dependency under `.slang/packages/`. The generated lockfile records
the exact tag and commit for every package. Later fetches use those commits without resolving again.

Use `slang package fetch --locked` in CI to require an existing compatible lockfile. Use
`slang package update` to query tags and resolve a new lockfile deliberately.

The tool invokes the `git` executable from the system path. Existing Git credential and SSH
configuration therefore applies without separate package-tool authentication. Git locations
cannot begin with `-`, use Git's command-executing `ext::` transport, or contain whitespace or
control characters.

After fetching, `.slang/search-paths` lists the source roots to pass to `slangc` with `-I`. Paths in
this file are relative to the package root and are not added to compiler sessions automatically.

## Creating and editing packages

`slang package init` creates `slang-package.json` and the conventional directories in the current
directory.

`slang package edit NAME` creates a project-local working copy under `.slang/edit/NAME`. Search
paths prefer that copy, while the lockfile retains the original resolved commit. Use
`slang package unedit NAME` to return to the locked checkout. `unedit` refuses to remove a checkout
that has local changes, local commits, or stashes.

Fetched package trees contain source only. Compilation output must be written outside these trees
because the same source commit can be compiled against different resolved dependency graphs.
