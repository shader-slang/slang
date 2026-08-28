---
layout: user-guide
permalink: /user-guide/source-package-workflow
---

Using Source Packages
=====================

This chapter is a command walkthrough. It shows *when* to run each `slang package` command and
what you should see. Manifest fields, lock format, and validation rules are in
[Slang Source Packages](source-packages). Must-succeed and must-fail contracts for those commands
are in [Source Package Command Use Cases](source-package-command-use-cases). Module file naming is
in [Writing Module Files, Import, and Include](module-files).

The short form `slang pkg` accepts the same commands. Slang uses a single dash for
multi-character options, for example `-help`, not `--help`, except for `slang package` flags such
as `--dry-run` and `--from-local`.

## The example graph

The rest of this chapter uses the public demonstration workspace
[video-preview](https://github.com/jhelferty-nv/video-preview). It is a small command-line Rec. 709
preview, not a video player: it converts synthetic Y′CbCr samples and does not decode media or
open a window.

```text
video-preview
├── ycbcr-display
│   ├── color-convert
│   │   └── color-encoding
│   └── color-encoding
└── color-encoding
```

Three packages constrain `color-encoding`. The resolver intersects those ranges and selects one
tag, which appears once in the lock and is checked out once under `deps/`.

Clone the workspace and work from its root:

```sh
git clone https://github.com/jhelferty-nv/video-preview.git
cd video-preview
```

The committed lock selects `v1.0.0` for every Git dependency. The same repositories also publish
`v1.1.0`. `color-encoding` `v1.1.0` retracts `1.0.0` because that release used truncated luma
weights. That retraction is what makes `fetch` versus `update` interesting: fetch still uses the
lock; update will not reselect the retracted tag.

The workspace lists `video-preview` under `host.executables`. The matching primary is
`src/video-preview.slang`, which declares `module video_preview;`. There is no reserved
`main.slang` filename.

## Three files

Open these two committed files first:

```sh
less slang-package.json
less slang-package-lock.json
```

- `slang-package.json` is published intent: package name, exports, licenses, Git or path
  dependencies, optional `tools.slang-toolchain` for a minimum installed compiler (and thus its
  builtins and standard library), optional `host` executables, optional publisher `retractions`,
  and optional root-only `workspace` settings (`deps`, `build`, `excludes`).
- `slang-package-lock.json` is the exact graph this workspace selected. `fetch` reproduces it
  without solving again.
- `slang-workspace.json` is gitignored machine-local state for `edit` and `override`. It should
  not be in the clone. If it is missing, that is correct for CI and for a clean checkout.

`host` is a sibling of `workspace` in the manifest, not a field inside it. A dependency may declare
`host`; only the package you run `build` and `run` in produces executables.

Most Git dependencies use `git` plus a `version` range. To follow a branch or non-release tag,
write `git`, `ref`, and `as`; `ref` chooses the Git name while `as` supplies the exact semantic
version used by the solver. You may also retain `version` as a checked compatibility assertion:
validation fails when `as` is outside that range. Path dependencies similarly require `path` plus
`as`. In all cases the lock records the exact effective version, and Git locks additionally record
the resolved commit.

## Reproduce the locked graph

```sh
slang package fetch
git -C deps/ycbcr-display describe --tags --exact-match
git -C deps/color-convert describe --tags --exact-match
git -C deps/color-encoding describe --tags --exact-match
```

All three `describe` commands should print `v1.0.0`, even though `v1.1.0` already exists and even
though the later encoding release retracts `1.0.0`. Fetch never consults retractions and never
rewrites the lock.

Use fetch for ordinary development and for CI. Pass `--clean` only when you intend to replace a
dirty or unowned checkout.

If the lock is missing, fetch fails and asks you to run `update` once to create it.

## Check workspace consistency

```sh
slang package status
```

After a successful fetch, status should report that the lock is current, that there is no local
edit or override state, and that tool-owned Git checkouts are clean. Status does not fetch, does
not update, and does not contact remotes. When something is wrong, it names the command that
fixes it: `fetch`, `update`, `update --from-local`, `edit`, or `--clean`.

If you run status before fetch, it fails because the locked packages are not materialized yet.

## Preview a new solve, then apply it

```sh
slang package update --dry-run
git -C deps/color-encoding describe --tags --exact-match
```

`--dry-run` prints the selected graph (what moved, what stayed, and why) and leaves
checkouts and the lock file alone. Pass `--minimal` to keep one-line package changes without
the constraint rationale. The encoding checkout should still be `v1.0.0`. Resolver clones
under `.slang/cache/` may still be populated so the tool can list tags. `--dry-run` cannot be
combined with `--clean`.

Then apply the solve:

```sh
slang package update
git -C deps/ycbcr-display describe --tags --exact-match
git -C deps/color-convert describe --tags --exact-match
git -C deps/color-encoding describe --tags --exact-match
```

All three should now print `v1.1.0`. Convert's tighter encoding range (`>=1.1.0`) and the
publisher retraction of `1.0.0` agree: the shared leaf is `color-encoding@v1.1.0`, once, in the
lock.

Before changing checkouts, update validates the workspace package. After materializing the
selection, it validates every reachable package's manifest, licenses, exports, and module layout,
and checks module import uniqueness across the graph. The new lock and successful resolution
report are written only after those checks pass. Fetch performs the same pre- and
post-materialization validation while reproducing the existing lock. `--skip-validate` is an
escape hatch that leaves lock identity checks in place but skips source, license, and
module-layout validation.

`v1.1.0` of the preview prints full-precision luma weights `(0.2126, 0.7152, 0.0722)` instead of
the truncated `(0.2130, 0.7150, 0.0720)` from `v1.0.0`.

Update the entire graph when you mean to take newer compatible releases. There is no
package-specific update mode yet.

## Build, run, and collect docs

```sh
slang package build
```

Build validates the materialized graph, then:

- When `workspace.bundle.modules` is enabled (the default), emits a `.slang-module` for every
  primary in the workspace and its dependencies under `build/bundle/modules/`, preserving
  import-relative paths (`video-preview`, `video/display`, `color/convert`, `color/encoding`), and
  writes `build/bundle/modules/provenance.json` naming the Slang toolchain that produced them.
- When `workspace.bundle.source` is enabled (the default), copies exported `.slang` files into
  `build/bundle/source/` at those same import-relative paths so the directory is one search path.
- Compiles each name in `host.executables` to `build/<name>` and copies `slang-rt` beside it.
- Copies Markdown from each package's `docs/` into `build/docs/<package>/` and writes
  `build/docs/index.md`.

The `build/bundle/modules` tree is enough for a consumer that should not receive `deps/` source:
put that directory on the search path. The matching source layout is `build/bundle/source/`.

```sh
slang package run
```

Run executes the existing `host.default` artifact (`video-preview`) and does not compile. If the
binary is missing, it tells you to build first. A leading argument that matches a listed
executable name selects that artifact; remaining arguments are forwarded.

```sh
slang package docs
less build/docs/index.md
```

`docs` prints the generated directory; it does not copy or regenerate files. Run `build` when the
documentation should change. `slang package test` is reserved and not implemented yet.

## Develop against a local tree

Two local mechanisms, both recorded in gitignored `slang-workspace.json`:

`slang package edit NAME` keeps the published Git pin in the lock and stops treating
`deps/NAME` as replaceable tool-owned state. Fetch and update will not overwrite that checkout.
Changing its exports or dependencies still requires a new published tag and a normal `update`.
`unedit` refuses while the tree has extra commits, dirty files, or stashes.

`slang package override NAME PATH [AS]` points the package at another directory you already have.
Its effective version must satisfy every incoming dependency constraint. Omit `AS` to retain the
version from the current lock, or provide it when the local tree represents a different version.
Its current manifest participates in resolution only when you run:

```sh
slang package update --from-local
```

`--from-local --dry-run` prints the lock diff without writing it. The resulting lock records the
local path plus the original Git identity, so another machine or CI fails unless it has the same
`slang-workspace.json`. Restore a portable Git pin with a normal `update` before you `unoverride`
or commit.

Do not commit `slang-workspace.json`. Path dependencies in `slang-package.json` are the published
way to vendor a tree; overrides are the laptop way to redirect one.

## Retractions and excludes

These look similar and are not interchangeable.

**Publisher retractions** live in the tagged `slang-package.json` of the package that published
the bad release. The tool reads them from the highest available Git tag, even outside your
requested range. They skip matching Git candidates on `update`. They do **not** invalidate an
existing lock, so `fetch` in CI stays reproducible after the publisher adds advice.

**Workspace excludes** live in the root manifest's `workspace.excludes` array. They are committed
consumer policy for *this* workspace. Nested packages' `workspace` objects are ignored for the
solve. If a dependency still lists excludes this workspace did not copy, the tool warns; copy the
entry here if this project should skip that Git release too. Resolution skips excluded Git tags,
and `fetch` **rejects** a lock that still selects one: the lock is stale
relative to declared intent, so you must `update`. Path packages and overrides are local
selections, so remote release exclusions do not filter them even though they carry an effective
version for solver compatibility.

There is no personal exclude in `slang-workspace.json`. A machine-local skip that failed `fetch`
would make CI and your laptop disagree about the same lock. Use a committed exclude when the
whole project must avoid a release, or an override when you need a different tree on this machine.

Git-to-Git remapping is not available yet. Overrides replace a dependency with a local path only.

## What CI should run

```sh
slang package fetch
slang package status
slang package build
```

CI should not run `update`. Update is a deliberate choice to take newer tags and rewrite the
committed lock. After you have reviewed `update --dry-run` locally, commit the new lock and let
CI fetch it.
