
#include "cuda-compute-util.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-std-writers.h"
#include "../../source/core/slang-token-reader.h"

#include "../bind-location.h"

#include <cuda.h>
#include <cuda_runtime_api.h>

namespace renderer_test {
using namespace Slang;

#define SLANG_CUDA_RETURN_ON_FAIL(x) { int _res = (int)(x); if (_res != 0) return SLANG_FAIL; }

#define SLANG_CUDA_ASSERT_ON_FAIL(x) { int _res = (int)(x); if (_res !=0) { SLANG_ASSERT(!"Failed CUDA call"); }; }

class CUDAResource : public RefObject
{
public:
    typedef RefObject Super;

        /// Dtor
    CUDAResource(): m_cudaMemory(nullptr) {}
    CUDAResource(void* cudaMemory): m_cudaMemory(cudaMemory) {}

    ~CUDAResource()
    {
        if (m_cudaMemory)
        {
            SLANG_CUDA_ASSERT_ON_FAIL(cudaFree(&m_cudaMemory));
        }
    }

        /// Helper function to get the cuda memory pointer when given a value
    static void* getCUDAData(BindSet::Value* value)
    {
        if (value)
        {
            auto resource = dynamic_cast<CUDAResource*>(value->m_target.Ptr());
            return resource ? resource->m_cudaMemory : nullptr;
        }
        return nullptr;
    }

    void* m_cudaMemory;
};


static int _calcSMCountPerMultiProcessor(int major, int minor)
{
    // Defines for GPU Architecture types (using the SM version to determine
    // the # of cores per SM
    struct SMInfo
    {
        int sm;  // 0xMm (hexadecimal notation), M = SM Major version, and m = SM minor version
        int coreCount;
    };

    static const SMInfo infos[] =
    {
        {0x30, 192},
        {0x32, 192},
        {0x35, 192},
        {0x37, 192},
        {0x50, 128},
        {0x52, 128},
        {0x53, 128},
        {0x60,  64},
        {0x61, 128},
        {0x62, 128},
        {0x70,  64},
        {0x72,  64},
        {0x75,  64}
    };

    const int sm = ((major << 4) + minor);
    for (Index i = 0; i < SLANG_COUNT_OF(infos); ++i)
    {
        if (infos[i].sm == sm)
        {
            return infos[i].coreCount;
        }
    }

    const auto& last = infos[SLANG_COUNT_OF(infos) - 1];

    // It must be newer presumably
    SLANG_ASSERT(sm > last.coreCount );

    // Default to the last entry
    return last.coreCount;
}

static SlangResult _findMaxFlopsDeviceId(int* outDevice)
{
    int smPerMultiproc = 0;
    int maxPerfDevice = -1;
    int deviceCount = 0;
    int devicesProhibited = 0;

    uint64_t maxComputePerf = 0;
    SLANG_CUDA_RETURN_ON_FAIL(cudaGetDeviceCount(&deviceCount));

    // Find the best CUDA capable GPU device
    for (int currentDevice = 0; currentDevice < deviceCount; ++currentDevice)
    {
        int computeMode = -1, major = 0, minor = 0;
        SLANG_CUDA_RETURN_ON_FAIL(cudaDeviceGetAttribute(&computeMode, cudaDevAttrComputeMode, currentDevice));
        SLANG_CUDA_RETURN_ON_FAIL(cudaDeviceGetAttribute(&major, cudaDevAttrComputeCapabilityMajor, currentDevice));
        SLANG_CUDA_RETURN_ON_FAIL(cudaDeviceGetAttribute(&minor, cudaDevAttrComputeCapabilityMinor, currentDevice));

        // If this GPU is not running on Compute Mode prohibited,
        // then we can add it to the list
        if (computeMode != cudaComputeModeProhibited)
        {
            if (major == 9999 && minor == 9999)
            {
                smPerMultiproc = 1;
            }
            else
            {
                smPerMultiproc = _calcSMCountPerMultiProcessor(major, minor);
            }

            int multiProcessorCount = 0, clockRate = 0;
            SLANG_CUDA_RETURN_ON_FAIL(cudaDeviceGetAttribute(&multiProcessorCount, cudaDevAttrMultiProcessorCount, currentDevice));
            SLANG_CUDA_RETURN_ON_FAIL(cudaDeviceGetAttribute(&clockRate, cudaDevAttrClockRate, currentDevice));
            uint64_t compute_perf = uint64_t(multiProcessorCount) * smPerMultiproc * clockRate;

            if (compute_perf > maxComputePerf)
            {
                maxComputePerf = compute_perf;
                maxPerfDevice = currentDevice;
            }
        }
        else
        {
            devicesProhibited++;
        }
    }

    if (maxPerfDevice < 0)
    {
        return SLANG_FAIL;
    }

    *outDevice = maxPerfDevice;
    return SLANG_OK;
}

static SlangResult _initCuda()
{
    static CUresult res = cuInit(0);
    SLANG_CUDA_RETURN_ON_FAIL(res);

    return SLANG_OK;
}

/* static */SlangResult _createDevice(CUcontext* outContext)
{
    SLANG_RETURN_ON_FAIL(_initCuda());

    int deviceId;
    SLANG_RETURN_ON_FAIL(_findMaxFlopsDeviceId(&deviceId));
    SLANG_CUDA_RETURN_ON_FAIL(cudaSetDevice(deviceId));

    CUcontext context;

    // Create context
    SLANG_CUDA_RETURN_ON_FAIL(cuCtxCreate(&context, 0, deviceId));

    *outContext = context;
    return SLANG_OK;
}

/* static */bool CUDAComputeUtil::canCreateDevice()
{
    CUcontext context;
    if (SLANG_SUCCEEDED(_createDevice(&context)))
    {
        cuCtxDestroy(context);
        return true;
    }

    return false;
}

static SlangResult _compute(CUcontext context, CUmodule module, const ShaderCompilerUtil::OutputAndLayout& outputAndLayout, List<BindSet::Value*>& outBuffers)
{
    auto request = outputAndLayout.output.request;
    auto reflection = (slang::ShaderReflection*) spGetReflection(request);

    slang::EntryPointReflection* entryPoint = nullptr;
    auto entryPointCount = reflection->getEntryPointCount();
    SLANG_ASSERT(entryPointCount == 1);

    entryPoint = reflection->getEntryPointByIndex(0);

    const char* entryPointName = entryPoint->getName();

    // Get the entry point
    CUfunction kernel;
    SLANG_CUDA_RETURN_ON_FAIL(cuModuleGetFunction(&kernel, module, entryPointName));

    // A stream of 0 means no stream
    cudaStream_t stream = 0;
    //SLANG_CUDA_RETURN_ON_FAIL(cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking));

    {
        // Okay now we need to set up binding
        BindSet bindSet;
        CPULikeBindRoot bindRoot;
        bindRoot.init(&bindSet, reflection, 0);

        // Will set up any root buffers
        bindRoot.addDefaultValues();

        // Now set up the Values from the test

        auto outStream = StdWriters::getOut();
        SLANG_RETURN_ON_FAIL(ShaderInputLayout::addBindSetValues(outputAndLayout.layout.entries, outputAndLayout.sourcePath, outStream, bindRoot));

        ShaderInputLayout::getValueBuffers(outputAndLayout.layout.entries, bindSet, outBuffers);

        // First create all of the resources for the values

        {
            const auto& values = bindSet.getValues();
            const auto& entries = outputAndLayout.layout.entries;

            for (BindSet::Value* value : values)
            {
                auto typeLayout = value->m_type;
               
                // Get the type kind, if typeLayout is not set we'll assume a 'constant buffer' will do
                slang::TypeReflection::Kind kind = typeLayout ? typeLayout->getKind() : slang::TypeReflection::Kind::ConstantBuffer;
               
                // TODO(JS):
                // Here we should be using information about what textures hold to create appropriate
                // textures. For now we only support 2d textures that always return 1.
                
                switch (kind)
                {
                    case slang::TypeReflection::Kind::ConstantBuffer:
                    case slang::TypeReflection::Kind::ParameterBlock:
                    {
                        // We can construct the buffers. We can't copy into yet, as we need to set all of the bindings first

                        void* cudaMem;
                        SLANG_CUDA_RETURN_ON_FAIL(cudaMalloc(&cudaMem, value->m_sizeInBytes));
                        value->m_target = new CUDAResource(cudaMem);
                        break;
                    }
                    case slang::TypeReflection::Kind::Resource:
                    {
                        auto type = typeLayout->getType();
                        auto shape = type->getResourceShape();

                        //auto access = type->getResourceAccess();

                        switch (shape & SLANG_RESOURCE_BASE_SHAPE_MASK)
                        {
                            case SLANG_TEXTURE_2D:
                            {
                                SLANG_ASSERT(value->m_userIndex >= 0);
                                auto& srcEntry = entries[value->m_userIndex];

                                // TODO(JS):
                                // We should use the srcEntry to determine what data to store in the texture,
                                // it's dimensions etc. For now we just support it being 1.

                                slang::TypeReflection* typeReflection = typeLayout->getResourceResultType();

                                int count = 1;
                                if (typeReflection->getKind() == slang::TypeReflection::Kind::Vector)
                                {
                                    count = int(typeReflection->getElementCount());
                                }

                                // TODO(JS): Should use the input setup to work how to create this texture
                                // Store the target specific value
                                //value->m_target = _newOneTexture2D(count);
                                break;
                            }
                            case SLANG_TEXTURE_1D:
                            case SLANG_TEXTURE_3D:
                            case SLANG_TEXTURE_CUBE:
                            case SLANG_TEXTURE_BUFFER:
                            {
                                // Need a CPU impl for these...
                                // For now we can just leave as target will just be nullptr
                                break;
                            }

                            case SLANG_BYTE_ADDRESS_BUFFER:
                            case SLANG_STRUCTURED_BUFFER:
                            {
                                // On CPU we just use the memory in the BindSet buffer, so don't need to create anything

                                void* cudaMem;
                                SLANG_CUDA_RETURN_ON_FAIL(cudaMalloc(&cudaMem, value->m_sizeInBytes));
                                value->m_target = new CUDAResource(cudaMem);

                                break;
                            }
                        }
                    }
                    default: break;
                }
            }
        }
    
        // Now we need to go through all of the bindings and set the appropriate data

        {
            List<BindLocation> locations;
            List<BindSet::Value*> values;
            bindSet.getBindings(locations, values);

            for (Index i = 0; i < locations.getCount(); ++i)
            {
                const auto& location = locations[i];
                BindSet::Value* value = values[i];

                // Okay now we need to set up the actual handles that CPU will follow.
                auto typeLayout = location.getTypeLayout();

                const auto kind = typeLayout->getKind();
                switch (kind)
                {
                    case slang::TypeReflection::Kind::Array:
                    {
                        auto elementCount = int(typeLayout->getElementCount());
                        if (elementCount == 0)
                        {
                            void** array = location.getUniform<void*>();
                            // If set, we setup the data needed for array on CPU side
                            if (value && array)
                            {
                                // TODO(JS): For now we'll just assume a pointer...
                                *array = CUDAResource::getCUDAData(value);
                            }
                        }
                        break;
                    }
                    case slang::TypeReflection::Kind::ConstantBuffer:
                    case slang::TypeReflection::Kind::ParameterBlock:
                    {
                        // These map down to just pointers
                        *location.getUniform<void*>() = CUDAResource::getCUDAData(value);
                        break;
                    }
                    case slang::TypeReflection::Kind::Resource:
                    {
                        auto type = typeLayout->getType();
                        auto shape = type->getResourceShape();

                        //auto access = type->getResourceAccess();

                        switch (shape & SLANG_RESOURCE_BASE_SHAPE_MASK)
                        {
                            case SLANG_BYTE_ADDRESS_BUFFER:
                            case SLANG_STRUCTURED_BUFFER:
                            {
                                // TODO(JS): These will need bounds ... 
                                // For the moment these are just pointers
                                *location.getUniform<void*>() = CUDAResource::getCUDAData(value);
                                break;
                            }
                        }
                        break;
                    }
                    default: break;
                }
            }
        }

        // Okay now the memory is all set up, we can copy everything over
        {
            const auto& values = bindSet.getValues();
            const auto& entries = outputAndLayout.layout.entries;

            for (BindSet::Value* value : values)
            {
                void* cudaMem = CUDAResource::getCUDAData(value);
                if (value && value->m_data && cudaMem)
                {
                    // Okay copy the data over...
                    cudaMemcpy(cudaMem, value->m_data, value->m_sizeInBytes, cudaMemcpyHostToDevice);
                }
            }
        }

        // Now we can execute the kernel

        {
            // Get the max threads per block for this function

            int maxTheadsPerBlock;
            SLANG_CUDA_RETURN_ON_FAIL(cuFuncGetAttribute(&maxTheadsPerBlock, CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK, kernel));

            int sharedSizeInBytes;
            SLANG_CUDA_RETURN_ON_FAIL(cuFuncGetAttribute(&sharedSizeInBytes, CU_FUNC_ATTRIBUTE_SHARED_SIZE_BYTES, kernel));

            // Work out the args
            void* uniformCUDAData = CUDAResource::getCUDAData(bindRoot.getRootValue());
            void* entryPointCUDAData = CUDAResource::getCUDAData(bindRoot.getEntryPointValue());

            void* args[] = { uniformCUDAData, entryPointCUDAData };

            SlangUInt numThreadsPerAxis[3];
            entryPoint->getComputeThreadGroupSize(3, numThreadsPerAxis);

            // Launch
            // TODO(JS): We probably want to do something a little more clever here using the maxThreadsPerBlock,
            // but for now just launch a single block, and hope it all fits.

            auto cudaLaunchResult = cuLaunchKernel(kernel,
                1, 1, 1,                                                                                // Blocks
                int(numThreadsPerAxis[0]), int(numThreadsPerAxis[1]), int(numThreadsPerAxis[2]),        // Threads per block
                0,                                                                                      // Shared memory size
                stream,                                                                                 // Stream. 0 is no stream.
                args,                                                                                   // Args
                nullptr);                                                                               // extra

            SLANG_CUDA_RETURN_ON_FAIL(cudaLaunchResult);

            if (stream)
            {
                SLANG_CUDA_RETURN_ON_FAIL(cudaStreamSynchronize(stream));
                SLANG_CUDA_RETURN_ON_FAIL(cudaStreamDestroy(stream));
            }
        }

        // Finally we need to copy the data back

        {
            const auto& entries = outputAndLayout.layout.entries;

            for (Index i = 0; i < entries.getCount(); ++i)
            {
                const auto& entry = entries[i];
                BindSet::Value* value = outBuffers[i];

                if (entry.isOutput)
                {
                    // Copy back to CPU memory
                    void* cudaMem = CUDAResource::getCUDAData(value);
                    if (value && value->m_data && cudaMem)
                    {
                        // Okay copy the data back...
                        cudaMemcpy(value->m_data, cudaMem, value->m_sizeInBytes, cudaMemcpyDeviceToHost);
                    }
                }
            }
        }
    }

    return SLANG_OK;
}

/* static */SlangResult CUDAComputeUtil::execute(const ShaderCompilerUtil::OutputAndLayout& outputAndLayout, List<BindSet::Value*>& outBuffers)
{
    CUcontext context;
    SLANG_RETURN_ON_FAIL(_createDevice(&context));

    const Index index = outputAndLayout.output.findKernelDescIndex(StageType::Compute);
    if (index < 0)
    {
        return SLANG_FAIL;
    }

    const auto& kernel = outputAndLayout.output.kernelDescs[index];

    CUmodule module = 0;
    SLANG_CUDA_RETURN_ON_FAIL(cuModuleLoadData(&module, kernel.codeBegin));

    SLANG_RETURN_ON_FAIL(_compute(context, module, outputAndLayout, outBuffers));

    SLANG_CUDA_RETURN_ON_FAIL(cuModuleUnload(module));

    cuCtxDestroy(context);

    return SLANG_OK;
}


} // renderer_test
