#pragma once

#include "package-types.h"

namespace Slang
{
namespace PackageTool
{

enum class UneditMode
{
    Plain,
    Abort,
    Tag,
};

/// Set the lock pin. A version is required when `name` is not edited, and rejected when it is.
///
/// Pinning a version rewrites only that row; the next `update` reconciles the rest of the lock.
/// When that `update` would change other packages because of the pin, or fail, the command says so
/// and asks first.
SlangResult pinLockedPackage(
    const String& projectRoot,
    const String& name,
    const String& version,
    bool hasVersion,
    String& outError);

/// Clear the lock pin. The branch of an edit stays checked out.
SlangResult unpinLockedPackage(const String& projectRoot, const String& name, String& outError);

/// Check out `branch` for a resolved dependency and record the edit in the lock.
///
/// Without `create`, `branch` must already exist. With `create`, a missing branch is created at
/// the commit the lock currently records, and an existing branch is checked out without being
/// reset. The row keeps its version, tag, commit, and pin flag, and records the branch beside them.
/// The rest of the lock is not resolved; the next `update` reads the edited working tree.
SlangResult beginPackageEdit(
    const String& projectRoot,
    const String& name,
    const String& branch,
    bool create,
    String& outError);

/// End an edit on a release row without resolving the rest of the lock.
///
/// `Plain` is accepted only when the edit has no commits and no uncommitted files, so ending it
/// cannot lose work. `Abort` returns the checkout to the release the edit started from; it refuses
/// uncommitted files and leaves branch commits on the branch. `Tag` creates `tagVersion` on the
/// checked-out commit after warning, and asking, when a later `update` would not keep it.
/// Declining a prompt leaves the edit in place. The next `update` reconciles the other rows.
SlangResult endPackageEdit(
    const String& projectRoot,
    const String& name,
    UneditMode mode,
    const String& tagVersion,
    String& outError);

} // namespace PackageTool
} // namespace Slang
