---
layout: user-guide
permalink: /user-guide/source-package-workflow
---

Using Source Packages
=====================

This chapter is a command walkthrough. It shows _when_ to run each `slang package` command and
what you should see. Manifest fields, lock format, and validation rules are in
[Slang Source Packages](source-packages). Human package-growth journeys and must-succeed /
must-fail contracts are in
[Growing an Application with Source Packages](source-package-command-use-cases). Module file
naming is in [Writing Module Files, Import, and Include](module-files).

The short form `slang pkg` accepts the same commands. Slang uses a single dash for
multi-character options, for example `-help`, not `--help`, except for `slang package` flags such
as `--dry-run` and `--ignore-overrides`.

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

Clone the workspace. Commands that load the three JSON files (`slang-package.json`,
`slang-package-lock.json`, and `slang-package-overlay.json`) also work from ordinary subdirectories,
such as `src/`. They use the nearest ancestor that contains `slang-package.json`. Nested packages
under `deps/` keep their own root when they have `slang-package.json`.

```sh
git clone https://github.com/jhelferty-nv/video-preview.git
cd video-preview
```

The committed lock selects `v1.0.0` for every Git dependency. The same repositories also publish
`v1.1.0`. `color-encoding` `v1.1.0` retracts `1.0.0` because that release used truncated luma
weights. That retraction is what makes `fetch` versus `update` interesting: fetch still uses the
lock; update will not reselect the retracted tag.

The workspace lists `video-preview` under `build.host.executables`. The matching primary is
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
  builtins and standard library), optional `build.host` executables, optional publisher `retractions`,
  and optional root-only `workspace` settings (`deps`, `build`, `excludes`).
- `slang-package-lock.json` is the exact selection this workspace resolved: identity only (`version`,
  Git `ref`/`commit`, overlay `path`). `fetch` reproduces those pins without solving again.
  Declared exports and dependencies are reloaded from overlay working trees or from each locked
  version's manifest.
- `slang-package-overlay.json` is gitignored machine-local override state. `edit NAME` creates an
  override at `deps/NAME`; other overrides may point elsewhere. The file should not be in the
  clone. If it is missing, that is correct for CI and for a clean checkout.

`slang package help` groups commands under those three files, then build, in that pipeline
order: manifest, overlay, lock.

`build` is a sibling of `workspace` in the manifest. `workspace.build` names the output directory;
`build.host` configures native executables. A dependency may declare `build.host`; only the package
you run `build` and `run` in produces executables.

Most Git dependencies use `git` plus a `version` range, written with `slang package dependency
add NAME --git URL --version RANGE`. To pin a branch, tag, or full 40-character commit, use
`--git URL --ref REF --as VERSION`; `ref` is Git identity and `as` is the exact solver version.
Path dependencies use `--path PATH --as VERSION`. After a lock exists,
`slang package dependency pin NAME` copies that selection into a direct Git exact-version edge.
`--to MAJOR.MINOR.PATCH` writes a different exact version. `--commit` writes the locked SHA as
`ref`; it requires a Git-only lock row because an overlay lock row has no locked SHA. Pin leaves an
active overlay registered, but once the lock selects that overlay, pass `--to` explicitly because
its effective version need not be published in Git. The lock records selection identity: the
effective `version`, and for Git pins the resolved `ref` and `commit`.

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

Use fetch to materialize a committed lock without building. `build` also fetches missing locked
trees and, when there is no lock, runs fetch then update `--yes` so a first clone can bundle
without a prompt. Pass `--clean` only on fetch or update when you intend to replace a dirty or
unowned checkout; build does not accept `--clean`.

Fetch and update never discard local work as a side effect. Both inspect the checkouts the current
lock owns before doing anything else, and stop with an error naming each one that holds local
state: uncommitted files, stashes, a different `HEAD`, a different origin, or a directory that is
no longer a Git checkout. Update stops there before it even resolves the graph, so it reports no
plan and rewrites no lock. Enabled overrides are exempt because those trees are yours and are
never replaced. The three ways forward are to commit or discard the changes, run
`slang package edit NAME` to keep working in that checkout, or re-run with `--clean` to discard
the local state and restore the locked commit.

If dependencies exist but the lock is missing, fetch performs the initial solve, prints the
selection report, and asks before writing the first lock. Pass `--yes` in a non-interactive
checkout. Once a lock exists, fetch reproduces it and never reselects versions.

## Check workspace consistency

```sh
slang package status
```

After a successful fetch, status should be one line: the package name, that the lock is current,
and that the workspace is buildable. Extra lines appear only for drift such as a missing lock,
missing or dirty checkouts, edits, or enabled overrides. Status does not inspect `build/`,
fetch, update, or contact remotes. When it finds drift, it lists the problems and names the
corrective command without returning a failure merely because the workspace is dirty or incomplete.

If you run status before fetch, the header says the lock is absent and the graph is incomplete,
and the following lines name the missing lock or checkouts. Status returns nonzero only when
required JSON cannot be read and parsed.

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

Update prints the report again and asks whether to apply that exact in-memory selection. The
report is a plan, so it is written in the future tense (`would upgrade`, `Would update 3
packages`); the past-tense summary is printed only after the lock and the checkouts it names have
actually been written. Use `slang package update --yes` for automation. All three should now
print `v1.1.0`. Convert's tighter
encoding range (`>=1.1.0`) and the
publisher retraction of `1.0.0` agree: the shared leaf is `color-encoding@v1.1.0`, once, in the
lock.

Before clearing search paths or writing `deps/`, update checks that the selected graph is legal:
identities, trusted edges, toolchain, and exclusions, reading each Git manifest at the selected
commit from `.slang/cache`, or from `deps/NAME` when that checkout already holds the commit. After
materializing that whole graph, it checks the license, exports, and module layout of each new or
changed Git checkout and each changed local registration, and checks module layout and import
uniqueness across the complete graph. The new lock and successful resolution report are written
only after those checks pass. Fetch applies the same
two stages while reproducing the existing lock. Unchanged dependencies are still part of
closure-wide buildability, but their publish checks are not repeated. `--skip-validate` skips only
the post-materialize source-layout and publish checks.

`v1.1.0` of the preview prints full-precision luma weights `(0.2126, 0.7152, 0.0722)` instead of
the truncated `(0.2130, 0.7150, 0.0720)` from `v1.0.0`.

Update the entire graph when you mean to take newer compatible releases. There is no
package-specific update mode yet.

## Build, run, and collect docs

```sh
slang package --experimental build
```

This example uses experimental build because its manifest configures `build.host.executables` and the
walkthrough demonstrates `.slang-module` output. A stable `slang package build` distributes the
source bundle and docs only; binary module generation and host executable compilation are
experimental. Source interpretation with `run` is stable.

Build checks that the materialized graph is legal and buildable, without requiring publish
licenses or a portable workspace, then:

- When `workspace.bundle.modules` is enabled (the default), emits a `.slang-module` for every
  primary in the workspace and its dependencies under `build/bundle/modules/`, preserving
  import-relative paths (`video-preview`, `video/display`, `color/convert`, `color/encoding`), and
  writes `build/bundle/modules/provenance.json` naming the Slang version, source commit, and
  tracked-source dirty state that produced them. Build warns that this binary format is unstable
  and experimental.
- When `workspace.bundle.source` is enabled (the default), copies exported `.slang` files into
  `build/bundle/source/` at those same import-relative paths so the directory is one search path.
- Compiles each name in `build.host.executables` to `build/host/<name>`, copies `slang-rt` beside it, and
  writes `build/host/EXPERIMENTAL.txt`.
- Copies Markdown from each package's `docs/` into `build/docs/<package>/` and writes
  `build/docs/index.md`.

The `build/bundle/modules` tree can serve a consumer that should not receive `deps/` source only
when it uses the exact toolchain recorded in provenance; the binary format has no stability
guarantee. The stable distribution layout is `build/bundle/source/`.

```sh
slang package run
```

Run asks sibling `slangi` to interpret the existing `build.host.default` source
(`build/bundle/source/video-preview.slang`) and does not build first. If the source bundle is
missing, it tells you to build first. A leading argument that matches a listed executable name
selects that primary; remaining arguments are forwarded. To run the experimental native artifact
instead:

```sh
slang package --experimental run --binary
```

```sh
slang package docs
```

`docs` opens `build/docs/index.md` with the registered Markdown application. Pass `--print` to
write the path instead of launching. It does not copy or regenerate files; run `build` when the
documentation should change. `slang package test` is reserved and not implemented yet.

## Develop against a local tree

Both commands use overrides recorded in gitignored `slang-package-overlay.json`.

`slang package edit NAME` registers `deps/NAME` as an enabled in-place override at the version in
the current lock. The checkout may already contain local changes: preserving work you have already
started is the point of the command. Fetch and update will not overwrite it. Search paths already
point at that directory, so you can keep compiling without `update`. A plain update reads its
working-tree manifest, so new dependency edges enter the solve and the resulting lock records
both the Git identity and local path.

Plain `unedit` succeeds only when the lock is a Git pin and the checkout is clean at that exact
commit. `unedit NAME --clean` discards local state and restores that commit; pass `--yes` when
confirmation cannot be interactive. To keep a committed fix, use
`unedit NAME --adopt [--as VERSION]`: this pins the direct dependency in the manifest and writes a
Git-only lock at `HEAD`. It replaces the dependency's version range with canonical `ref` plus `as`
intent. A unique `vMAJOR.MINOR.PATCH` tag at `HEAD` supplies the version; otherwise `--as` is
required.

`slang package override add NAME PATH [AS]` points the package at a directory you already have.
`PATH=deps/NAME` updates the same in-place registration created by `edit`; another path creates an
out-of-tree override. Its effective version must satisfy every incoming dependency constraint.
Omit `AS` to retain the version from the current lock, or provide it when the local tree represents
a different version. An enabled override's current manifest participates in every plain update:

```sh
slang package update
```

Use `override disable NAME` to retain its path and version while selecting the published graph,
then `override enable NAME` to switch back. `override list` shows both states.
`update --ignore-overrides` solves from Git for this command only, without disabling the
registrations. In-place overrides stay active so those checkouts are not replaced, including
packages that drop out of the published graph. A later plain update restores them. A dry run
prints the lock diff without writing it. The resulting lock records the
local path plus the original Git identity, so another machine or CI fails unless it has the same
`slang-package-overlay.json`. Disable the override and run `update` to restore a portable Git pin before
you remove the registration or commit.

`slang package validate NAME` checks that package's tree against this workspace lock, so
you can certify a library using an in-place override before a remote tag exists. Bare `validate`
still rejects the workspace while any override is enabled.

Do not commit `slang-package-overlay.json` or `slang-package-includes.txt`. Path dependencies in `slang-package.json` are in-package
vendoring, not extract. Overrides are the laptop way to redirect one identity at a local tree.

## Extract a package from this application

There is no `slang package extract` command. Create a sidecar package by hand, then point this
workspace at that one tree.

A sibling directory is the recommended layout (`../color-math` next to `image-viewer`). `deps/NAME`
is gitignored, so it is only a laptop override: the inner tree is not in the application
repository until that sidecar git has a remote.

Wire it with a declared Git identity plus an override so the application uses **one** tree:

```sh
slang package dependency add color-math --git https://example.com/color-math.git --version ">=1.0.0 <2.0.0"
slang package override add color-math ../color-math 1.0.0
slang package update
```

Do not `dependency add --git` that same sidecar path without an override. Fetch would clone the
path onto itself under `deps/NAME`. Path dependencies in the root manifest vendor a tree that
lives in this repository; they are not extract.

`slang package validate color-math` and root `build` use this workspace's lock pins, not a nested
lock under the sidecar. When the sidecar has an origin, push tags, keep `git` pointed at that
origin, disable the override, and `update` so the lock records the published pin.

## Retractions and excludes

These look similar and are not interchangeable.

**Publisher retractions** live in the tagged `slang-package.json` of the package that published
the bad release. The tool reads them from the highest available Git tag, even outside your
requested range. They skip matching Git candidates on `update`. They do **not** invalidate an
existing lock, so `fetch` in CI stays reproducible after the publisher adds advice.

**Workspace excludes** live in the root manifest's `workspace.excludes` array. They are committed
consumer policy for _this_ workspace. Nested packages' `workspace` objects are ignored for the
solve. If a dependency still lists excludes this workspace did not copy, the tool warns; copy the
entry here if this project should skip that Git release too. Resolution skips excluded Git tags,
and `fetch` **rejects** a lock that still selects one: the lock is stale
relative to declared intent, so you must `update`. Path packages and overrides are local
selections, so remote release exclusions do not filter them even though they carry an effective
version for solver compatibility.

There is no personal exclude in `slang-package-overlay.json`. A machine-local skip that failed `fetch`
would make CI and your laptop disagree about the same lock. Use a committed exclude when the
whole project must avoid a release, or an override when you need a different tree on this machine.

Git-to-Git remapping is not available yet. Overrides replace a dependency with a local path only.

## What CI should run

```sh
slang package fetch
slang package status
slang package --experimental build
```

`fetch` is optional when CI only needs a bundle: `build` fetches any missing locked trees itself.
A first clone with no lock also works: `build` runs fetch, which runs update `--yes` and writes
the first lock. An existing lock is never rewritten. Keep an explicit `fetch` when you want
materialization without building, or when you need `--clean`.

Drop `--experimental` when CI only needs the stable source bundle and documentation. Keep it only
when CI deliberately tests unstable module or host outputs.

CI should not run `update`. Update is a deliberate choice to take newer tags and rewrite the
committed lock. After you have reviewed `update --dry-run` locally, commit the new lock and let
CI fetch it.
