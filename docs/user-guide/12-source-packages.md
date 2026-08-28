---
layout: user-guide
permalink: /user-guide/source-packages
---

Slang Source Packages
=====================

The `slang package` command manages source dependencies stored in Git repositories. The short form
`slang pkg` accepts the same commands. Package management does not change Slang's `import` syntax;
its build command emits `.slang-module` files for the resolved graph and optional native
executables. A command-order walkthrough using the public `video-preview` demo is in
[Using Source Packages](source-package-workflow).

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

`src/` contains importable Slang modules. Host executables are workspace primaries whose filename
stem matches a name in `host.executables`. For example, `src/video-preview.slang` is the source for
the `video-preview` executable and must declare `module video_preview;`. `slang package run` uses
`host.default`, or the only listed executable when `default` is omitted.

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
    "build": "build",
    "excludes": [
      {
        "package": "noise",
        "version": "1.3.0",
        "reason": "Known regression in generated gradients"
      }
    ]
  },
  "host": {
    "executables": ["my-shaders"],
    "default": "my-shaders"
  },
  "dependencies": {
    "noise": {
      "git": "https://github.com/example/slang-noise.git",
      "version": ">=1.2.0 <2.0.0"
    }
  },
  "tools": {
    "slang-toolchain": {
      "version": ">=2026.8.0"
    }
  },
  "retractions": [
    {
      "version": "1.1.0",
      "reason": "Published with an incomplete source export"
    }
  ]
}
```

`schema_version` is the file format version and is currently `1`. `name`, `exports`, and
`license_files` are also required. `dependencies`, `tools`, `workspace`, and `host` are optional.
Package manifests allow JSON comments.

`name` identifies the package throughout the dependency graph. `exports` lists relative source
roots whose primary module paths become importable. Every path in `license_files` must name a
non-empty license file inside the package repository. `slang package init` creates a file named
`LICENSE` containing placeholder text; keeping that filename is fine, but the file's placeholder
contents must be replaced before validation succeeds.

The `workspace` object is read only from the manifest that starts the solve. `deps` is where Git
dependency source is materialized, and `build` contains generated workspace output. Their defaults
are `deps/` and `build/`; `slang package init` writes those defaults explicitly. The same fields in
a dependency's manifest do not affect the enclosing workspace.

`workspace.bundle` selects optional distribution outputs under `build/bundle/`. `modules` (default
true) compiles every primary to `build/bundle/modules/` and writes `provenance.json` beside those
artifacts. `source` (default true) copies every exported `.slang` file into
`build/bundle/source/` using the same import-relative layout, so that directory is one compiler
search path. Two files that would occupy the same name on a case-insensitive filesystem are an
error. Set either flag to `false` to skip that output.

The optional root `host` object requests native host executables. `executables` lists output
filenames without directory separators; the package tool adds the platform executable suffix and
writes each result at the root of `workspace.build`. Each name must match an exported workspace
primary whose source filename is `<name>.slang`. When more than one executable is listed,
`default` names the artifact `slang package run` executes if you do not pass an executable name.
With a single executable, `default` may be omitted and that name is used. Dependency manifests may
declare this field, but only the workspace package controls executable generation.

The optional `tools` object declares required **system tools**: programs already installed on the
machine, not source packages and not lock rows. `slang package` checks that each named tool is
present and that its version satisfies the declared constraint. It does not fetch those tools into
`deps/` or record their versions in `slang-package-lock.json`.

Schema `1` accepts only `slang-toolchain`, which is the Slang compiler that provides
`slang-package`, regardless of how it was installed (a shader-slang/slang GitHub release, a Vulkan
SDK distribution, or another bundle). Its `version` uses the same constraint grammar as Git
dependencies (three-part versions, no `v` prefix). In the short term this is how a package states
the minimum compiler it needs to work correctly: a builtin such as `neural`, a standard-library
API, or a compiler fix that did not exist (or was broken) before that version. There is no
separate way to depend on those builtins, so the toolchain version stands in for them.

Every package in the graph may declare the field; `update`, `fetch`, `status`, `build`, and
`validate` intersect those constraints against the installed compiler. `slang package init` writes
`>=` that installed version when it can parse it. Slang compiler versions are calendar-based, not
API levels, so a toolchain constraint should be a lower bound such as `>=2026.8.0`. Do not cap it
at a speculative future date. Use `!=` to skip one known-bad compiler version without inventing an
upper bound, for example `>=2026.8.0 !=2026.9.0`.

Later schemas may add other system tools, or split Slang into separately versioned components if
distributions start shipping them independently. Unknown `tools` keys are errors today.

Ordinary dependency versions come from Git tags named `vMAJOR.MINOR.PATCH`, which package
publishers must treat as immutable. A manifest may instead pin an opaque branch or tag with `ref`
and assign its solver identity with `as`. `schema_version` in `slang-package.json` is only the file
format version. The lock always records the resolved ref, exact semantic version, and commit.

The optional top-level `retractions` array is publisher advice not to select releases matching a
version constraint. Each entry requires `version` and a non-empty `reason`. To retract a published
release, add the entry to the manifest and publish a new, higher release tag. During resolution,
the package tool reads retractions from the highest available release, even when that release is
outside the consumer's requested range, and skips matching candidates. A retraction does not
invalidate an existing lock: `fetch` remains reproducible, while the next `update` moves away from
the retracted release when another candidate satisfies the graph.

The root-only `workspace.excludes` array is committed consumer policy. Each entry names a
`package`, a `version` constraint, and a non-empty `reason`. Dependency manifests' workspace
settings, including exclusions, are ignored for the solve. If a reachable package lists
`workspace.excludes` entries that this workspace does not copy (same `package` and `version`
strings), `update`, `fetch`, `status`, `validate`, and `build` warn so those ignored excludes are
visible. Resolution skips excluded Git releases. Unlike a publisher retraction, adding an
exclusion changes the workspace's declared resolution intent, so
`fetch` rejects a lock that still selects an excluded release and asks for `slang package update`.
Path dependencies and local overrides carry an effective version for solver compatibility, but
workspace exclusions apply only to remote Git selections.

Each dependency entry has one of four shapes:

- `git` plus `version` selects the highest compatible `vMAJOR.MINOR.PATCH` release tag.
- `path` plus `as` uses one relative tree as the exact semantic version named by `as`.
- `git`, `ref`, and `as` pins an opaque branch or tag and uses `as` as its exact solver version.
- `git`, `version`, `ref`, and `as` adds a compatibility assertion: `as` must satisfy `version`, or
  manifest validation fails.

`git` may be a URL or a local Git repository path. A `version` is one or more clauses joined by
`||`. Each clause is a space-separated intersection of `>`, `>=`, `<`, `<=`, and `!=` comparisons,
or a single exact version. For example, `>=1.2.0 !=1.3.0` skips 1.3.0, and
`>=1.0.0 <1.3.0 || >=1.3.1 <2.0.0` accepts either interval. Dependents still unify one version per
package name: every incoming constraint must match that version. Both `version` and `as` omit the
release tag's `v` prefix. `ref` is normally a branch or tag; the lock, rather than the
manifest, records the exact commit.

A dependency `path` must be relative to the manifest that declares it. The target directory must
contain its own `slang-package.json`, and its package name must match the dependency key.

For example, a package can check in another package under `vendor/noise`:

```json
{
  "name": "my-shaders",
  "exports": ["src"],
  "license_files": ["LICENSE"],
  "dependencies": {
    "noise": {
      "path": "vendor/noise",
      "as": "1.4.0"
    }
  }
}
```

The resolver reads every reachable path package manifest and includes all of its transitive
dependencies in the workspace lock. Path packages are used in place and are not copied under
`deps/`; Git packages are fetched there.

One package name identifies one node in the graph. Git requirements from multiple dependents must
use the same Git location, and the resolver intersects their constraints and chooses the highest
satisfying tag. Path requirements for one name must resolve to the same canonical directory and
claim the same `as` version or resolution fails. A path requirement wins over the Git source for
the same name, but its `as` version must satisfy every Git version constraint and pinned `as`
identity. The tool warns that the Git source was shadowed. The path package's transitive
dependencies are still resolved normally.

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

Every lock row has an exact `version`. A Git row also records the selected `ref` and `commit`; a
range-selected release uses its `vMAJOR.MINOR.PATCH` tag as the ref. A path row records its
effective `as` version as `version`. A local-override row records its original Git location, local
path, and effective version.

Dependency checkout paths are stable. A pin stays at `deps/NAME` while it is tool-owned, edited,
and returned to tool ownership. Fetch and update refuse to replace an unregistered checkout with
changed files, extra commits, or stashes. Pass `--clean` explicitly to permit replacement.

Run `slang package update` deliberately when manifest constraints or upstream releases change.
`slang package update --dry-run` and `slang package update --from-local --dry-run` print the
selected graph (what moved, what stayed, and why) without writing the lock or replacing
checkouts. `--minimal` keeps one-line package changes and the summary count. Resolver Git clones
under `.slang/cache/` may still be populated so the tool can inspect available tags. Normal CI and
developer builds use `slang package fetch`; a missing or inconsistent lock is an
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

The same validation is part of commands that establish or consume a dependency graph:

- `fetch` validates the workspace package before changing dependency checkouts, then validates the
  complete reachable graph after materialization.
- `update` validates the workspace package before solving, then validates the complete selected
  graph after materialization and before writing the new lock. It does not validate the old graph
  first, so a broken old dependency cannot prevent an update to a fixed release.
- `update --dry-run` validates the workspace package and every manifest read by the solver. It
  cannot validate source files from remote candidates because it does not materialize them.
- `build` performs the same full graph validation before compiling anything.

Successful `fetch`, `update`, and `build` therefore require the workspace package and every
reachable dependency to conform to the closed manifest schema, license and export rules, module
layout, and graph-wide module import uniqueness. Validation covers the whole reachable graph, not
only changed lock rows, because an unchanged package can conflict with a newly selected module.

`slang package status` checks the root manifest against the lock, verifies registered edits and
overrides, validates the materialized manifests, and checks that tool-owned Git checkouts remain at
their locked commits without changed files or stashes. It prints the registered local package
state and reports the corrective `fetch`, `update`, `update --from-local`, `edit`, or `--clean`
command when the workspace is inconsistent. The command does not modify package state or contact
remotes.

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
directory. It writes `tools.slang-toolchain` as `>=` the installed compiler version when that
version can be parsed. It adds `.slang/`, `deps/`, `build/`, and `slang-workspace.json` to
`.gitignore`.
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
      "path": "../shared",
      "as": "2.3.0"
    }
  }
}
```

Use the package commands to change this file; its schema is tool-owned and may evolve.

`slang package override NAME PATH [AS]` uses an existing local package directory instead. `AS` is
an exact semantic version for solver compatibility. When it is omitted, the command uses the
version in the package's current lock row. Both commands register local state in
`slang-workspace.json`, which must remain uncommitted. An override does not copy or modify the
supplied directory. `slang package unoverride NAME` removes an override registration.
`unoverride` refuses while the lock has a local-path entry for the package; run normal
`slang package update` first to restore a published pin.

A registered local manifest must agree with the lock. An in-place edit keeps the published Git pin
in the lock, so changing its exports or dependencies requires publishing a new release tag and
running normal `slang package update`. Use an override when local manifest changes must participate
in resolution before publication. `slang package update --from-local` resolves override manifests
and their transitive requirements into the definitive lock. An override records both its original
Git location and its effective path and requires the matching registration in
`slang-workspace.json`; it therefore fails explicitly on another machine or in CI. The override's
effective version must satisfy every incoming constraint, and all of its transitive dependencies
are resolved. Run normal `slang package update` before removing an override registration or
committing a portable published resolution.

Fetched package trees contain source only. Compilation output must be written outside these trees
because the same source commit can be compiled against different resolved dependency graphs.

`slang package build` validates the materialized package graph. When `workspace.bundle.modules` is
enabled, it compiles every primary module in the workspace and its resolved dependencies to a
front-end `.slang-module` under `workspace.build/bundle/modules`, preserving its import path. For
example, an exported `src/acme/noise.slang` becomes `build/bundle/modules/acme/noise.slang-module`,
whether that source belongs to the workspace or a dependency. Companion files included by that
primary are compiled into the same artifact and do not produce separate files.

The `.slang-module` format is not independently versioned, so those files are only useful with the
same Slang toolchain that produced them. Build writes `build/bundle/modules/provenance.json`
identifying that compiler (`name`, `version`, and `path`).

When `workspace.bundle.source` is enabled, build also copies every exported `.slang` file into
`build/bundle/source/` at the same import-relative paths. The resulting tree is a single search
path: `src/acme/noise.slang` and its companion `src/acme/noise/helper.slang` become
`build/bundle/source/acme/noise.slang` and `build/bundle/source/acme/noise/helper.slang`. A
case-insensitive name collision across packages is an error.

The `build/bundle/modules` tree is sufficient for source-free consumption: place it on the
consumer's search path and distribute it without the materialized `deps/` source trees. Imports
from one generated module to another resolve at the same import-relative paths used by source. The
`build/bundle/source` tree is the corresponding source-form search path.

When `host.executables` is present, build also compiles each matching workspace primary with the
host executable target and writes `build/<executable-name>` (plus `.exe` on Windows). The `main`
function in that file must use the native ABI, such as `export __extern_cpp int main()`, and a
supported downstream C++ compiler must be available. Build copies the matching `slang-rt` shared
library beside the executables so the artifacts can locate their runtime support. Configuring a
host executable without a matching workspace `.slang` primary is an error.

The same command copies every `.md` file below each materialized package's `docs/` directory to
`build/docs/<package-name>/`, preserving paths below `docs/`. Namespacing the output by package
keeps files such as `docs/README.md` from different packages distinct. Other file types are not
copied. Build also writes `build/docs/index.md`: the workspace dependency tree, then an
alphabetized list of copied Markdown files per package. Every package in the graph is listed;
only packages that contributed Markdown link from the tree into that file list.

`slang package run` executes an existing native artifact from `host.executables` and forwards
trailing arguments. With no executable name, it runs `host.default`. It does not build first. Run
fails with instructions when the manifest does not configure a host executable or when
`slang package build` has not produced that artifact.

`slang package test` is reserved and currently reports that it is not implemented. It does not
invoke `slang-test`. Package testing will get a dedicated model; `slang-test` remains an internal
compiler harness and is not part of the package command surface.

`slang package docs` prints the path of the generated `build/docs/` directory so you can open
`index.md` and the copied package files. It does not copy or regenerate documentation; run
`slang package build` for that.

## Possible future enhancements

The initial workspace layout deliberately keeps resolver clones in `.slang/cache/` and compile
inputs in the workspace. Future versions may add a user-global immutable cache with copy-on-edit,
let compiler sessions consume workspace metadata without `build/search-paths`, and share immutable
dependency trees between workspaces. Git-to-Git replacement is also deferred until Slang has a
global user remapping policy or package-index integration; current overrides intentionally replace
a dependency with a local path only.
