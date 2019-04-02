
#include "slang-test-tool-util.h"

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

/* static */SlangResult TestToolUtil::extractArg(const char*const* args, int argsCount, const char* argName, const char** outValue)
{
    SLANG_ASSERT(strlen(argName) > 0 && argName[0] == '-');
    for (int i = 0; i < argsCount - 1; ++i)
    {
        if (strcmp(args[i], argName) == 0)
        {
            *outValue = args[i + 1];
            return SLANG_OK;
        }
    }
    return SLANG_FAIL;
}

/* static */bool TestToolUtil::hasOption(const char*const* args, int argsCount, const char* argName)
{
    for (int i = 0; i < argsCount; ++i)
    {
        if (strcmp(args[i], argName) == 0)
        {
            return true;
        }
    }
    return false;
}

/* static */BackendType TestToolUtil::toBackendType(const UnownedStringSlice& slice)
{
    if (slice == "dxc")
    {
        return BackendType::Dxc;
    }
    else if (slice == "fxc")
    {
        return BackendType::Fxc;
    }
    else if (slice == "glslang")
    {
        return BackendType::Glslang;
    }
    return BackendType::Unknown;
}

/* static */BackendFlags TestToolUtil::getBackendFlagsForTarget(SlangCompileTarget target)
{
    switch (target)
    {
        case SLANG_TARGET_UNKNOWN:
        case SLANG_HLSL:
        case SLANG_GLSL:
        {
            return 0;
        }
        case SLANG_DXBC:
        case SLANG_DXBC_ASM:
        {
            return BackendFlag::Fxc;
        }
        case SLANG_SPIRV:
        case SLANG_SPIRV_ASM:
        {
            return BackendFlag::Glslang;
        }
        case SLANG_DXIL:
        case SLANG_DXIL_ASM:
        {
            return BackendFlag::Dxc;
        }
        default:
        {
            SLANG_ASSERT(!"Unknown type");
            return 0;
        }
    }
}

/* static */BackendType TestToolUtil::toBackendTypeFromPassThroughType(SlangPassThrough passThru)
{
    switch (passThru)
    {
        case SLANG_PASS_THROUGH_DXC: return BackendType::Dxc;
        case SLANG_PASS_THROUGH_FXC: return BackendType::Fxc;
        case SLANG_PASS_THROUGH_GLSLANG: return BackendType::Glslang;
        default:                     return BackendType::Unknown;
    }
}

/* static */SlangCompileTarget TestToolUtil::toCompileTarget(const UnownedStringSlice& name)
{
#define CASE(NAME, TARGET)  if(name == NAME) return SLANG_##TARGET;

    CASE("hlsl", HLSL)
        CASE("glsl", GLSL)
        CASE("dxbc", DXBC)
        CASE("dxbc-assembly", DXBC_ASM)
        CASE("dxbc-asm", DXBC_ASM)
        CASE("spirv", SPIRV)
        CASE("spirv-assembly", SPIRV_ASM)
        CASE("spirv-asm", SPIRV_ASM)
        CASE("dxil", DXIL)
        CASE("dxil-assembly", DXIL_ASM)
        CASE("dxil-asm", DXIL_ASM)
#undef CASE

        return SLANG_TARGET_UNKNOWN;
}


}

