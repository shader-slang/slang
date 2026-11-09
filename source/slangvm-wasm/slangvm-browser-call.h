#pragma once

#include "core/slang-string.h"

#include <emscripten/val.h>

namespace Slang::HGVM
{
/// Calls a browser service immediately and suspends only when it returns a Promise. JavaScript
/// failures become error objects so the caller can return a SlangResult and unwind native owners.
emscripten::val callBrowserHost(
    const emscripten::val& host,
    const char* method,
    const emscripten::val& arguments);

/// Extracts a failed browser call's diagnostic; ordinary service results are left untouched.
bool getBrowserError(const emscripten::val& result, String& outError);
} // namespace Slang::HGVM
