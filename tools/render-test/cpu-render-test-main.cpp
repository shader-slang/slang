// cpu-render-test-main.cpp

#include "options.h"

#include "slang-support.h"

#include "../source/core/slang-io.h"

#include "shader-input-layout.h"
#include <stdio.h>
#include <stdlib.h>

#include "../../source/core/slang-test-tool-util.h"

#include "cpu-compute-util.h"

SLANG_TEST_TOOL_API SlangResult innerMain(Slang::StdWriters* stdWriters, SlangSession* session, int argcIn, const char*const* argvIn)
{
    using namespace renderer_test;
    using namespace Slang;

    StdWriters::setSingleton(stdWriters);

	// Parse command-line options
	SLANG_RETURN_ON_FAIL(parseOptions(argcIn, argvIn, StdWriters::getError()));

    // Declare window pointer before renderer, such that window is released after renderer
    RefPtr<renderer_test::Window> window;
    // Renderer is constructed (later) using the window
	Slang::RefPtr<Renderer> renderer;

    ShaderCompilerUtil::Input input;
    
    input.profile = "";
    input.target = SLANG_TARGET_NONE;
    input.args = &gOptions.slangArgs[0];
    input.argCount = gOptions.slangArgCount;

	SlangSourceLanguage nativeLanguage = SLANG_SOURCE_LANGUAGE_UNKNOWN;
	SlangPassThrough slangPassThrough = SLANG_PASS_THROUGH_NONE;
    char const* profileName = "";
	switch (gOptions.rendererType)
	{
        case RendererType::CPU:
            input.target = SLANG_HOST_CALLABLE;
            input.profile = "";
            nativeLanguage = SLANG_SOURCE_LANGUAGE_CPP;
            slangPassThrough = SLANG_PASS_THROUGH_GENERIC_C_CPP;
            break;
		default:
			fprintf(stderr, "error: unexpected\n");
			return SLANG_FAIL;
	}

    switch (gOptions.inputLanguageID)
    {
        case Options::InputLanguageID::Slang:
            input.sourceLanguage = SLANG_SOURCE_LANGUAGE_SLANG;
            input.passThrough = SLANG_PASS_THROUGH_NONE;
            break;

        case Options::InputLanguageID::Native:
            input.sourceLanguage = nativeLanguage;
            input.passThrough = slangPassThrough;
            break;

        default:
            break;
    }

    // Use the profile name set on options if set
    input.profile = gOptions.profileName ? gOptions.profileName : input.profile;

    {
        ShaderCompilerUtil::OutputAndLayout compilationAndLayout;
        SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(session, gOptions.sourcePath, gOptions.shaderType, input, compilationAndLayout));

        CPUComputeUtil::Context context;
        SLANG_RETURN_ON_FAIL(CPUComputeUtil::calcBindings(compilationAndLayout, context));
        SLANG_RETURN_ON_FAIL(CPUComputeUtil::execute(compilationAndLayout, context));

        // Dump everything out that was written
        return CPUComputeUtil::writeBindings(compilationAndLayout.layout, context.buffers, gOptions.outputPath);
    }
}

int main(int argc, char**  argv)
{
    using namespace Slang;
    SlangSession* session = spCreateSession(nullptr);

    TestToolUtil::setSessionDefaultPrelude(argv[0], session);
    auto stdWriters = StdWriters::initDefaultSingleton();
    SlangResult res = innerMain(stdWriters, session, argc, argv);
    spDestroySession(session);

	return (int)TestToolUtil::getReturnCode(res);
}

