---
layout: user-guide
permalink: /user-guide/source-packages
---

Slang Source Packages
=====================

The `slang package` command manages source dependencies stored in Git repositories. The short form
`slang pkg` accepts the same commands. Package management does not change Slang's `import` syntax
and does not build or distribute `.slang-module` files.

A **package** is a directory with `slang-package.json`. Its name, exports, license files, and
dependencies apply wherever that package appears in a graph, including as a Git pin or a path
dependency.

A **workspace** is the directory where you run `slang package` for a given solve. It is also a
package: the one whose manifest starts resolution. The workspace owns `slang-package-lock.json` and
generated state under `.slang/`. Nested packages' lockfiles are not used for that solve. A module
is a Slang language unit (`module NAME;`), not a package-manager concept.

## Package layout

A package uses these conventional paths:

```text
slang-package.json
LICENSE
src/
tests/
docs/
```

`src/` contains importable Slang modules. When `exports` includes `src`, the workspace package's
default run module is `main`: `src/main.slang` must declare `module main;`. `slang package run`
(not yet implemented) will use that module unless a later option selects another primary.

`tests/` and `docs/` are reserved for package tests and documentation. The initial package tool
creates these directories.

How to name those modules, write `import` and `__include`, and keep implementation files from
colliding across packages is described in
[Writing Module Files, Import, and Include](module-files).

The manifest declares the package and its source dependencies:

```json
{
  "schema_version": 1,
  "name": "my-shaders",
  "exports": ["src"],
  "license_files": ["LICENSE"],
  "workspace": {
    "deps": "deps",
    "build": "build"
  },
  "dependencies": {
    "noise": {
      "git": "https://github.com/example/slang-noise.git",
      "version": ">=1.2.0 <2.0.0"
    }
  }
}
```

`schema_version` is the file format version and is currently `1`. `name`, `exports`, and
`license_files` are also required. `dependencies` and `workspace` are optional. Package manifests
allow JSON comments.

`name` identifies the package throughout the dependency graph. `exports` lists relative source
roots whose primary module paths become importable. Every path in `license_files` must name a
non-empty license file inside the package repository. `slang package init` creates a file named
`LICENSE` containing placeholder text; keeping that filename is fine, but the file's placeholder
contents must be replaced before validation succeeds.

The `workspace` object is read only from the manifest that starts the solve. `deps` is where Git
dependency source is materialized, and `build` contains generated workspace output. Their defaults
are `deps/` and `build/`; `slang package init` writes those defaults explicitly. The same fields in
a dependency's manifest do not affect the enclosing workspace.

Dependency versions come from Git tags named `vMAJOR.MINOR.PATCH`, which package publishers must
treat as immutable. The tag is the package's version; `schema_version` in `slang-package.json` is
only the file format version. Fetching fails if a locked tag no longer identifies its locked
commit.

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
dependencies in the workspace lock. Path packages are used in place and are not copied under
`deps/`; Git packages are fetched there.

One package name identifies one node in the graph. Git requirements from multiple dependents must
use the same Git location, and the resolver intersects their constraints and chooses the highest
satisfying tag. Path requirements for one name must resolve to the same canonical directory or
resolution fails. A path requirement wins over a Git requirement for the same name without
checking the Git version range, and the tool warns that the Git requirement was shadowed. The path
package's transitive dependencies are still resolved normally.

A path in the workspace package or another local package may use `..` to leave the package that
declares it, which supports sibling packages in a larger checkout. `slang package update` and
`slang package validate` warn because fetching the declaring package alone may not reproduce that
layout. A path inside a Git release must remain in that release's checkout. A missing target is
always an error.

## Locking and fetching

`slang package update` resolves all manifests reachable from the workspace package, materializes
the resulting dependency set, and writes one `slang-package-lock.json` in the workspace root. The
lockfile is the definitive dependency graph and records both Git and path packages. It starts with
`"schema_version": 1`, the same file-format version as `slang-package.json` and
`slang-workspace.json`. Nested
packages' lockfiles are not used for that solve. `slang package fetch` requires the workspace
lockfile, checks that it still satisfies every recorded manifest, and checks out every direct and
transitive Git dependency under `workspace.deps` (`deps/` by default). Path dependencies remain at their locked
relative locations. Fetch never changes dependency resolution, so it is the appropriate command
for normal builds and CI.

Dependency checkout paths are stable. A pin stays at `deps/NAME` while it is tool-owned, edited,
and returned to tool ownership. Fetch and update refuse to replace an unregistered checkout with
changed files, extra commits, or stashes. Pass `--clean` explicitly to permit replacement.

Run `slang package update` deliberately when manifest constraints or upstream releases change.
Normal CI and developer builds use `slang package fetch`; a missing or inconsistent lock is an
error.

The tool invokes the `git` executable from the system path. Existing Git credential and SSH
configuration therefore applies without separate package-tool authentication. Git locations
cannot begin with `-`, use Git's command-executing `ext::` transport, or contain whitespace or
control characters.

After fetching, `{workspace.build}/search-paths` (by default `build/search-paths`) lists the source
roots to pass to `slangc` with `-I`. It is a derived file and may be deleted with the rest of the
build directory; fetch or update regenerates it. Paths in this file are relative to the workspace
root and are not added to compiler sessions automatically.

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
directory. It adds `.slang/`, `deps/`, `build/`, and `slang-workspace.json` to `.gitignore`.
`.slang/cache/` contains resolver Git repositories used to inspect release manifests. Fetched
source remains visible under `deps/`; generated files go under `build/`.

`slang package edit NAME` marks the existing `{workspace.deps}/NAME` checkout (by default
`deps/NAME`) as editable without moving it. The Git pin remains in the lock; gitignored
`slang-workspace.json` records that the package tool no longer owns the working tree. Fetch and
update do not modify an edited checkout. Use `slang package unedit NAME` to return an unchanged
checkout to package-tool ownership. `unedit` refuses while the checkout has changed files, commits
not selected by the lock, or stashes.

For example, the generated local-state file may contain:

```json
{
  "schema_version": 1,
  "edits": {
    "noise": {}
  },
  "overrides": {
    "shared": {
      "path": "../shared"
    }
  }
}
```

Use the package commands to change this file; its schema is tool-owned and may evolve.

`slang package override NAME PATH` uses an existing local package directory instead. Both commands
register local state in `slang-workspace.json`, which must remain uncommitted. An override does not
copy or modify the supplied directory. `slang package unoverride NAME` removes an override
registration.
`unoverride` refuses while the lock has a local-path entry for the package; run normal
`slang package update` first to restore a published pin.

A registered local manifest must agree with the lock. An in-place edit keeps the published Git pin
in the lock, so changing its exports or dependencies requires publishing a new release tag and
running normal `slang package update`. Use an override when local manifest changes must participate
in resolution before publication. `slang package update --from-local` resolves override manifests
and their transitive requirements into the definitive lock. An override records both its original
Git location and its effective path and requires the matching registration in
`slang-workspace.json`; it therefore fails explicitly on another machine or in CI. A local tree
does not need to satisfy the shadowed Git version range, but all of its transitive dependencies are
resolved. Run normal `slang package update` before removing an override registration or committing
a portable published resolution.

Fetched package trees contain source only. Compilation output must be written outside these trees
because the same source commit can be compiled against different resolved dependency graphs.

`slang package build` validates the materialized package graph and compiles every primary module in
the workspace package to a front-end `.slang-module` under `workspace.build`, preserving its import
path. For example, `src/acme/noise.slang` becomes `build/acme/noise.slang-module`.

## Possible future enhancements

The initial workspace layout deliberately keeps resolver clones in `.slang/cache/` and compile
inputs in the workspace. Future versions may add a user-global immutable cache with copy-on-edit,
let compiler sessions consume workspace metadata without `build/search-paths`, and share immutable
dependency trees between workspaces.
