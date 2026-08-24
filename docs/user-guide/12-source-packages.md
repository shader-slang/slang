Slang Source Packages
=====================

The `slang package` command manages source dependencies stored in Git repositories. The short form
`slang pkg` accepts the same commands. Package management does not change Slang's `import` syntax
and does not build or distribute `.slang-module` files.

## Package layout

A package uses these conventional paths:

```text
slang-package.json
LICENSE
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
  "license_files": ["LICENSE"],
  "dependencies": {
    "noise": {
      "git": "https://github.com/example/slang-noise.git",
      "version": ">=1.2.0 <2.0.0"
    }
  }
}
```

Every path in `license_files` must name a non-empty license file in the package repository.
`slang package init` creates a `LICENSE` placeholder that must be replaced before validation
succeeds.

Dependency versions come from Git tags named `vMAJOR.MINOR.PATCH`, which package publishers must
treat as immutable. Fetching fails if a locked tag no longer identifies its locked commit.

A dependency uses either `version` or `tag`. `version` is a space-separated intersection of `>`,
`>=`, `<`, and `<=` comparisons, or a single exact version, written without a `v` prefix. `tag`
names one Git tag, including the `v` prefix. When both fields are present, `tag` wins. The
resolver combines constraints from direct and transitive dependencies, chooses the highest
satisfying version, and reports an error when no version satisfies the full graph.

## Locking and fetching

`slang package update` resolves the manifest against Git release tags and writes
`slang-package-lock.json`. The lockfile is the definitive dependency graph. `slang package fetch`
requires that lockfile, checks that it still satisfies the manifest, and checks out every direct
and transitive Git dependency under `.slang/packages/`. Fetch never changes dependency resolution.

Run `slang package update` deliberately when manifest constraints or upstream releases change.
Normal CI and developer builds use `slang package fetch`; a missing or inconsistent lock is an
error.

The tool invokes the `git` executable from the system path. Existing Git credential and SSH
configuration therefore applies without separate package-tool authentication. Git locations
cannot begin with `-`, use Git's command-executing `ext::` transport, or contain whitespace or
control characters.

After fetching, `.slang/search-paths` lists the source roots to pass to `slangc` with `-I`. Paths in
this file are relative to the package root and are not added to compiler sessions automatically.

## Validating packages

`slang package validate` checks the current package and every materialized package reachable
through `slang-package-lock.json`. It validates each manifest and license file, requires all source
exports to exist, and rejects module import paths exported by more than one package.

Each `.slang` file that is not below a module's companion directory is a primary module file. Its
first declaration must be `module NAME;`, where `NAME` matches the filename. Namespace directories
do not form part of the declaration. For example, `src/acme/noise.slang` declares `module noise;`.
Every `.slang` file below `src/acme/noise/` belongs to that module and must instead begin with
`implementing noise;`.

## Creating and editing packages

`slang package init` creates `slang-package.json` and the conventional directories in the current
directory. It also adds `.slang/` to `.gitignore`; that directory contains generated checkouts,
search paths, and developer-local registrations.

`slang package edit NAME` creates a project-local working copy under `.slang/edit/NAME`. Search
paths prefer that copy, while the lockfile initially retains the original resolved commit. Use
`slang package unedit NAME` to return to the locked checkout. `unedit` refuses to remove a checkout
that has local changes, local commits, or stashes. It also refuses while the lock has a local-path
entry for the package; run normal `slang package update` first to restore a published pin.

`slang package override NAME PATH` uses an existing local package directory instead. Both commands
register the local tree in `.slang/overrides.json`, which must remain uncommitted. An edit records
its original commit so `unedit` can check that removal is safe; an override does not copy or modify
the supplied directory. `slang package unoverride NAME` removes an override registration.
`unoverride` refuses while the lock has a local-path entry for the package; run normal
`slang package update` first to restore a published pin.

A registered local manifest must agree with the lock. If its version, exports, or dependencies
change, `fetch` and `validate` report the drift. Run `slang package update --from-local` to resolve
the changed local manifests and their transitive requirements into the definitive lock. Local
packages are then represented by `path` and `version` entries. Such a lock requires matching
registrations in `.slang/overrides.json`; it therefore fails explicitly on another machine or in
CI. Run normal `slang package update` before removing the registration or committing a portable
published resolution.

Fetched package trees contain source only. Compilation output must be written outside these trees
because the same source commit can be compiled against different resolved dependency graphs.
