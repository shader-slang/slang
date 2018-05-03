// options.cpp

#include "options.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace renderer_test {

Options gOptions;

// Only set it, if the 
void setDefaultRendererType(RendererType type)
{
    gOptions.rendererType = (gOptions.rendererType == RendererType::Unknown) ? type : gOptions.rendererType;
}

SlangResult parseOptions(int* argc, char** argv)
{
    typedef Options::ShaderProgramType ShaderProgramType;
    typedef Options::InputLanguageID InputLanguageID;


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
            gOptions.inputLanguageID = InputLanguageID::NativeRewrite;
        }
        else if( strcmp(arg, "-glsl-rewrite") == 0 )
        {
            setDefaultRendererType(RendererType::OpenGl);
            gOptions.inputLanguageID = InputLanguageID::NativeRewrite;
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
                fprintf(stderr, "expected argument for '%s' option\n", arg);
                return SLANG_FAIL;
            }
            if( gOptions.slangArgCount == Options::kMaxSlangArgs )
            {
                fprintf(stderr, "maximum number of '%s' options exceeded (%d)\n", arg, Options::kMaxSlangArgs);
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
