#pragma once

#include "package-types.h"

namespace Slang
{
namespace PackageTool
{

enum class UneditMode
{
    Plain,
    Advance,
    Restore,
    Tag,
};

/// Set the lock pin. A version is required when `name` is not edited, and rejected when it is.
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
/// reset. The row keeps its version and its pin flag, and replaces the commit with the branch.
SlangResult beginPackageEdit(
    const String& projectRoot,
    const String& name,
    const String& branch,
    bool create,
    String& outError);

/// Move an edit's stored version to the greatest newer canonical tag on its restore line.
///
/// The checkout is not moved. When no such tag satisfies the incoming constraints, the row is
/// left unchanged.
SlangResult advancePackageEdit(const String& projectRoot, const String& name, String& outError);

/// End an edit on a release row. Declining a prompt leaves the edit in place.
SlangResult endPackageEdit(
    const String& projectRoot,
    const String& name,
    UneditMode mode,
    const String& tagVersion,
    bool clean,
    String& outError);

} // namespace PackageTool
} // namespace Slang
