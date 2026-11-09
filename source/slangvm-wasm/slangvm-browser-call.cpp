#include "slangvm-browser-call.h"

#include <emscripten.h>

using emscripten::val;
using emscripten::EM_VAL;

// This import itself is synchronous: a JSPI Suspending import yields even for plain results.
// Return a Promise as an ordinary emval handle and await it explicitly below when necessary.
// clang-format off
EM_JS(EM_VAL, slangvmCallHost, (EM_VAL hostHandle, const char* method, EM_VAL argumentsHandle), {
    const failed = error => ({error: String(error?.message ?? error)});
    try {
        const host = Emval.toValue(hostHandle);
        const result = host[UTF8ToString(method)](...Emval.toValue(argumentsHandle));
        return Emval.toHandle(result instanceof Promise ? result.catch(failed) : result);
    } catch (error) {
        return Emval.toHandle(failed(error));
    }
});
// clang-format on

namespace Slang::HGVM
{
val callBrowserHost(const val& host, const char* method, const val& arguments)
{
    auto result =
        val::take_ownership(slangvmCallHost(host.as_handle(), method, arguments.as_handle()));
    return result.instanceof(val::global("Promise")) ? result.await() : result;
}

bool getBrowserError(const val& result, String& outError)
{
    if (!result.isNull() && !result.isUndefined() &&
        result.typeOf().as<std::string>() == "object" && result.hasOwnProperty("error"))
    {
        outError = result["error"].as<std::string>().c_str();
        return true;
    }
    return false;
}
} // namespace Slang::HGVM
