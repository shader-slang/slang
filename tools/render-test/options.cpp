// options.cpp

#include "options.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../source/core/slang-writer.h"
#include "../../source/core/slang-render-api-util.h"

#include "../../source/core/slang-list.h"
#include "../../source/core/slang-string-util.h"
//#include "../../source/core/slang-downstream-compiler.h"

#include "../../source/core/slang-type-text-util.h"

namespace renderer_test {
using namespace Slang;

static gfx::DeviceType _toRenderType(Slang::RenderApiType apiType)
{
    using namespace Slang;
    switch (apiType)
    {
    case RenderApiType::D3D11:  return gfx::DeviceType::DirectX11;
    case RenderApiType::D3D12:  return gfx::DeviceType::DirectX12;
    case RenderApiType::OpenGl: return gfx::DeviceType::OpenGl;
    case RenderApiType::Vulkan: return gfx::DeviceType::Vulkan;
    case RenderApiType::CPU:    return gfx::DeviceType::CPU;
    case RenderApiType::CUDA:   return gfx::DeviceType::CUDA;
    default:
        return gfx::DeviceType::Unknown;
    }
}

static SlangResult _setRendererType(DeviceType type, const char* arg, Slang::WriterHelper stdError, Options& ioOptions)
{
    if (ioOptions.deviceType != DeviceType::Unknown)
    {
        stdError.print("Already has renderer option set. Found '%s'\n", arg);
        return SLANG_FAIL;
    }
    ioOptions.deviceType = type;
    return SLANG_OK;
}

/* static */SlangResult Options::parse(int argc, const char*const* argv, Slang::WriterHelper stdError, Options& outOptions)
{
    using namespace Slang;

    outOptions = Options();

    List<const char*> positionalArgs;

    typedef Options::ShaderProgramType ShaderProgramType;
    typedef Options::InputLanguageID InputLanguageID;

    //int argCount = argc;

    char const* const* argCursor = argv;
    char const* const* argEnd = argCursor + argc;

    // first argument is the application name
    if( argCursor != argEnd )
    {
        outOptions.appName = *argCursor++;
    }

    // now iterate over arguments to collect options
    while(argCursor != argEnd)
    {
        char const* arg = *argCursor++;
        if( arg[0] != '-' )
        {
            positionalArgs.add(arg);
            continue;
        }

        if( strcmp(arg, "--") == 0 )
        {
            while(argCursor != argEnd)
            {
                positionalArgs.add(*argCursor++);
            }
            break;
        }
        else if( strcmp(arg, "-o") == 0 )
        {
            if( argCursor == argEnd )
            {
                stdError.print("expected argument for '%s' option\n", arg);
                return SLANG_FAIL;
            }
            outOptions.outputPath = *argCursor++;
        }
        else if (strcmp(arg, "-profile") == 0)
        {
            if (argCursor == argEnd)
            {
                stdError.print("expected argument for '%s' option\n", arg);
                return SLANG_FAIL;
            }
            outOptions.profileName = *argCursor++;
        }
        else if (strcmp(arg, "-render-features") == 0 || strcmp(arg, "-render-feature") == 0)
        {
            if (argCursor == argEnd)
            {
                stdError.print("expected argument for '%s' option\n", arg);
                return SLANG_FAIL;
            }
            const char* value = *argCursor++;

            List<UnownedStringSlice> values;
            StringUtil::split(UnownedStringSlice(value), ',', values);

            for (const auto& value : values)
            {
                outOptions.renderFeatures.add(value);
            }
        }
        else if( strcmp(arg, "-xslang") == 0 )
        {
            // This is an option that we want to pass along to Slang

            if( argCursor == argEnd )
            {
                stdError.print("expected argument for '%s' option\n", arg);
                return SLANG_FAIL;
            }
            if( outOptions.slangArgCount == Options::kMaxSlangArgs )
            {
                stdError.print("maximum number of '%s' options exceeded (%d)\n", arg, Options::kMaxSlangArgs);
                return SLANG_FAIL;
            }
            outOptions.slangArgs[outOptions.slangArgCount++] = *argCursor++;
        }
		else if (strcmp(arg, "-compute") == 0)
		{
			outOptions.shaderType = ShaderProgramType::Compute;
		}
		else if (strcmp(arg, "-graphics") == 0)
		{
			outOptions.shaderType = ShaderProgramType::Graphics;
		}
        else if (strcmp(arg, "-gcompute") == 0)
        {
            outOptions.shaderType = ShaderProgramType::GraphicsCompute;
        }
        else if (strcmp(arg, "-rt") == 0)
        {
            outOptions.shaderType = ShaderProgramType::RayTracing;
        }
        else if( strcmp(arg, "-use-dxil") == 0 )
        {
            outOptions.useDXIL = true;
        }
        else if (strcmp(arg, "-only-startup") == 0)
        {
            outOptions.onlyStartup = true;
        }
        else if (strcmp(arg, "-compile-arg") == 0)
        {
            if (argCursor == argEnd)
            {
                stdError.print("expected argument for '%s' option\n", arg);
                return SLANG_FAIL;
            }

            const char* compileArg = *argCursor++;
            outOptions.compileArgs.add(compileArg);
        }
        else if (strcmp(arg, "-performance-profile") == 0)
        {
            outOptions.performanceProfile = true;
        }
        else if (strcmp(arg, "-adapter") == 0)
        {
            if (argCursor == argEnd)
            {
                stdError.print("expected argument for '%s' option\n", arg);
                return SLANG_FAIL;
            }

            outOptions.adapter = *argCursor++;
        }
        else if (strcmp(arg, "-output-using-type") == 0)
        {
            outOptions.outputUsingType = true;
        }
        else if (strcmp(arg, "-compute-dispatch") == 0)
        {
            if (argCursor == argEnd)
            {
                stdError.print("error: expecting a comma separated compute dispatch size for '%s'\n", arg);
                return SLANG_FAIL;
            }
            List<UnownedStringSlice> slices;
            StringUtil::split(UnownedStringSlice(*argCursor++), ',', slices);
            if (slices.getCount() != 3)
            {
                stdError.print("error: expected 3 comma separated integers for compute dispatch size for '%s'\n", arg);
                return SLANG_FAIL;
            }
           
            String string;
            for (Index i = 0; i < 3; ++i)
            {
                string = slices[i];
                int v = StringToInt(string);
                if (v < 1)
                {
                    stdError.print("error: expected 3 comma positive integers for compute dispatch size for '%s'\n", arg);
                    return SLANG_FAIL;
                }
                outOptions.computeDispatchSize[i] = v;
            }
        }
        else if (strcmp(arg, "-source-language") == 0)
        {
            if (argCursor == argEnd)
            {
                stdError.print("error: expecting a source language name for '%s'\n", arg);
                return SLANG_FAIL;
            }
            UnownedStringSlice sourceLanguageText(*argCursor++);

            SlangSourceLanguage sourceLanguage = TypeTextUtil::findSourceLanguage(sourceLanguageText);
            if (sourceLanguage == SLANG_SOURCE_LANGUAGE_UNKNOWN)
            {
                stdError.print("error: expecting unknown source language name '%s' for '%s'\n", String(sourceLanguageText).getBuffer(), arg);
                return SLANG_FAIL;
            }

            outOptions.sourceLanguage = sourceLanguage;
        }
        else if( strcmp(arg, "-no-default-entry-point") == 0 )
        {
            outOptions.dontAddDefaultEntryPoints = true;
        }
        else if (strcmp(arg, "-nvapi-slot") == 0)
        {
            if (argCursor == argEnd)
            {
                stdError.print("error: expecting a register name for '%s'\n", arg);
                return SLANG_FAIL;
            }

            outOptions.nvapiExtnSlot = (*argCursor++);
        }
        else if (strcmp(arg, "-shaderobj") == 0)
        {
            // Note: We ignore this option because it is always enabled now.
            //
            // TODO: At some point we could warn/error and deprecate this option.
        }
        else
        {
            // Lookup
            Slang::UnownedStringSlice argSlice(arg);
            if (argSlice.getLength() && argSlice[0] == '-')
            {
                // Look up the rendering API if set
                UnownedStringSlice argName = UnownedStringSlice(argSlice.begin() + 1, argSlice.end());
                DeviceType deviceType = _toRenderType(RenderApiUtil::findApiTypeByName(argName));

                if (deviceType != DeviceType::Unknown)
                {
                    outOptions.deviceType = deviceType;
                    continue;
                }

                // Lookup the target language type
                DeviceType languageRenderType =
                    _toRenderType(RenderApiUtil::findImplicitLanguageRenderApiType(argName));
                if (languageRenderType != DeviceType::Unknown)
                {
                    outOptions.targetLanguageDeviceType = languageRenderType;
                    outOptions.inputLanguageID = (argName == "hlsl" || argName == "glsl" || argName == "cpp" || argName == "cxx" || argName == "c") ?  InputLanguageID::Native : InputLanguageID::Slang;
                    continue;
                }
            }

            stdError.print("unknown option '%s'\n", arg);
            return SLANG_FAIL;
        }
    }

    // If a render option isn't set use defaultRenderType 
    outOptions.deviceType = (outOptions.deviceType == DeviceType::Unknown)
                                ? outOptions.targetLanguageDeviceType
                                : outOptions.deviceType;

    // first positional argument is source shader path
    if(positionalArgs.getCount())
    {
        outOptions.sourcePath = positionalArgs[0];
        positionalArgs.removeAt(0);
    }

    // any remaining arguments represent an error
    if(positionalArgs.getCount() != 0)
    {
        stdError.print("unexpected arguments\n");
        return SLANG_FAIL;
    }

	return SLANG_OK;
}

} // renderer_test
