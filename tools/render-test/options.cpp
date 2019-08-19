// options.cpp

#include "options.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../source/core/slang-writer.h"
#include "../../source/core/slang-render-api-util.h"

#include "../../source/core/slang-list.h"
#include "../../source/core/slang-string-util.h"

namespace renderer_test {
using namespace Slang;

static const Options gDefaultOptions;

Options gOptions;

static gfx::RendererType _toRenderType(Slang::RenderApiType apiType)
{
    using namespace Slang;
    switch (apiType)
    {
    case RenderApiType::D3D11: return gfx::RendererType::DirectX11;
    case RenderApiType::D3D12: return gfx::RendererType::DirectX12;
    case RenderApiType::OpenGl: return gfx::RendererType::OpenGl;
    case RenderApiType::Vulkan: return gfx::RendererType::Vulkan;
    case RenderApiType::CPU:    return gfx::RendererType::CPU;
    default: return gfx::RendererType::Unknown;
    }
}

static SlangResult _setRendererType(RendererType type, const char* arg, Slang::WriterHelper stdError)
{
    if (gOptions.rendererType != RendererType::Unknown)
    {
        stdError.print("Already has renderer option set. Found '%s'\n", arg);
        return SLANG_FAIL;
    }
    gOptions.rendererType = type;
    return SLANG_OK;
}

SlangResult parseOptions(int argc, const char*const* argv, Slang::WriterHelper stdError)
{
    using namespace Slang;

    // Reset the options
    gOptions = gDefaultOptions;

    List<const char*> positionalArgs;

    typedef Options::ShaderProgramType ShaderProgramType;
    typedef Options::InputLanguageID InputLanguageID;

    //int argCount = argc;

    char const* const* argCursor = argv;
    char const* const* argEnd = argCursor + argc;

    // first argument is the application name
    if( argCursor != argEnd )
    {
        gOptions.appName = *argCursor++;
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
            gOptions.outputPath = *argCursor++;
        }
        else if (strcmp(arg, "-profile") == 0)
        {
            if (argCursor == argEnd)
            {
                stdError.print("expected argument for '%s' option\n", arg);
                return SLANG_FAIL;
            }
            gOptions.profileName = *argCursor++;
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
                gOptions.renderFeatures.add(value);
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
            if( gOptions.slangArgCount == Options::kMaxSlangArgs )
            {
                stdError.print("maximum number of '%s' options exceeded (%d)\n", arg, Options::kMaxSlangArgs);
                return SLANG_FAIL;
            }
            gOptions.slangArgs[gOptions.slangArgCount++] = *argCursor++;
        }
		else if (strcmp(arg, "-compute") == 0)
		{
			gOptions.shaderType = ShaderProgramType::Compute;
		}
		else if (strcmp(arg, "-graphics") == 0)
		{
			gOptions.shaderType = ShaderProgramType::Graphics;
		}
        else if (strcmp(arg, "-gcompute") == 0)
        {
            gOptions.shaderType = ShaderProgramType::GraphicsCompute;
        }
        else if( strcmp(arg, "-use-dxil") == 0 )
        {
            gOptions.useDXIL = true;
        }
        else if (strcmp(arg, "-only-startup") == 0)
        {
            gOptions.onlyStartup = true;
        }
        else if (strcmp(arg, "-adapter") == 0)
        {
            if (argCursor == argEnd)
            {
                stdError.print("expected argument for '%s' option\n", arg);
                return SLANG_FAIL;
            }

            gOptions.adapter = *argCursor++;
        }
        else
        {
            // Lookup
            Slang::UnownedStringSlice argSlice(arg);
            if (argSlice.size() && argSlice[0] == '-')
            {
                // Look up the rendering API if set
                UnownedStringSlice argName = UnownedStringSlice(argSlice.begin() + 1, argSlice.end());
                RendererType rendererType = _toRenderType(RenderApiUtil::findApiTypeByName(argName));

                if (rendererType != RendererType::Unknown)
                {
                    gOptions.rendererType = rendererType;
                    continue;
                }

                // Lookup the target language type
                RendererType languageRenderType = _toRenderType(RenderApiUtil::findImplicitLanguageRenderApiType(argName));
                if (languageRenderType != RendererType::Unknown)
                {
                    gOptions.targetLanguageRendererType = languageRenderType;
                    gOptions.inputLanguageID = (argName == "hlsl" || argName == "glsl") ?  InputLanguageID::Native : InputLanguageID::Slang;
                    continue;
                }
            }

            stdError.print("unknown option '%s'\n", arg);
            return SLANG_FAIL;
        }
    }

    // If a render option isn't set use defaultRenderType 
    gOptions.rendererType = (gOptions.rendererType == RendererType::Unknown) ? gOptions.targetLanguageRendererType : gOptions.rendererType;

    // first positional argument is source shader path
    if(positionalArgs.getCount())
    {
        gOptions.sourcePath = positionalArgs[0];
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
