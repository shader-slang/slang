// slang-support.cpp

#define _CRT_SECURE_NO_WARNINGS 1

#include "slang-support.h"

#include <assert.h>
#include <stdio.h>

namespace renderer_test {

struct SlangShaderCompilerWrapper : public ShaderCompiler
{
    ShaderCompiler*     innerCompiler;
    SlangCompileTarget  target;
    SlangSourceLanguage sourceLanguage;

	virtual ShaderProgram* compileProgram(ShaderCompileRequest const& request) override
	{
		SlangSession* slangSession = spCreateSession(NULL);
		SlangCompileRequest* slangRequest = spCreateCompileRequest(slangSession);

		spSetCodeGenTarget(slangRequest, target);

		// Define a macro so that shader code in a test can detect what language we
		// are nominally working with.
		char const* langDefine = nullptr;
		switch (sourceLanguage)
		{
		case SLANG_SOURCE_LANGUAGE_GLSL:    langDefine = "__GLSL__";    break;
		case SLANG_SOURCE_LANGUAGE_HLSL:    langDefine = "__HLSL__";    break;
		case SLANG_SOURCE_LANGUAGE_SLANG:   langDefine = "__SLANG__";   break;
		default:
			assert(!"unexpected");
			break;
		}
		spAddPreprocessorDefine(slangRequest, langDefine, "1");

        // If we are dealing with GLSL input, then we need to set up
        // Slang to pass through to glslang instead of actually running
        // the compiler (this is a workaround to make direct comparisons
        // possible)
        if (sourceLanguage == SLANG_SOURCE_LANGUAGE_GLSL)
        {
            spSetPassThrough(slangRequest, SLANG_PASS_THROUGH_GLSLANG);
        }

        // Preocess any additional command-line options specified for Slang using
        // the `-xslang <arg>` option to `render-test`.
		spProcessCommandLineArguments(slangRequest, &gOptions.slangArgs[0], gOptions.slangArgCount);

		int computeTranslationUnit = 0;
		int vertexTranslationUnit = 0;
		int fragmentTranslationUnit = 0;
		char const* vertexEntryPointName = request.vertexShader.name;
		char const* fragmentEntryPointName = request.fragmentShader.name;
		char const* computeEntryPointName = request.computeShader.name;

		if (sourceLanguage == SLANG_SOURCE_LANGUAGE_GLSL)
		{
			// GLSL presents unique challenges because, frankly, it got the whole
			// compilation model wrong. One aspect of working around this is that
			// we will compile the same source file multiple times: once per
			// entry point, and we will have different preprocessor definitions
			// active in each case.

			vertexTranslationUnit = spAddTranslationUnit(slangRequest, sourceLanguage, nullptr);
			spAddTranslationUnitSourceString(slangRequest, vertexTranslationUnit, request.source.path, request.source.dataBegin);
			spTranslationUnit_addPreprocessorDefine(slangRequest, vertexTranslationUnit, "__GLSL_VERTEX__", "1");
			vertexEntryPointName = "main";

			fragmentTranslationUnit = spAddTranslationUnit(slangRequest, sourceLanguage, nullptr);
			spAddTranslationUnitSourceString(slangRequest, fragmentTranslationUnit, request.source.path, request.source.dataBegin);
			spTranslationUnit_addPreprocessorDefine(slangRequest, fragmentTranslationUnit, "__GLSL_FRAGMENT__", "1");
			fragmentEntryPointName = "main";

			computeTranslationUnit = spAddTranslationUnit(slangRequest, sourceLanguage, nullptr);
			spAddTranslationUnitSourceString(slangRequest, computeTranslationUnit, request.source.path, request.source.dataBegin);
			spTranslationUnit_addPreprocessorDefine(slangRequest, computeTranslationUnit, "__GLSL_COMPUTE__", "1");
			computeEntryPointName = "main";
		}
		else
		{
			int translationUnit = spAddTranslationUnit(slangRequest, sourceLanguage, nullptr);
			spAddTranslationUnitSourceString(slangRequest, translationUnit, request.source.path, request.source.dataBegin);

			vertexTranslationUnit = translationUnit;
			fragmentTranslationUnit = translationUnit;
			computeTranslationUnit = translationUnit;
		}


		ShaderProgram * result = nullptr;
        Slang::List<const char*> rawTypeNames;
        for (auto typeName : request.entryPointTypeArguments)
            rawTypeNames.Add(typeName.Buffer());
		if (request.computeShader.name)
		{
		    int computeEntryPoint = spAddEntryPointEx(slangRequest, computeTranslationUnit, 
                computeEntryPointName, 
                spFindProfile(slangSession, request.computeShader.profile),
                (int)rawTypeNames.Count(),
                rawTypeNames.Buffer());

            spSetLineDirectiveMode(slangRequest, SLANG_LINE_DIRECTIVE_MODE_NONE);
			int compileErr = spCompile(slangRequest);
			if (auto diagnostics = spGetDiagnosticOutput(slangRequest))
			{
				fprintf(stderr, "%s", diagnostics);
			}
			if (!compileErr)
			{
				ShaderCompileRequest innerRequest = request;

                size_t codeSize = 0;
                char const* code = (char const*) spGetEntryPointCode(slangRequest, computeEntryPoint, &codeSize);
                innerRequest.computeShader.source.dataBegin = code;
                innerRequest.computeShader.source.dataEnd = code + codeSize;
				result = innerCompiler->compileProgram(innerRequest);
			}
		}
		else
		{
			int vertexEntryPoint = spAddEntryPointEx(slangRequest, vertexTranslationUnit, vertexEntryPointName, spFindProfile(slangSession, request.vertexShader.profile), (int)rawTypeNames.Count(), rawTypeNames.Buffer());
			int fragmentEntryPoint = spAddEntryPointEx(slangRequest, fragmentTranslationUnit, fragmentEntryPointName, spFindProfile(slangSession, request.fragmentShader.profile), (int)rawTypeNames.Count(), rawTypeNames.Buffer());

			int compileErr = spCompile(slangRequest);
			if (auto diagnostics = spGetDiagnosticOutput(slangRequest))
			{
				// TODO(tfoley): re-enable when I get a logging solution in place
	//            OutputDebugStringA(diagnostics);
				fprintf(stderr, "%s", diagnostics);
			}
			if (!compileErr)
			{
				ShaderCompileRequest innerRequest = request;

                size_t vertexCodeSize = 0;
				char const* vertexCode = (char const*) spGetEntryPointCode(slangRequest, vertexEntryPoint, &vertexCodeSize);

                size_t fragmentCodeSize = 0;
                char const* fragmentCode = (char const*) spGetEntryPointCode(slangRequest, fragmentEntryPoint, &fragmentCodeSize);

				innerRequest.vertexShader.source.dataBegin = vertexCode;
                innerRequest.vertexShader.source.dataEnd   = vertexCode + vertexCodeSize;

				innerRequest.fragmentShader.source.dataBegin = fragmentCode;
                innerRequest.fragmentShader.source.dataEnd   = fragmentCode + fragmentCodeSize;

				result = innerCompiler->compileProgram(innerRequest);
			}
		}
        // We clean up the Slang compilation context and result *after*
        // we have run the downstream compiler, because Slang
        // owns the memory allocation for the generated text, and will
        // free it when we destroy the compilation result.
        spDestroyCompileRequest(slangRequest);
        spDestroySession(slangSession);

        return result;
    }
};

ShaderCompiler* createSlangShaderCompiler(
    ShaderCompiler*     innerCompiler,
    SlangSourceLanguage sourceLanguage,
    SlangCompileTarget  target)
{
    auto result = new SlangShaderCompilerWrapper();
    result->innerCompiler = innerCompiler;
    result->sourceLanguage = sourceLanguage;
    result->target = target;

    return result;
}

SlangResult generateTextureResource(const InputTextureDesc& inputDesc, int bindFlags, Renderer* renderer, Slang::RefPtr<TextureResource>& textureOut)
{
    using namespace Slang;

    TextureData texData;
    generateTextureData(texData, inputDesc);

    TextureResource::Desc textureResourceDesc;
    textureResourceDesc.init();

    textureResourceDesc.format = Format::RGBA_Unorm_UInt8;
    textureResourceDesc.numMipLevels = texData.mipLevels;
    textureResourceDesc.arraySize = inputDesc.arrayLength;
    textureResourceDesc.bindFlags = bindFlags;

    // It's the same size in all dimensions 
    Resource::Type type = Resource::Type::Unknown;
    switch (inputDesc.dimension)
    {
        case 1:
        {
            type = Resource::Type::Texture1D;
            textureResourceDesc.size.init(inputDesc.size);
            break;
        }
        case 2:
        {
            type = inputDesc.isCube ? Resource::Type::TextureCube : Resource::Type::Texture2D;
            textureResourceDesc.size.init(inputDesc.size, inputDesc.size);
            break;
        }
        case 3:
        {
            type = Resource::Type::Texture3D;
            textureResourceDesc.size.init(inputDesc.size, inputDesc.size, inputDesc.size);
            break;
        }
    }

    const int effectiveArraySize = textureResourceDesc.calcEffectiveArraySize(type);
    const int numSubResources = textureResourceDesc.calcNumSubResources(type);

    Resource::Usage initialUsage = Resource::Usage::GenericRead;
    TextureResource::Data initData;

    List<ptrdiff_t> mipRowStrides;
    mipRowStrides.SetSize(textureResourceDesc.numMipLevels);
    List<const void*> subResources;
    subResources.SetSize(numSubResources);

    // Set up the src row strides
    for (int i = 0; i < textureResourceDesc.numMipLevels; i++)
    {
        const int mipWidth = TextureResource::calcMipSize(textureResourceDesc.size.width, i);
        mipRowStrides[i] = mipWidth * sizeof(uint32_t);
    }

    // Set up pointers the the data
    {
        int subResourceIndex = 0;
        const int numGen = int(texData.dataBuffer.Count());
        for (int i = 0; i < numSubResources; i++)
        {
            subResources[i] = texData.dataBuffer[subResourceIndex].Buffer();
            // Wrap around
            subResourceIndex = (subResourceIndex + 1 >= numGen) ? 0 : (subResourceIndex + 1);
        }
    }

    initData.mipRowStrides = mipRowStrides.Buffer();
    initData.numMips = textureResourceDesc.numMipLevels;
    initData.numSubResources = numSubResources;
    initData.subResources = subResources.Buffer();

    textureOut = renderer->createTextureResource(type, Resource::Usage::GenericRead, textureResourceDesc, &initData);

    return textureOut ? SLANG_OK : SLANG_FAIL;
}

SlangResult createInputBufferResource(const InputBufferDesc& inputDesc, bool isOutput, size_t bufferSize, const void* initData, Renderer* renderer, Slang::RefPtr<BufferResource>& bufferOut)
{
    using namespace Slang;

    Resource::Usage initialUsage = Resource::Usage::GenericRead;

    BufferResource::Desc srcDesc;
    srcDesc.init(bufferSize);

    int bindFlags = 0;
    if (inputDesc.type == InputBufferType::ConstantBuffer)
    {
        bindFlags |= Resource::BindFlag::ConstantBuffer;
        srcDesc.cpuAccessFlags |= Resource::AccessFlag::Write;
        initialUsage = Resource::Usage::ConstantBuffer;
    }
    else
    {
        bindFlags |= Resource::BindFlag::UnorderedAccess | Resource::BindFlag::PixelShaderResource | Resource::BindFlag::NonPixelShaderResource;
        srcDesc.elementSize = inputDesc.stride;
        initialUsage = Resource::Usage::UnorderedAccess;
    }

    if (isOutput)
    {
        srcDesc.cpuAccessFlags |= Resource::AccessFlag::Read;
    }

    srcDesc.bindFlags = bindFlags;

    RefPtr<BufferResource> bufferResource = renderer->createBufferResource(initialUsage, srcDesc, initData);
    if (!bufferResource)
    {
        return SLANG_FAIL;
    }

    bufferOut = bufferResource;
    return SLANG_OK;
}
    
static BindingState::SamplerDesc _calcSamplerDesc(const InputSamplerDesc& srcDesc)
{
    BindingState::SamplerDesc dstDesc;
    dstDesc.isCompareSampler = srcDesc.isCompareSampler;
    return dstDesc;
}

SlangResult createBindingSetDesc(ShaderInputLayoutEntry* srcEntries, int numEntries, Renderer* renderer, BindingState::Desc& descOut)
{
    using namespace Slang;

    const int textureBindFlags = Resource::BindFlag::NonPixelShaderResource | Resource::BindFlag::PixelShaderResource;

    descOut.clear();
    for (int i = 0; i < numEntries; i++)
    {
        const ShaderInputLayoutEntry& srcEntry = srcEntries[i];

        BindingState::RegisterDesc registerDesc;
        registerDesc.registerSets[int(BindingState::ShaderStyle::Hlsl)] = descOut.addRegisterSet(srcEntry.hlslBinding);
        registerDesc.registerSets[int(BindingState::ShaderStyle::Glsl)] = descOut.addRegisterSet(srcEntry.glslBinding.Buffer(), int(srcEntry.glslBinding.Count()));

        switch (srcEntry.type)
        {
            case ShaderInputType::Buffer:
            {
                const InputBufferDesc& srcBuffer = srcEntry.bufferDesc;

                const size_t bufferSize = srcEntry.bufferData.Count() * sizeof(uint32_t);

                RefPtr<BufferResource> bufferResource;
                SLANG_RETURN_ON_FAIL(createInputBufferResource(srcEntry.bufferDesc, srcEntry.isOutput, bufferSize, srcEntry.bufferData.Buffer(), renderer, bufferResource));
                
                descOut.addBufferResource(bufferResource, registerDesc);
                break;
            }
            case ShaderInputType::CombinedTextureSampler:
            {
                RefPtr<TextureResource> texture;
                SLANG_RETURN_ON_FAIL(generateTextureResource(srcEntry.textureDesc, textureBindFlags, renderer, texture));
                descOut.addCombinedTextureSampler(texture, _calcSamplerDesc(srcEntry.samplerDesc), registerDesc);
                break;
            }
            case ShaderInputType::Texture:
            {
                RefPtr<TextureResource> texture;
                SLANG_RETURN_ON_FAIL(generateTextureResource(srcEntry.textureDesc, textureBindFlags, renderer, texture));

                descOut.addTextureResource(texture, registerDesc);
                break;
            }
            case ShaderInputType::Sampler:
            {
                descOut.addSampler(_calcSamplerDesc(srcEntry.samplerDesc), registerDesc);
                break;
            }
            default: 
            {
                assert(!"Unhandled type");
                return SLANG_FAIL;
            }
        }
    }    

    return SLANG_OK;
}

SlangResult createBindingSetDesc(const ShaderInputLayout& layout, Renderer* renderer, BindingState::Desc& descOut)
{
    SLANG_RETURN_ON_FAIL(createBindingSetDesc(layout.entries.Buffer(), int(layout.entries.Count()), renderer, descOut));
    descOut.m_numRenderTargets = layout.numRenderTargets;

    return SLANG_OK;
}

SlangResult serializeBindingOutput(const ShaderInputLayout& layout, BindingState* bindingState, Renderer* renderer, const char* fileName)
{
    // Submit the work
    renderer->submitGpuWork();
    // Wait until everything is complete
    renderer->waitForGpu();

    FILE * f = fopen(fileName, "wb");
    if (!f)
    {
        return SLANG_FAIL;
    }

    const BindingState::Desc& bindingStateDesc = bindingState->getDesc();
    // Must be the same amount of entries
    assert(bindingStateDesc.m_bindings.Count() == layout.entries.Count());

    int id = 0;
    const int numBindings = int(bindingStateDesc.m_bindings.Count());

    for (int i = 0; i < numBindings; ++i)
    {
        const auto& layoutBinding = layout.entries[i];
        const auto& binding = bindingStateDesc.m_bindings[i];

        if (layoutBinding.isOutput)
        {
            if (binding.resource && binding.resource->isBuffer())
            {
                BufferResource* bufferResource = static_cast<BufferResource*>(binding.resource.Ptr());
                const size_t bufferSize = bufferResource->getDesc().sizeInBytes;

                unsigned int* ptr = (unsigned int*)renderer->map(bufferResource, MapFlavor::HostRead);
                if (!ptr)
                {
                    fclose(f);
                    return SLANG_FAIL;
                }

                const int size = int(bufferSize / sizeof(unsigned int));
                for (int i = 0; i < size; ++i)
                {
                    fprintf(f, "%X\n", ptr[i]);
                }
                renderer->unmap(bufferResource);
            }
            else
            {
                printf("invalid output type at %d.\n", id);
            }
        }
        id++;
    }
    fclose(f);

    return SLANG_OK;
}


} // renderer_test
