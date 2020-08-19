
#include "slang-test-tool-util.h"

#include "../../slang-com-helper.h"

#include "slang-io.h"
#include "slang-string-util.h"

namespace Slang
{

/* static */ToolReturnCode TestToolUtil::getReturnCode(SlangResult res)
{
    switch (res)
    {
        case SLANG_OK:              return ToolReturnCode::Success;
        case SLANG_E_INTERNAL_FAIL: return ToolReturnCode::CompilationFailed;
        case SLANG_FAIL:            return ToolReturnCode::Failed;
        case SLANG_E_NOT_AVAILABLE: return ToolReturnCode::Ignored;
        default:
        {
            return (SLANG_SUCCEEDED(res)) ? ToolReturnCode::Success : ToolReturnCode::Failed;
        }
    }
}

/* static */ToolReturnCode TestToolUtil::getReturnCodeFromInt(int code)
{
    if (code >= int(ToolReturnCodeSpan::First) && code <= int(ToolReturnCodeSpan::Last))
    {
        return ToolReturnCode(code);
    }
    else
    {
        SLANG_ASSERT(!"Invalid integral code");
        return ToolReturnCode::Failed;
    }
}

static SlangResult _calcIncludePath(const String& parentPath, const char* path, String& outIncludePath)
{
    String includePath;
    SLANG_RETURN_ON_FAIL(Path::getCanonical(Path::combine(parentPath, path), includePath));

    // Use forward slashes, to avoid escaping the path
    includePath = StringUtil::calcCharReplaced(includePath, '\\', '/');

    // It must exist!
    if (!File::exists(includePath))
    {
        return SLANG_FAIL;
    }

    outIncludePath = includePath;
    return SLANG_OK;
}

static SlangResult _addCPPPrelude(const String& parentPath, slang::IGlobalSession* session)
{
    String includePath;
    SLANG_RETURN_ON_FAIL(_calcIncludePath(parentPath, "../../../prelude/slang-cpp-prelude.h", includePath));
    StringBuilder prelude;
    prelude << "#include \"" << includePath << "\"\n\n";
    session->setLanguagePrelude(SLANG_SOURCE_LANGUAGE_CPP, prelude.getBuffer());
    return SLANG_OK;
}

static SlangResult _addCUDAPrelude(const String& parentPath, slang::IGlobalSession* session)
{
    String includePath;
    SLANG_RETURN_ON_FAIL(_calcIncludePath(parentPath, "../../../prelude/slang-cuda-prelude.h", includePath));
    StringBuilder prelude;
    prelude << "#include \"" << includePath << "\"\n\n";
    session->setLanguagePrelude(SLANG_SOURCE_LANGUAGE_CUDA, prelude.getBuffer());
    return SLANG_OK;
}

/* static */SlangResult TestToolUtil::setSessionDefaultPrelude(const PreludeInfo& info, slang::IGlobalSession* session)
{
    // Set the prelude to a path
    if (info.exePath)
    {
        String exePath(info.exePath);

        String canonicalPath;
        if (SLANG_SUCCEEDED(Path::getCanonical(exePath, canonicalPath)))
        {
            // Get the directory
            String parentPath = Path::getParentDirectory(canonicalPath);

            if (SLANG_FAILED(_addCPPPrelude(parentPath, session)))
            {
                SLANG_ASSERT(!"Couldn't find the C++ prelude relative to the executable");
            }

            if (SLANG_FAILED(_addCUDAPrelude(parentPath, session)))
            {
                SLANG_ASSERT(!"Couldn't find the CUDA prelude relative to the executable");
            }
        }
    }
    // If the nvAPI path is set, and we find nvHLSLExtns.h, put that in the HLSL prelude
    if (info.nvapiPath)
    {
        String includePath;
        if (SLANG_SUCCEEDED(_calcIncludePath(info.nvapiPath, "nvHLSLExtns.h", includePath)))
        {
            StringBuilder buf;

            buf << "#include \"" << includePath << "\"\n";

            session->setLanguagePrelude(SLANG_SOURCE_LANGUAGE_HLSL, buf.getBuffer());
            return SLANG_OK;
        }
    }

    return SLANG_OK;
}

/* static */SlangResult TestToolUtil::setSessionDefaultPrelude(const char* exePath, slang::IGlobalSession* session)
{
    PreludeInfo info;
    info.exePath = exePath;
    return setSessionDefaultPrelude(info, session);
}

}

