// options.cpp

#include "options.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace renderer_test {

Options gOptions;

// Only set it, if the 
void setDefaultRendererID(RendererID id)
{
    gOptions.rendererID = (gOptions.rendererID == RendererID::NONE) ? id : gOptions.rendererID;
}

SlangResult parseOptions(int* argc, char** argv)
{
    int argCount = *argc;
    char const* const* argCursor = argv;
    char const* const* argEnd = argCursor + argCount;

    char const** writeCursor = (char const**) argv;

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
            *writeCursor++ = arg;
            continue;
        }

        if( strcmp(arg, "--") == 0 )
        {
            while(argCursor != argEnd)
            {
                char const* arg = *argCursor++;
                *writeCursor++ = arg;
            }
            break;
        }
        else if( strcmp(arg, "-o") == 0 )
        {
            if( argCursor == argEnd )
            {
                fprintf(stderr, "expected argument for '%s' option\n", arg);
                return SLANG_FAIL;
            }
            gOptions.outputPath = *argCursor++;
        }
        else if( strcmp(arg, "-hlsl") == 0 )
        {
            setDefaultRendererID( RendererID::D3D11);
            gOptions.inputLanguageID = InputLanguageID::Native;
        }
        else if( strcmp(arg, "-glsl") == 0 )
        {
            setDefaultRendererID(RendererID::GL);
            gOptions.inputLanguageID = InputLanguageID::Native;
        }
        else if( strcmp(arg, "-hlsl-rewrite") == 0 )
        {
            setDefaultRendererID(RendererID::D3D11);
            gOptions.inputLanguageID = InputLanguageID::NativeRewrite;
        }
        else if( strcmp(arg, "-glsl-rewrite") == 0 )
        {
            setDefaultRendererID(RendererID::GL);
            gOptions.inputLanguageID = InputLanguageID::NativeRewrite;
        }
        else if( strcmp(arg, "-slang") == 0 )
        {
            setDefaultRendererID(RendererID::D3D11);
            gOptions.inputLanguageID = InputLanguageID::Slang;
        }
        else if( strcmp(arg, "-glsl-cross") == 0 )
        {
            setDefaultRendererID(RendererID::GL);
            gOptions.inputLanguageID = InputLanguageID::Slang;
        }
        else if( strcmp(arg, "-xslang") == 0 )
        {
            // This is an option that we want to pass along to Slang

            if( argCursor == argEnd )
            {
                fprintf(stderr, "expected argument for '%s' option\n", arg);
                return SLANG_FAIL;
            }
            if( gOptions.slangArgCount == kMaxSlangArgs )
            {
                fprintf(stderr, "maximum number of '%s' options exceeded (%d)\n", arg, kMaxSlangArgs);
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
            gOptions.rendererID = RendererID::VK;
        }
        else if (strcmp(arg, "-d3d12") == 0
            || strcmp(arg, "-dx12") == 0)
        {
            gOptions.rendererID = RendererID::D3D12;
        }
        else if(strcmp(arg, "-gl") == 0)
        {
            gOptions.rendererID = RendererID::GL;
        }
        else if (strcmp(arg, "-d3d11") == 0 
            || strcmp(arg, "-dx11") == 0)
        {
            gOptions.rendererID = RendererID::D3D11;
        }
        else
        {
            fprintf(stderr, "unknown option '%s'\n", arg);
            return SLANG_FAIL;
        }
    }
    
    // any arguments left over were positional arguments
    argCount = (int)(writeCursor - argv);
    argCursor = argv;
    argEnd = argCursor + argCount;

    // first positional argument is source shader path
    if( argCursor != argEnd )
    {
        gOptions.sourcePath = *argCursor++;
    }

    // any remaining arguments represent an error
    if(argCursor != argEnd)
    {
        fprintf(stderr, "unexpected arguments\n");
        return SLANG_FAIL;
    }

    *argc = 0;
	return SLANG_OK;
}

} // renderer_test
