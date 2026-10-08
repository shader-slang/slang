# Source package edits

This note describes the packaging prototype. Every dependency is a Git repository checked out at `deps/<name>`. There is no path dependency and no overlay that points a package at another directory.

The story is one resolved record per dependency. A release record says which commit to check out. A pin is a boolean on that record: while it is set, `update` does not move the version. An edit record says which branch to check out. The boolean stays set through the edit, and the edit is how that version is allowed to move. `unedit` is the only way back from a branch record to a commit record, and it has to land on a real release tag. The boolean is left as it was.

## What the workspace records

Resolved dependencies live in `slang-package-lock.json`. Each dependency keeps the Git URL already chosen for it. The fields that change between a release and an edit are the locator.

A dependency that was solved as a release stores:

- the version number
- the commit hash

`fetch` checks out that commit. A tag that later moves does not change an existing release row.

A dependency that is being edited stores:

- the version number, its canonical tag, and that tag's commit
- the branch name

The branch name is the branch the checkout is expected to be on. The version, tag, and commit are the release that branch is representing. `fetch` checks out a release row's commit and leaves an edited checkout on its branch. `update` does not fetch, merge, reset, or otherwise change an edited checkout: not its branch, `HEAD`, files, recorded version, or pin boolean. It reads the tree as it exists and solves the transitive dependencies declared by that manifest. The developer moves the branch with Git. When `HEAD` is not on the recorded branch, `update` reports the mismatch and leaves the checkout alone. `fetch` does not replace an edited checkout either. Two machines that share an edited lock can therefore see different trees. `validate` rejects a workspace that still has an edit, and an edited lock is not something to commit. The pin boolean is separate from the branch. A pinned dependency that is not edited still stores a version and a commit, and that row can be committed.

The version stored on an edit is the release that row is representing. It starts as the release that was current when the edit began. `edit advance` is what moves it, and that move replaces the version, the tag, and the commit together. `update` does not replace it with a newer tag that later appears on the branch, whether or not the pin boolean is set. The source and the manifest come from the checkout as it exists, which may be commits ahead of that version. The solver keeps matching constraints against the recorded version, so a tag created during the edit does not change which release the rest of the graph thinks it selected. The boolean matters again after `unedit`: a set boolean keeps the version `unedit` landed on, and a clear boolean lets the next `update` drift.

## Pinning a version

A pin is a boolean stored on that dependency's row in `slang-package-lock.json`. It is not a field in `slang-package.json`. Committing the lock commits the pin, so the next `update` on another machine keeps the same version. The manifest range is left as it was. The boolean does not check out a branch, and it does not by itself change which commit is checked out. `update` reads it from the existing lock and will not move a pinned version to a newer tag. `edit advance` and `unedit` will, and they leave the boolean set.

```text
slang package pin <name> <version>
slang package pin <name>
slang package unpin <name>
```

`<version>` is an exact dotted version, and it is required when the dependency is not edited. The command resolves the canonical tag for that release, such as `v1.2` for `1.2` or `1.2.0`, sets the boolean, and stores that version plus the tag's commit. `fetch` checks out the commit. The version has to satisfy every incoming constraint. When it does not, `pin` fails and leaves the row unchanged. When no canonical tag for that release exists, `pin` fails the same way.

While the dependency is edited, `pin <name>` sets the boolean and leaves the version and the branch alone. Passing `<version>` in that state fails. Moving the version is `edit advance`, or an ending of `unedit`.

`unpin` clears the boolean and works during an edit. The branch stays checked out. After `unedit`, the next `update` may select a different tag. Other dependencies still solve normally either way.

`edit` and `unedit` do not change the boolean. Editing a pinned dependency starts the branch at the pinned commit and keeps that version until an explicit move. Editing a dependency that is not pinned does not set the boolean, so leaving the edit returns the package to ordinary solving.

Canonical tags are the same spelling the solver already accepts: `v` plus the version with no leading zeros and no trailing zero components. `v1.2` counts for release 1.2. `v1.2.0` does not.

## Starting an edit

`edit` applies to a dependency that is already resolved, because leaving the edit has to be able to restore that resolution.

```text
slang package edit <name> --branch <branch>
slang package edit <name> --branch <branch> --create
```

The branch name is required. Without `--create`, the branch must already exist and `edit` checks it out in `deps/<name>`. With `--create`, a missing branch is created from the commit the dependency is resolved to. `--create` does not reset a branch that already exists.

`edit` then solves. The lock row keeps the version, tag, and commit it already had, and records the branch beside them. There is no disabled state and no separate directory. Overrides and other path substitutions are not part of this model.

## Moving the frozen version

A newer tag on the branch does not become the stored version by itself. A separate command does that, and only when asked:

```text
slang package edit advance <name>
```

The command looks for canonical release tags on one line of history: from the branch tip back to the commit of the release the row is representing. The stop point is that saved commit, not whatever commit currently carries the same version tag. One `edit advance` stores the latest tag that walk can adopt: its version, its tag name, and its commit. The next walk stops at that new commit. It does not stop at the first tag after the represented release.

A merge commit records its parents in order. The first parent is the commit that was checked out when the merge was created, and each later parent is a line that was merged in. The walk uses that record. At a merge it asks, for each parent, whether the saved pin commit is an ancestor of that parent. A parent that does not contain the pin is a line that cannot lead back to the edit's starting point, and the walk does not enter it. When more than one parent contains the pin, which is the usual case when `main` also grew from that same release, the walk takes the first parent. The tags on that walk are considered together. The one written is the greatest canonical tag newer than the represented version that still satisfies the dependency constraints. If `v1.3` and `v1.4` are both on the line and both are legal, the row becomes `1.4` in that single command. The branch name stays, and the checkout does not move. When no such tag exists, or when the walk reaches a root without visiting the represented commit, the command says so and leaves the row unchanged.

Consider a branch created at the `v1.2` commit, then updated by merging `main`:

```text
v1.2 -- fix -- M (tip)
         \    /
          v1.4
```

`M` lists `fix` as its first parent and the `v1.4` commit as its second. Both parents contain the `v1.2` commit, so the ancestor test does not separate them. Parent order does: the walk takes `fix` and never visits `v1.4`. A tag created on `fix` after `v1.2` is the one `edit advance` can adopt. The same merge performed the other way around, with `main` checked out, would record `v1.4` as the first parent, and the walk would see `v1.4` on its way back to `v1.2`. The result follows whichever line was current at the merge.

`update` does not run this, and it does not move the checkout. An edit keeps claiming the release that was selected at freeze time until this command or `unedit` says otherwise.

## What an edit must remember

The edit row's locator is the branch. The release it is representing is the version, the canonical tag, and that tag's commit. Those three stay together: `edit advance` moves all of them, and `unedit --restore` returns to that same commit. There is no second, older release kept from the start of the edit. `fetch` does not check the represented commit out.

## Leaving an edit

`unedit` must end on a release row: a version and a commit. It does not leave the dependency on the branch, and it does not succeed by forgetting the branch and keeping the working tree as it happens to be. Declining a prompt leaves the edit in place. There is no `--yes`. An ending that needs approval asks, and a run without a terminal fails and leaves the edit in place.

```text
slang package unedit <name> [--restore | --advance | --tag <version>] [--clean]
```

Plain `unedit <name>` follows the endings below. `--advance`, `--restore`, and `--tag` select one ending and cannot be combined. None of them changes the pin boolean. When the boolean is set, the version this command lands on stays pinned, and the next `update` does not drift. When it is clear, the row is an ordinary solved release and that `update` may move it. Set or clear the boolean with `pin` and `unpin` before leaving the edit.

`--advance` selects the newer tag on the edit line. The version written is that tag's version. The command fails when no newer tag on that line satisfies the constraints. The checkout moves only when the tag is behind the tip, and that move asks, naming how many commits would leave the workspace.

`--restore` checks out the version and commit the edit is representing. After `edit advance`, that is the commit just advanced to. A newer legal tag is named and then left unused. This asks, because the branch's later commits leave the workspace.

`--tag <version>` creates a release tag on the commit currently checked out, then selects it the way `--advance` does. `<version>` is an exact dotted version. The tag name is `v` plus the canonical spelling, so `1.4.0` creates `v1.4`. The command fails before creating a tag when any of these is true: the release already has a tag, the version is not strictly greater than the version stored on the edit, the version is not strictly greater than every canonical tag already on the edit line, or the version does not satisfy the dependency constraints. The last of those keeps `--advance` from selecting some older tag and ignoring the one just created. The tag points at the checked-out commit, so the advance that follows selects it without moving `HEAD`. The tag is created locally and is not pushed.

`--clean` discards uncommitted changes and stashes so the checkout is allowed to move. It does not approve dropping commits that follow a tag. Those commits remain the question the prompt asks.

The command looks for a canonical release tag on the same line of history `edit advance` uses: from the tip back to the represented commit, staying on the parent that still reaches that commit. The candidate is the greatest such tag that is strictly newer than the represented version and that still satisfies the dependency constraints. A tag that arrived only through a merge of some other line is not a candidate.

**The newer tag is the commit that is checked out.** The lock switches to that tag's version and that commit. The checkout does not move.

**The newer tag is behind the checked-out commit.** The command tells the user how many commits on the branch are after the tag and would no longer be in the workspace, and asks whether to check out the tag and select it. Approving writes that version and that tag's commit. Declining does not fall through to the older release. The newer work stays checked out, and the dependency stays edited.

**A newer tag exists and violates the constraints.** It is not a legal selection. The command says which tag was rejected and why, then offers only the release the edit is already representing.

**No newer satisfying tag exists.** The only ending is that same represented release. The checkout returns to its commit, which discards the branch's later commits from the workspace. This requires an explicit approval even when the checkout is already on that commit, because the lock stops tracking the branch.

A dirty checkout blocks any ending that would move `HEAD`. `unedit` refuses that move until the user has committed the local changes or passed `--clean`.

## Example

`noise` is pinned at version `1.2`, commit `abc`, which is the tag `v1.2`. The constraint is `>=1.2 <2`.

`slang package pin noise 1.2` sets the boolean and holds the solver on `1.2` at commit `abc`. A later tag `v1.3` that also satisfies `<2` is not selected by `update`. `unpin` clears the boolean and lets the next `update` select it.

`slang package edit noise --branch fix-hash --create` creates `fix-hash` at `abc` and leaves the boolean set. The row still represents version `1.2`, tag `v1.2`, commit `abc`, and its locator is `fix-hash`. That stays true after `v1.4` is tagged on the branch, until `edit advance`. `update` still solves `noise` as `1.2` and leaves the `fix-hash` checkout where the developer put it.

`slang package edit advance noise` moves the represented release to `1.4`: the version, the tag, and that tag's commit. Until then the graph does not see `1.4`. `--restore` after that advance returns to the `1.4` commit, not to `abc`.

`unedit` before that advance sees that `v1.4` is newer than `1.2`, satisfies `<2`, and, when the tip is that tag, switches the lock to version `1.4` and that commit. No prompt. That adoption happens because the edit is ending, not because `update` noticed the tag. The boolean is still set, so a later `update` keeps `1.4`. The same `unedit` after `unpin` lands on `1.4` and lets that `update` drift.

If the tip is three commits after `v1.4` and the row still represents `1.2`, `unedit` asks whether to select `v1.4` and ignore those three commits. No leaves `fix-hash` in place. Yes checks out the tag and stores version `1.4` plus the tag's commit. The boolean is unchanged.

If the only tag on the branch is still `v1.2`, `unedit` can only return to commit `abc`, and only with approval. `unedit noise --tag 1.3` instead creates `v1.3` on the checked-out commit and selects that commit as version `1.3`, because `1.3` is unused, greater than `1.2`, and inside `<2`. A boolean that was set stays set on `1.3`.

If the branch is tagged `v2` at the tip, `v2` does not satisfy `<2`. `unedit` says so and offers the return to `1.2` at `abc`.

## Every dependency is a Git repository

A dependency has a Git URL and is checked out at `deps/<name>`. There is no `path` dependency and no `as` version standing in for one. `dependency add --path` goes away. There is also no override that points a package at another directory, another checkout, or another machine-local tree. One package name has one Git URL and one checkout directory. An edit changes the branch of that checkout. It does not change where the package lives.
