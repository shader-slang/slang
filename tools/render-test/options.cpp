// options.cpp

#include "options.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../source/core/slang-writer.h"

namespace renderer_test {

static const Options gDefaultOptions;

Options gOptions;

// Only set it, if the 
void setDefaultRendererType(RendererType type)
{
    gOptions.rendererType = (gOptions.rendererType == RendererType::Unknown) ? type : gOptions.rendererType;
}

SlangResult parseOptions(int argc, const char*const* argv, Slang::WriterHelper stdError)
{
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
            positionalArgs.Add(arg);
            continue;
        }

        if( strcmp(arg, "--") == 0 )
        {
            while(argCursor != argEnd)
            {
                positionalArgs.Add(*argCursor++);
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
        else if( strcmp(arg, "-hlsl") == 0 )
        {
            setDefaultRendererType( RendererType::DirectX11);
            gOptions.inputLanguageID = InputLanguageID::Native;
        }
        else if( strcmp(arg, "-glsl") == 0 )
        {
            setDefaultRendererType( RendererType::OpenGl);
            gOptions.inputLanguageID = InputLanguageID::Native;
        }
        else if( strcmp(arg, "-hlsl-rewrite") == 0 )
        {
            setDefaultRendererType( RendererType::DirectX11);
            gOptions.inputLanguageID = InputLanguageID::Slang;
        }
        else if( strcmp(arg, "-glsl-rewrite") == 0 )
        {
            setDefaultRendererType(RendererType::OpenGl);
            gOptions.inputLanguageID = InputLanguageID::Slang;
        }
        else if( strcmp(arg, "-slang") == 0 )
        {
            setDefaultRendererType( RendererType::DirectX11);
            gOptions.inputLanguageID = InputLanguageID::Slang;
        }
        else if( strcmp(arg, "-glsl-cross") == 0 )
        {
            setDefaultRendererType(RendererType::OpenGl);
            gOptions.inputLanguageID = InputLanguageID::Slang;
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
        else if (strcmp(arg, "-vk") == 0
            || strcmp(arg, "-vulkan") == 0)
        {
            gOptions.rendererType = RendererType::Vulkan;
        }
        else if (strcmp(arg, "-d3d12") == 0
            || strcmp(arg, "-dx12") == 0)
        {
            gOptions.rendererType = RendererType::DirectX12;
        }
        else if(strcmp(arg, "-gl") == 0)
        {
            gOptions.rendererType = RendererType::OpenGl;
        }
        else if (strcmp(arg, "-d3d11") == 0 
            || strcmp(arg, "-dx11") == 0)
        {
            gOptions.rendererType = RendererType::DirectX11;
        }
        else if( strcmp(arg, "-use-dxil") == 0 )
        {
            gOptions.useDXIL = true;
        }
        else
        {
            stdError.print("unknown option '%s'\n", arg);
            return SLANG_FAIL;
        }
    }
    
    
    // first positional argument is source shader path
    if(positionalArgs.Count())
    {
        gOptions.sourcePath = positionalArgs[0];
        positionalArgs.RemoveAt(0);
    }

    // any remaining arguments represent an error
    if(positionalArgs.Count() != 0)
    {
        stdError.print("unexpected arguments\n");
        return SLANG_FAIL;
    }

	return SLANG_OK;
}

} // renderer_test
