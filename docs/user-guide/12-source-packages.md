---
layout: user-guide
permalink: /user-guide/source-packages
---

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

How to name those modules, write `import` and `__include`, and keep implementation files from
colliding across packages is described in
[Writing Module Files, Import, and Include](module-files).

The manifest declares the package and its source dependencies:

```json
{
  "name": "my-shaders",
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

`name`, `exports`, and `license_files` are required. `dependencies` is optional. Package manifests
allow JSON comments.

`name` identifies the package throughout the dependency graph. `exports` lists relative source
roots whose primary module paths become importable. Every path in `license_files` must name a
non-empty license file inside the package repository. `slang package init` creates a file named
`LICENSE` containing placeholder text; keeping that filename is fine, but the file's placeholder
contents must be replaced before validation succeeds.

Dependency versions come from Git tags named `vMAJOR.MINOR.PATCH`, which package publishers must
treat as immutable. The tag is the package's version; `slang-package.json` does not repeat a
self-version. Fetching fails if a locked tag no longer identifies its locked commit.

Each dependency entry contains exactly one source:

- A Git dependency contains `git` and either `version` or `tag`. `git` may be a URL or a local Git
  repository path. `version` is a space-separated intersection of `>`, `>=`, `<`, and `<=`
  comparisons, or a single exact version, written without a `v` prefix. `tag` names one Git tag,
  including the `v` prefix. When both version selectors are present, `tag` wins.
- A path dependency contains only `path`, which must be relative to the manifest that declares it.
  The target directory must contain its own `slang-package.json`, and its package name must match
  the dependency key. A path selects that one tree of files and therefore has no version range.

For example, a package can check in another package under `vendor/noise`:

```json
{
  "name": "my-shaders",
  "exports": ["src"],
  "license_files": ["LICENSE"],
  "dependencies": {
    "noise": {
      "path": "vendor/noise"
    }
  }
}
```

The resolver reads every reachable path package manifest and includes all of its transitive
dependencies in the app's lock. Path packages are used in place and are not copied under
`.slang/packages/`; Git packages are fetched there.

One package name identifies one node in the graph. Git requirements from multiple dependents must
use the same Git location, and the resolver intersects their constraints and chooses the highest
satisfying tag. Path requirements for one name must resolve to the same canonical directory or
resolution fails. A path requirement wins over a Git requirement for the same name without
checking the Git version range, and the tool warns that the Git requirement was shadowed. The path
package's transitive dependencies are still resolved normally.

A path in the app or another local package may use `..` to leave the package that declares it,
which supports sibling packages in a larger checkout. `slang package update` and
`slang package validate` warn because fetching the declaring package alone may not reproduce that
layout. A path inside a Git release must remain in that release's checkout. A missing target is
always an error.

## Locking and fetching

`slang package update` resolves all manifests reachable from the app, materializes the resulting
dependency set, and writes one `slang-package-lock.json` in the app root. The lockfile is the
definitive dependency graph and records both Git and path packages. Nested packages' lockfiles are
not used for that build. `slang package fetch` requires the app lockfile, checks that it still
satisfies every recorded manifest, and checks out every direct and transitive Git dependency under
`.slang/packages/`. Path dependencies remain at their locked relative locations. Fetch never
changes dependency resolution, so it is the appropriate command for normal builds and CI.

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
first declaration must be `module NAME;`, where `NAME` matches the filename stem with hyphens
replaced by underscores. Namespace directories do not form part of the declaration. For example,
`src/acme/noise.slang` declares `module noise;`, and `src/acme/image-noise.slang` declares
`module image_noise;`. Every `.slang` file below `src/acme/noise/` belongs to that module and must
instead begin with `implementing noise;`. From the primary, `__include` those files with a path
relative to the primary (for example `__include "noise/hash";`), as shown in
[Writing Module Files, Import, and Include](module-files).

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

A registered local manifest must agree with the lock. If its exports or dependencies change,
`fetch` and `validate` report the drift. Run `slang package update --from-local` to resolve the
changed local manifests and their transitive requirements into the definitive lock. Unlike a
manifest path dependency, this is a project-local replacement for a package selected through Git:
the lock records both its original Git location and its effective path and requires the matching
registration in `.slang/overrides.json`. It therefore fails explicitly on another machine or in
CI. The local tree does not need to satisfy the shadowed Git version range, but all of its
transitive dependencies are resolved. Run normal `slang package update` before removing the
registration or committing a portable published resolution.

Fetched package trees contain source only. Compilation output must be written outside these trees
because the same source commit can be compiled against different resolved dependency graphs.
