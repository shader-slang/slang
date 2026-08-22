// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SLANG_PACKAGE_JSON_H
#define SLANG_PACKAGE_JSON_H

#include "package-types.h"

namespace Slang
{
namespace PackageTool
{

bool isValidPackageName(const String& name);

SlangResult readManifest(const String& path, Manifest& outManifest, String& outError);
SlangResult readManifestText(
    const String& sourceName,
    const String& text,
    Manifest& outManifest,
    String& outError);
SlangResult writeManifest(const String& path, const Manifest& manifest, String& outError);

SlangResult readLockFile(const String& path, LockFile& outLock, String& outError);
SlangResult writeLockFile(const String& path, const LockFile& lock, String& outError);

} // namespace PackageTool
} // namespace Slang

#endif
