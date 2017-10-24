// main.cpp

#include "options.h"
#include "render.h"
#include "render-d3d11.h"
#include "render-gl.h"
#include "slang-support.h"
#include "shader-input-layout.h"
#include <stdio.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

namespace renderer_test {

//


int gWindowWidth = 1024;
int gWindowHeight = 768;

//
// For the purposes of a small example, we will define the vertex data for a
// single triangle directly in the source file. It should be easy to extend
// this example to load data from an external source, if desired.
//

struct Vertex
{
    float position[3];
    float color[3];
};

static const int kVertexCount = 3;
static const Vertex kVertexData[kVertexCount] = {
    { { 0,  0, 0.5 }, {1, 0, 0} },
    { { 0,  1, 0.5 }, {0, 0, 1} },
    { { 1,  0, 0.5 }, {0, 1, 0} },
};


// Global variables for state to be used for rendering...

uintptr_t gConstantBufferSize, gComputeResultBufferSize;

Buffer*         gConstantBuffer;
InputLayout*    gInputLayout;
Buffer*         gVertexBuffer;
ShaderProgram*      gShaderProgram;
BindingState*       gBindingState;
ShaderInputLayout   gShaderInputLayout;

// Entry point name to use for vertex/fragment shader
static char const* vertexEntryPointName    = "vertexMain";
static char const* fragmentEntryPointName  = "fragmentMain";
static char const* computeEntryPointName = "computeMain";

// "Profile" to use when compiling for HLSL targets
// TODO: does this belong here?
static char const* vertexProfileName   = "vs_4_0";
static char const* fragmentProfileName = "ps_4_0";
static char const* computeProfileName = "cs_4_0";

Error initializeShaders(
    ShaderCompiler* shaderCompiler)
{
    // Read in the source code
    char const* sourcePath = gOptions.sourcePath;
    FILE* sourceFile = fopen(sourcePath, "rb");
    if( !sourceFile )
    {
        fprintf(stderr, "error: failed to open '%s' for reading\n", sourcePath);
        exit(1);
    }
    fseek(sourceFile, 0, SEEK_END);
    size_t sourceSize = ftell(sourceFile);
    fseek(sourceFile, 0, SEEK_SET);
    char* sourceText = (char*) malloc(sourceSize + 1);
    if( !sourceText )
    {
        fprintf(stderr, "error: out of memory");
        exit(1);
    }
    fread(sourceText, sourceSize, 1, sourceFile);
    fclose(sourceFile);
    sourceText[sourceSize] = 0;

    gShaderInputLayout.Parse(sourceText);

    ShaderCompileRequest::SourceInfo sourceInfo;
    sourceInfo.path = sourcePath;
    sourceInfo.text = sourceText;

    ShaderCompileRequest compileRequest;
    compileRequest.source                   = sourceInfo;
	if (gOptions.shaderType == ShaderProgramType::Graphics)
	{
		compileRequest.vertexShader.source = sourceInfo;
		compileRequest.vertexShader.name = vertexEntryPointName;
		compileRequest.vertexShader.profile = vertexProfileName;
		compileRequest.fragmentShader.source = sourceInfo;
		compileRequest.fragmentShader.name = fragmentEntryPointName;
		compileRequest.fragmentShader.profile = fragmentProfileName;
	}
	else
	{
		compileRequest.computeShader.source = sourceInfo;
		compileRequest.computeShader.name = computeEntryPointName;
		compileRequest.computeShader.profile = computeProfileName;
	}
    gShaderProgram = shaderCompiler->compileProgram(compileRequest);
    if( !gShaderProgram )
    {
        return Error::Unexpected;
    }

    return Error::None;
}
//
// At initialization time, we are going to load and compile our Slang shader
// code, and then create the D3D11 API objects we need for rendering.
//
Error initializeInner(
    Renderer*       renderer,
    ShaderCompiler* shaderCompiler)
{
    Error err = Error::None;

    err = initializeShaders(shaderCompiler);
    if(err != Error::None) return err;

    gBindingState = renderer->createBindingState(gShaderInputLayout);

    // Do other initialization that doesn't depend on the source language.

    // TODO(tfoley): use each API's reflection interface to query the constant-buffer size needed
    gConstantBufferSize = 16 * sizeof(float);

    BufferDesc constantBufferDesc;
    constantBufferDesc.size = gConstantBufferSize;
    constantBufferDesc.flavor = BufferFlavor::Constant;

    gConstantBuffer = renderer->createBuffer(constantBufferDesc);
    if(!gConstantBuffer)
        return Error::Unexpected;
    
    // Input Assembler (IA)

    InputElementDesc inputElements[] = {
        { "A", 0, Format::RGB_Float32, offsetof(Vertex, position) },
        { "A", 1, Format::RGB_Float32, offsetof(Vertex, color) },
    };

    gInputLayout = renderer->createInputLayout(&inputElements[0], 2);
    if(!gInputLayout)
        return Error::Unexpected;

    BufferDesc vertexBufferDesc;
    vertexBufferDesc.size = kVertexCount * sizeof(Vertex);
    vertexBufferDesc.flavor = BufferFlavor::Vertex;
    vertexBufferDesc.initData = &kVertexData[0];

    gVertexBuffer = renderer->createBuffer(vertexBufferDesc);
    if(!gVertexBuffer)
        return Error::Unexpected;

    return Error::None;
}

void renderFrameInner(
    Renderer* renderer)
{
    auto mappedData = renderer->map(gConstantBuffer, MapFlavor::WriteDiscard);
    if(mappedData)
    {
        static const float kIdentity[] =
        { 1, 0, 0, 0,
          0, 1, 0, 0,
          0, 0, 1, 0,
          0, 0, 0, 1 };
        memcpy(mappedData, kIdentity, sizeof(kIdentity));

        renderer->unmap(gConstantBuffer);
    }

    // Input Assembler (IA)

    renderer->setInputLayout(gInputLayout);
    renderer->setPrimitiveTopology(PrimitiveTopology::TriangleList);

    renderer->setVertexBuffer(0, gVertexBuffer, sizeof(Vertex));

    // Vertex Shader (VS)
    // Pixel Shader (PS)

    renderer->setShaderProgram(gShaderProgram);
    renderer->setConstantBuffer(0, gConstantBuffer);
    renderer->setBindingState(gBindingState);
    //

    renderer->draw(3);
}

void runCompute(Renderer * renderer)
{
	renderer->setShaderProgram(gShaderProgram);
    renderer->setBindingState(gBindingState);
	renderer->dispatchCompute(1, 1, 1);
}

void finalize()
{
}



//
// We use a bare-minimum window procedure to get things up and running.
//

static LRESULT CALLBACK windowProc(
    HWND    windowHandle,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch (message)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(windowHandle, message, wParam, lParam);
}


} // renderer_test


//

int main(
    int argc,
    char**  argv)
{
    using namespace renderer_test;

    // Parse command-line options
    parseOptions(&argc, argv);


    // Do initial window-creation stuff here, rather than in the renderer-specific files

    HINSTANCE instance = GetModuleHandleA(0);
    int showCommand = SW_SHOW;

    // First we register a window class.

    WNDCLASSEXW windowClassDesc;
    windowClassDesc.cbSize = sizeof(windowClassDesc);
    windowClassDesc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClassDesc.lpfnWndProc = &windowProc;
    windowClassDesc.cbClsExtra = 0;
    windowClassDesc.cbWndExtra = 0;
    windowClassDesc.hInstance = instance;
    windowClassDesc.hIcon = 0;
    windowClassDesc.hCursor = 0;
    windowClassDesc.hbrBackground = 0;
    windowClassDesc.lpszMenuName = 0;
    windowClassDesc.lpszClassName = L"HelloWorld";
    windowClassDesc.hIconSm = 0;
    ATOM windowClassAtom = RegisterClassExW(&windowClassDesc);
    if(!windowClassAtom)
    {
        fprintf(stderr, "error: failed to register window class\n");
        return 1;
    }

    // Next, we create a window using that window class.

    // We will create a borderless window since our screen-capture logic in GL
    // seems to get thrown off by having to deal with a window frame.
    DWORD windowStyle = WS_POPUP;
    DWORD windowExtendedStyle = 0;


    RECT windowRect = { 0, 0, gWindowWidth, gWindowHeight };
    AdjustWindowRectEx(&windowRect, windowStyle, /*hasMenu=*/false, windowExtendedStyle);

    auto width = windowRect.right - windowRect.left;
    auto height = windowRect.bottom - windowRect.top;

    LPWSTR windowName = L"Slang Render Test";
    HWND windowHandle = CreateWindowExW(
        windowExtendedStyle,
        (LPWSTR)windowClassAtom,
        windowName,
        windowStyle,
        0, 0, // x, y
        width, height,
        NULL, // parent
        NULL, // menu
        instance,
        NULL);
    if(!windowHandle)
    {
        fprintf(stderr, "error: failed to create window\n");
        return 1;
    }


    Renderer* renderer = nullptr;
    switch( gOptions.mode )
    {
    case Mode::Slang:
    case Mode::HLSL:
    case Mode::HLSLRewrite:
        renderer = createD3D11Renderer();
        break;

    case Mode::GLSL:
    case Mode::GLSLRewrite:
    case Mode::GLSLCrossCompile:
        renderer = createGLRenderer();
        break;

    default:
        fprintf(stderr, "error: unexpected\n");
        exit(1);
        break;
    }

    renderer->initialize(windowHandle);

    auto shaderCompiler = renderer->getShaderCompiler();
    switch( gOptions.mode )
    {
    case Mode::Slang:
        shaderCompiler = createSlangShaderCompiler(shaderCompiler, SLANG_SOURCE_LANGUAGE_SLANG, SLANG_HLSL);
        break;

    case Mode::HLSLRewrite:
        shaderCompiler = createSlangShaderCompiler(shaderCompiler, SLANG_SOURCE_LANGUAGE_HLSL, SLANG_HLSL);
        break;

    case Mode::GLSLRewrite:
        shaderCompiler = createSlangShaderCompiler(shaderCompiler, SLANG_SOURCE_LANGUAGE_GLSL, SLANG_GLSL);
        break;

    case Mode::GLSLCrossCompile:
        shaderCompiler = createSlangShaderCompiler(shaderCompiler, SLANG_SOURCE_LANGUAGE_SLANG, SLANG_GLSL);
        break;

    default:
        break;
    }

    Error err = initializeInner(renderer, shaderCompiler);
    if( err != Error::None )
    {
        exit(1);
    }


    // Once initialization is all complete, we show the window...
    ShowWindow(windowHandle, showCommand);
	
	// ... and enter the event loop:
	for (;;)
	{
		MSG message;

		int result = PeekMessageW(&message, NULL, 0, 0, PM_REMOVE);
		if (result != 0)
		{
			if (message.message == WM_QUIT)
			{
                return (int)message.wParam;
			}

			TranslateMessage(&message);
			DispatchMessageW(&message);
		}
		else
		{
			// Whenver we don't have Windows events to process,
			// we render a frame.
			static const float kClearColor[] = { 0.25, 0.25, 0.25, 1.0 };
			renderer->setClearColor(kClearColor);
			renderer->clearFrame();

			if (gOptions.shaderType == ShaderProgramType::Compute)
			{
				runCompute(renderer);
			}
			else
			{
				renderFrameInner(renderer);
			}
			// If we are in a mode where output is requested, we need to snapshot the back buffer here
			if (gOptions.outputPath)
			{
                if (gOptions.shaderType == ShaderProgramType::Compute)
                    renderer->serializeOutput(gBindingState, gOptions.outputPath);
                else
				    renderer->captureScreenShot(gOptions.outputPath);
				return 0;
			}

			renderer->presentFrame();
		}
	}

    return 0;
}

