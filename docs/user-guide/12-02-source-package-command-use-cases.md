---
layout: user-guide
permalink: /user-guide/source-package-command-use-cases
---

Growing an Application with Source Packages
============================================

This chapter follows one application from an empty directory to a graph of packages. Each journey
says what you do by hand and what `slang package` does. The ready-made `video-preview` example is
in [Using Source Packages](source-package-workflow). Manifest fields are in
[Slang Source Packages](source-packages). Options are in
[Slang Package Command Reference](source-package-command-reference).

Journeys 1 through 7 use stable commands. Binary modules, host executables, and `run` are
experimental and are collected in Journey 8.

## Two files

- `slang-package.json` is intent. You edit and commit it. A dependency is a Git URL and a version
  range.
- `slang-package-lock.json` is the graph `update` selected for this workspace. You review and
  commit a release lock. A row that records `branch` is an edit in progress; unedit it before
  committing.

`deps/` checkouts, `.slang/repositories/`, `out/`, and `slang-package-includes.txt` are generated.
`init` gitignores them. A `slang-package-overlay.json` that contains entries is an error.

## Journey 1: bootstrap one application

Create the package in the current directory. The directory name becomes the package name.

```sh
mkdir video-preview
cd video-preview
slang package init
```

`init` writes `slang-package.json`, a placeholder `LICENSE`, `src/`, `tests/`, `docs/`, `deps/`,
and `out/`. It records `tools.slang-toolchain` as `>=` the installed compiler when that version
can be parsed. It does not write a lock: there is nothing to resolve yet.

Replace the placeholder license. Bare `validate` rejects the generated text. Add a primary module
whose filename stem matches the `module` declaration, with hyphens turned into underscores:

```slang
module video_preview;
```

saved as `src/video-preview.slang`. Then:

```sh
slang package validate
slang package status
```

`validate` is the publish check. `status` reports a workspace with no dependencies as current.
Commit `slang-package.json`, `LICENSE`, and `src/`. Do not commit `deps/`, `out/`, or `.slang/`.

## Journey 2: add the first published dependency

A dependency is a Git repository that already has a canonical release tag. `v1` is the tag for
release `1`; `v1.1` is the tag for `1.1`.

```sh
slang package dependency add color-encoding \
    --git https://github.com/jhelferty-nv/color-encoding.git \
    --version "^1"
slang package update
```

`dependency add` writes only the manifest. `update` fetches tags into `.slang/repositories/`,
selects one release, checks it out at `deps/color-encoding`, and writes
`slang-package-lock.json`. Review the printed graph and answer `y`, or pass `--yes` from a
script.

When `SLANG_PACKAGE_INDEX` names a package index, you can omit `--git` and the command copies
the URL from that index. `--version` is still required. An `http` or `https` index is downloaded
with `curl`.

```sh
slang package status
slang package tree
slang package why color-encoding
```

`status` should report the lock current and the workspace buildable. `tree` prints the selected
graph. `why` prints why this lock contains the package. Neither command contacts a remote.

Commit the manifest and the lock together. A later clone runs `fetch`, not `update`, and gets
this graph.

Adding the same name again replaces the version range. It still does not rewrite the lock until
`update`.

## Journey 3: author and publish the first library

The steps that make `color-encoding` publishable are the same ones Journey 1 used, in that
package's own repository.

`slang package tag 1.0.0` checks that the package is suitable for sharing, then creates the
annotated tag `v1` at `HEAD`. `1.1` creates `v1.1`. The command does not push. Push the tag
yourself. Consumers then use Journey 2.

A retraction belongs in the package that owns the bad release. Publish it on a newer tag. The
manifest entry names the release, for example `"version": "1.0.0"`, which is tag `v1`. Existing
locks stay on the old commit until someone runs `update`.

## Journey 4: work on a consumed package locally

The checkout at `deps/color-encoding` is detached on the locked commit. To change it, put it on
a branch:

```sh
slang package edit color-encoding --branch luma-fix --create
```

`--create` starts the branch at the commit the lock already selected. The lock keeps that
release's tag, commit, and version, and records `branch`. The pin boolean is unchanged. Commit
inside `deps/color-encoding` with ordinary Git.

`update` does not move this checkout. It reads the working-tree manifest and solves the
dependencies that manifest declares. `fetch` leaves the edit alone. `status` lists it. Bare
`validate` rejects the application while the edit is open. `validate color-encoding` checks that
package on its own.

`edit color-encoding --advance` moves the represented release forward to the greatest canonical
tag on the line of history from the branch tip back to the commit the row is representing. The
files at `HEAD` stay put. Use this after you have merged or rebased onto a newer published
release and want the lock to say which release the branch now contains.

`pin color-encoding` during the edit sets the boolean only. `unpin` clears it. `update` will not
move a pinned version onto a newer tag.

When the fix should become the locked release, end the edit:

```sh
slang package unedit color-encoding
```

Plain `unedit` selects the greatest newer canonical tag on that line that still satisfies
incoming constraints. `--restore` returns to the release the edit started from. `--advance`
checks out a newer legal tag. `--tag 1.2` creates a local annotated tag `v1.2` on the current
commit and selects it; you push that tag yourself. These endings cannot be combined. Dropping
commits asks, and declining leaves the edit. `--clean` discards uncommitted files so the checkout
may move.

Push the branch when you want it on a remote. After `unedit` selects a tag you have pushed,
commit the application's lock.

## Journey 5: a consumed package adds a dependency

Upstream `color-encoding` grows a dependency of its own and publishes `v1.1`. Your manifest still
says `^1`, which already admits that release. You do not edit a transitive edge by hand.

```sh
slang package update --dry-run
slang package update
```

`--dry-run` prints the lock it would write, including the new transitive package, and leaves
`deps/` and the lock unchanged. `update` materializes the new checkout and rewrites the lock.
Commit that lock. Consumers who only `fetch` stay on the previous graph until this lock is
committed.

If the new release retracts the one you had locked, the dry run shows that retraction. Fetch of
the old lock does not apply it.

## Journey 6: a consumed package splits in two

Upstream divides `color-encoding` into `color-encoding` and `color-transfer`, each in its own
Git repository, and tags a release of each. There is one version of each package name in the
graph.

In the application, point the direct dependency at the repository you import from, with the
range that admits the new tags:

```sh
slang package dependency add color-encoding \
    --git https://github.com/jhelferty-nv/color-encoding.git \
    --version "^2"
slang package update
```

`update` reads the published manifests. The new `color-encoding` release declares
`color-transfer`, and the solver checks that package out once. `why color-transfer` shows the
path. Commit the manifest and the lock.

Both repositories have to be tagged before this update can select them. The application cannot
try the split against two directories that have not been published.

## Journey 7: split your application into packages

Move a cohesive piece of `video-preview`, such as the color math, into a new Git repository.
Give that repository the Journey 1 skeleton: `slang package init`, a real license, exported
source, and `validate`. Tag `v1` and push it.

In the application, delete the moved files from the export so the same primary is not declared
twice. Then depend on the published repository:

```sh
slang package dependency add color-math \
    --git https://github.com/example/color-math.git \
    --version "^1"
slang package update
slang package validate color-math
```

`validate color-math` checks the locked library. Bare `validate` checks the application. Commit
the application's manifest and lock. A clone of the application runs `fetch` and gets
`color-math` at `deps/color-math`.

The extracted package follows Journey 4 when you need to change it from the application: `edit`
checks out a branch of that same `deps/color-math` tree.

## Journey 8: build binary artifacts (experimental)

Stable `bundle` copies source and documentation. Compiling modules and host executables requires
`--experimental` immediately before the command.

In the application manifest, `build.host.executables` lists filenames that match exported
primaries. The tool adds the platform suffix and writes them under `out/host`.

```sh
slang package bundle
slang package --experimental build
slang package --experimental run
slang package docs
```

`bundle` validates, writes `out/bundle/source/`, and collects `out/docs/`. `build` does that and
then compiles `.slang-module` files and the host executable. `run` executes the interpreter on
the bundled source. `run --binary` executes the native program from the last `build`. `docs`
opens the index `bundle` or `build` already wrote.

`slang package test` is reserved. It does not build or run package tests yet.

A later `bundle` removes `out/bundle/modules` and `out/host`. Treat those binary outputs as local
build products. CI that only needs the locked source should run `fetch`, `status`, and `bundle`,
as [the workflow chapter](source-package-workflow#what-ci-should-run) describes.
