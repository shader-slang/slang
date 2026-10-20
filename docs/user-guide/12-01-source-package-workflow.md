---
layout: user-guide
permalink: /user-guide/source-package-workflow
---

Using Source Packages
=====================

This chapter is a command walkthrough. It shows _when_ to run each `slang package` command and
what you should see. Manifest fields, lock format, and validation rules are in
[Slang Source Packages](source-packages). The same commands, followed from an empty directory
through publishing and editing, are in
[Growing an Application with Source Packages](source-package-command-use-cases). Module file
naming is in [Writing Module Files, Import, and Include](module-files). Every option is in
[Slang Package Command Reference](source-package-command-reference).

The short form `slang pkg` accepts the same commands. Slang uses a single dash for
multi-character options, for example `-help`, not `--help`, except for `slang package` flags such
as `--dry-run` and `--yes`.

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

Clone the workspace. Commands that load `slang-package.json` and `slang-package-lock.json` also
work from ordinary subdirectories, such as `src/`. They use the nearest ancestor that contains
`slang-package.json`. Nested packages under `deps/` keep their own root when they have
`slang-package.json`.

```sh
git clone https://github.com/jhelferty-nv/video-preview.git
cd video-preview
```

The committed lock selects `v1` for every Git dependency. The same repositories also publish
`v1.1`. `color-encoding` `v1.1` retracts `1.0.0` because that release used truncated luma
weights. The retraction names the release `1.0.0`; the canonical tag of that release is `v1`.
Fetch still uses the lock. Update will not reselect the retracted tag.

The workspace lists `video-preview` under `build.host.executables`. The matching primary is
`src/video-preview.slang`, which declares `module video_preview;`. There is no reserved
`main.slang` filename.

## Two files

Open the two committed files first:

```sh
less slang-package.json
less slang-package-lock.json
```

- `slang-package.json` is the intent you edit. Each direct dependency is a Git URL and a version
  range. There is no exact commit in this file.
- `slang-package-lock.json` is the exact graph `update` selected. A release row records `git`,
  `ref` (the canonical tag, such as `v1`), `commit`, and `version`. `pinned` appears only when
  that release is held. An edit row also records `branch`. Do not commit a lock while a row is
  edited.

`slang package dependency add NAME --git URL --version RANGE` writes the manifest only.
`slang package pin NAME VERSION` sets the lock boolean for a solved release.
`slang package edit NAME --branch BRANCH` checks out that branch in `deps/NAME` and records it
on the lock row.

Generated and machine-local paths are gitignored: `.slang/`, `deps/`, `out/`, and
`slang-package-includes.txt`. A `slang-package-overlay.json` that contains entries is an error.

## Reproduce the locked graph

`status` reads local files and does not contact remotes. On a fresh clone the checkouts are
missing, so the header says the graph is incomplete:

```sh
slang package status
```

`fetch` installs exactly the commits in the lock. It does not select newer tags, and it does not
apply the `v1.1` retraction. The first fetch clones into `.slang/repositories/` and checks out a
worktree for each package. Later fetches reuse those repositories.

```sh
slang package fetch
slang package status
```

The header should now say the lock is current and the workspace is buildable, with three
packages. Nothing under `deps/` should be listed as dirty.

The lock's `ref` is the tag. Fetch stores the remote tag at `refs/slang-cache/tags/v1` and, when
`refs/tags/v1` does not already point at a different commit, copies it to `refs/tags/v1`. That
copy is what `git describe --tags` and an unqualified `v1` resolve:

```sh
git -C deps/color-encoding describe --tags --exact-match
```

The line is `v1`. A local tag of the same name that points elsewhere is left alone, and fetch
fails until you rename or delete it, or until the remote tag is restored to that object.

`fetch` also writes `slang-package-includes.txt`, a gitignored list of absolute dependency export
directories. `slangc -include-source-list slang-package-includes.txt` uses that list. The file is
an input to the compiler, not something to commit.

Fetch stops when a tool-owned checkout is dirty, diverged, or no longer the commit the lock
names. It lists those checkouts and changes nothing there. Finish the work in one of three ways:
commit it on the dependency's branch, move the lock onto that branch with `edit`, or discard it
with `fetch --clean`. `--clean` needs confirmation because it throws away uncommitted files.
`--yes` answers that prompt for a script.

## Check workspace consistency

After fetch, `status` is the fast local check:

```sh
slang package status
```

A current workspace prints one line, for example
`Package 'video-preview': lock current, 3 packages, buildable.` Extra lines appear only for drift:
a missing checkout, a dirty checkout, an edit, or a source-layout problem. Like `git status`,
that drift does not by itself make the command fail. Status fails when the manifest or an
existing lock cannot be read, and when `slang-package-overlay.json` contains entries.

`status` does not look in `out/`. A missing bundle is not drift.

`slang package validate` is the stricter publish check. It rejects the placeholder license text
that `init` writes, and it rejects the workspace while any dependency is edited. `validate NAME`
checks that one locked package instead. `video-preview` is an application with a real license, so
after fetch:

```sh
slang package validate
```

## Preview a new solve, then apply it

`update --dry-run` resolves against current tags and prints the lock it would write. It may
refresh `.slang/repositories/`. It does not move `deps/` or replace the lock, and it cannot prove
that source it has not checked out satisfies the layout rules.

```sh
slang package update --dry-run
```

Every dependency moves from `v1` to `v1.1`. `color-encoding` `1` is retracted, so it cannot be
selected again. The preview is a proposal. Declining it, or leaving the command at `--dry-run`,
keeps the committed lock.

Apply that proposal:

```sh
slang package update
```

Review the printed graph and answer `y`. `--yes` is the same approval for a script. Afterward the
`color-encoding` tag is `refs/slang-cache/tags/v1.1`. Its luma weights are the exact Rec. 601
constants `0.299`, `0.587`, and `0.114`, replacing the truncated `0.30`, `0.59`, and `0.11` from
`v1`.

`--minimal` prints one line per package instead of the constraint rationale. `--offline` resolves
only from repositories already under `.slang/repositories/`.

## Bundle, run, and collect docs

`bundle` is the stable distribution step. It validates the graph, copies exported `.slang` files
to `out/bundle/source/`, and collects Markdown from each package's `docs/` into `out/docs/`.

```sh
slang package bundle
```

Compilation of `.slang-module` files and host executables is experimental. Pass `--experimental`
immediately before the command:

```sh
slang package --experimental build
slang package --experimental run
slang package docs
```

`build` refreshes the same source and documentation as `bundle`, then adds the binary artifacts.
`run` uses the existing bundle. It does not bundle or build first. `docs` opens
`out/docs/index.md` from the last bundle. `docs --print` prints that path instead, which is the
form to use from a script.

`slang package test` is reserved. It fails with a not-implemented diagnostic and does not run the
compiler's own test harness.

## Develop against a local tree

An edit is a branch checkout of a dependency that is already in the lock. The package stays at
`deps/NAME`. The lock keeps the release the branch is representing, and records the branch beside
it.

```sh
slang package edit color-encoding --branch luma-fix
```

`--create` creates that branch from the resolved commit when the branch does not already exist.
The checkout is detached until this command. After it, commit on `luma-fix` with ordinary Git
commands inside `deps/color-encoding`. `fetch` and `update` do not move an edited checkout.
`update` reads the working tree and solves whatever that manifest depends on.

`status` lists the edit. `validate color-encoding` can check that package before a remote tag
exists. Bare `validate` still rejects the workspace while any dependency is edited. The includes
file points at the edited tree; do not commit it.

`pin` and `unpin` set or clear the lock boolean. `update` will not move a pinned version. During an
edit, the pin decides whether the release the edit ends on is held afterwards; see
[How pins work](source-package-command-reference#how-pins-work).

`unedit` ends the edit on a release and never discards work:

- `unedit color-encoding --tag 1.2` creates a local annotated tag `v1.2` on `HEAD` and ends the
  edit on it. The tag is not pushed. When a later `update` would not keep `v1.2`, the command says
  why and asks first.
- `unedit color-encoding --abort` returns to the release the edit started from. Commits stay on
  `luma-fix`; uncommitted files block it.
- Plain `unedit color-encoding` is accepted only when the edit has no changes.

`unedit` does not resolve the rest of the lock. Run `update` afterwards to reconcile it. The
details are in the [command reference](source-package-command-reference#unedit).

Push `luma-fix` yourself when you want it on a remote. After `unedit`, commit the resulting
release row if it should be what the next clone fetches.

## Extract a package from this application

A dependency is a published Git repository. To move source out of `video-preview`, put it in its
own repository, give that repository a `slang-package.json`, and publish a canonical tag (`v1`
for release `1`). From the application:

```sh
slang package dependency add color-math --git https://github.com/example/color-math.git --version ^1
slang package update
```

Delete the copied files from the application's export. Two packages must not declare the same
primary module. `slang package validate color-math` checks the locked tree. Bare `validate` checks
the application, which is rejected while its license is still the `init` placeholder or while any
dependency is edited. Publish the repository before the application can depend on it.

## Retractions and excludes

A retraction is publisher advice in the package that owns the bad release. `color-encoding`
`v1.1` retracts `1.0.0`. The next `update` avoids that release. An application that already
locked `v1` keeps it until someone runs `update`: `fetch` and `status` do not change the lock.

An exclude is consumer policy in the workspace manifest. It removes matching releases from
consideration. Use it when the whole project must avoid a release.

```json
"excludes": [
    {
        "package": "color-encoding",
        "version": "1.0.0",
        "reason": "Truncated luma weights"
    }
]
```

`update --remap-urls URL` is how a workspace resolves listed packages from a different Git host.
The URL is an `http` or `https` package index. Manifest URLs stay as published. A pin keeps its
version and is re-resolved on the new remote. An edited package whose URL would change fails
before the prompt; `unedit` it first.

## What CI should run

CI reproduces the committed lock. It does not take newer tags.

```sh
slang package fetch
slang package status
slang package bundle
```

`fetch` is required. `status` is the readable check that the lock matches the tree. `bundle`
checks source layout and writes the source distribution. A compile of one file can use the
generated includes list:

```sh
slangc -include-source-list slang-package-includes.txt src/video-preview.slang
```

`bundle` already runs `fetch` when a locked checkout is missing, and that fetch runs
`update --yes` when dependencies exist and the lock does not. Call `fetch` yourself when you want
materialization without distributing, or when you need `--clean`.

Use `slang package --experimental build` only when CI deliberately tests unstable module or host
outputs. That command refreshes the same source and documentation as `bundle`, then adds the
binary artifacts.

CI should not run `update`. Update is a deliberate choice to take newer tags and rewrite the
committed lock. After you have reviewed `update --dry-run` locally, commit the new lock and let
CI fetch it.
