#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"
#include "source/core/slang-string-util.h"
#include "source/core/slang-io.h"
#include "source/core/slang-file-system.h"

#include "gfx-test-texture-util.h"

using namespace gfx;
using namespace Slang;

namespace gfx_test
{

    struct ShaderCacheTest
    {
        UnitTestContext* context;
        Slang::RenderApiFlag::Enum api;

        String testDirectory;
        String cacheDirectory;

        ComPtr<ISlangMutableFileSystem> diskFileSystem;

        IDevice::ShaderCacheDesc shaderCacheDesc = {};

        ComPtr<IDevice> device;
        ComPtr<IShaderCache> shaderCache;
        ComPtr<IPipelineState> pipelineState;
        ComPtr<IResourceView> bufferView;

        String computeShaderA = String(
            R"(
            [shader("compute")]
            [numthreads(4, 1, 1)]
            void main(
                uint3 sv_dispatchThreadID : SV_DispatchThreadID,
                uniform RWStructuredBuffer<float> buffer)
            {
                var input = buffer[sv_dispatchThreadID.x];
                buffer[sv_dispatchThreadID.x] = input + 1.0f;
            }
            )");    

        String computeShaderB = String(
            R"(
            [shader("compute")]
            [numthreads(4, 1, 1)]
            void main(
                uint3 sv_dispatchThreadID : SV_DispatchThreadID,
                uniform RWStructuredBuffer<float> buffer)
            {
                var input = buffer[sv_dispatchThreadID.x];
                buffer[sv_dispatchThreadID.x] = input + 2.0f;
            }
            )");

        String computeShaderC = String(
            R"(
            [shader("compute")]
            [numthreads(4, 1, 1)]
            void main(
                uint3 sv_dispatchThreadID : SV_DispatchThreadID,
                uniform RWStructuredBuffer<float> buffer)
            {
                var input = buffer[sv_dispatchThreadID.x];
                buffer[sv_dispatchThreadID.x] = input + 3.0f;
            }
            )");


        void removeDirectory(const String& directory)
        {
            auto osFileSystem = OSFileSystem::getMutableSingleton();

            struct Context
            {
                ISlangMutableFileSystem *fileSystem;
                const String& directory;
            } context { osFileSystem, directory };

            osFileSystem->enumeratePathContents(
                directory.getBuffer(),
                [](SlangPathType pathType, const char* fileName, void* userData)
                {
                    struct Context* context = static_cast<Context *>(userData);
                    if (pathType == SlangPathType::SLANG_PATH_TYPE_FILE)
                    {
                        String path = Path::simplify(context->directory + "/" + fileName);
                        context->fileSystem->remove(path.getBuffer());
                    }
                },
                &context);

            osFileSystem->remove(directory.getBuffer());
        }

        void writeShader(const String& source, const String& fileName)
        {
            diskFileSystem->saveFile(fileName.getBuffer(), source.getBuffer(), source.getLength());
        }

        void init(UnitTestContext* context, Slang::RenderApiFlag::Enum api)
        {
            this->context = context;
            this->api = api;

            testDirectory = Path::simplify(Path::getParentDirectory(Path::getExecutablePath()) + "/shader-cache-test");
            cacheDirectory = Path::simplify(testDirectory + "/cache");

            // Cleanup if there are stale files from a previously aborted test.
            removeDirectory(cacheDirectory);
            removeDirectory(testDirectory);

            Path::createDirectory(testDirectory);
            diskFileSystem = new RelativeFileSystem(OSFileSystem::getMutableSingleton(), testDirectory);
            shaderCacheDesc.shaderCachePath = cacheDirectory.getBuffer();
        }

        void cleanup()
        {
            removeDirectory(cacheDirectory);
            removeDirectory(testDirectory);
        }

        template<typename Func>
        void runStep(Func func)
        {
            List<const char*> additionalSearchPaths;
            additionalSearchPaths.add(testDirectory.getBuffer());

            runTestImpl(
                [this, func] (IDevice* device, UnitTestContext* ctx)
                {
                    this->device = device;
                    device->queryInterface(SLANG_UUID_IShaderCache, (void**)this->shaderCache.writeRef());
                    func();
                    this->device = nullptr;
                    this->shaderCache = nullptr;
                },
                context, api, additionalSearchPaths, shaderCacheDesc);
        }

        void createComputeResources()
        {
            const int numberCount = 4;
            float initialData[] = { 0.0f, 1.0f, 2.0f, 3.0f };
            IBufferResource::Desc bufferDesc = {};
            bufferDesc.sizeInBytes = numberCount * sizeof(float);
            bufferDesc.format = Format::Unknown;
            bufferDesc.elementSize = sizeof(float);
            bufferDesc.allowedStates = ResourceStateSet(
                ResourceState::ShaderResource,
                ResourceState::UnorderedAccess,
                ResourceState::CopyDestination,
                ResourceState::CopySource);
            bufferDesc.defaultState = ResourceState::UnorderedAccess;
            bufferDesc.memoryType = MemoryType::DeviceLocal;

            ComPtr<IBufferResource> numbersBuffer;
            GFX_CHECK_CALL_ABORT(device->createBufferResource(
                bufferDesc,
                (void*)initialData,
                numbersBuffer.writeRef()));

            IResourceView::Desc viewDesc = {};
            viewDesc.type = IResourceView::Type::UnorderedAccess;
            viewDesc.format = Format::Unknown;
            GFX_CHECK_CALL_ABORT(
                device->createBufferView(numbersBuffer, nullptr, viewDesc, bufferView.writeRef()));
        }

        void freeComputeResources()
        {
            bufferView = nullptr;
            pipelineState = nullptr;
        }

        void createComputePipeline(const char* moduleName, const char* entryPointName)
        {
            ComPtr<IShaderProgram> shaderProgram;
            slang::ProgramLayout* slangReflection;
            GFX_CHECK_CALL_ABORT(loadComputeProgram(device, shaderProgram, moduleName, entryPointName, slangReflection));

            ComputePipelineStateDesc pipelineDesc = {};
            pipelineDesc.program = shaderProgram.get();
            GFX_CHECK_CALL_ABORT(
                device->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));
        }

        void dispatchComputePipeline()
        {
            ComPtr<ITransientResourceHeap> transientHeap;
            ITransientResourceHeap::Desc transientHeapDesc = {};
            transientHeapDesc.constantBufferSize = 4096;
            GFX_CHECK_CALL_ABORT(
                device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

            ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
            auto queue = device->createCommandQueue(queueDesc);

            auto commandBuffer = transientHeap->createCommandBuffer();
            auto encoder = commandBuffer->encodeComputeCommands();

            auto rootObject = encoder->bindPipeline(pipelineState);

            ShaderCursor entryPointCursor(rootObject->getEntryPoint(0));
            entryPointCursor.getPath("buffer").setResource(bufferView);

            // ShaderCursor rootCursor(rootObject);
            // Bind buffer view to the entry point.
            // rootCursor.getPath("buffer").setResource(bufferView);

            encoder->dispatchCompute(4, 1, 1);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->waitOnHost();
        }        

        void runComputePipeline(const char* moduleName, const char* entryPointName)
        {
            createComputeResources();
            createComputePipeline(moduleName, entryPointName);
            dispatchComputePipeline();
            freeComputeResources();
        }

        ShaderCacheStats getStats()
        {
            SLANG_ASSERT(shaderCache);
            ShaderCacheStats stats;
            shaderCache->getShaderCacheStats(&stats);
            return stats;
        }

        void run(UnitTestContext* context, Slang::RenderApiFlag::Enum api)
        {
            init(context, api);
            runTests();
            cleanup();
        }

        virtual void runTests() = 0;
    };

    // Basic shader cache test using 3 different shader files stored on disk.
    struct ShaderCacheTestBasic : ShaderCacheTest
    {
        void runTests()
        {
            // Write shader source files.
            writeShader(computeShaderA, "shader-cache-tmp-a.slang");
            writeShader(computeShaderB, "shader-cache-tmp-b.slang");
            writeShader(computeShaderC, "shader-cache-tmp-c.slang");

            // Cache is cold and we expect 3 misses.
            runStep(
                [this]()
                {
                    runComputePipeline("shader-cache-tmp-a", "main");
                    runComputePipeline("shader-cache-tmp-b", "main");
                    runComputePipeline("shader-cache-tmp-c", "main");

                    SLANG_CHECK(getStats().missCount == 3);
                    SLANG_CHECK(getStats().hitCount == 0);
                    SLANG_CHECK(getStats().entryCount == 3);
                }
            );

            // Cache is hot and we expect 3 hits.
            runStep(
                [this]()
                {
                    runComputePipeline("shader-cache-tmp-a", "main");
                    runComputePipeline("shader-cache-tmp-b", "main");
                    runComputePipeline("shader-cache-tmp-c", "main");

                    SLANG_CHECK(getStats().missCount == 0);
                    SLANG_CHECK(getStats().hitCount == 3);
                    SLANG_CHECK(getStats().entryCount == 3);
                }
            );

            // Write shader source files, all rotated by one.
            writeShader(computeShaderA, "shader-cache-tmp-b.slang");
            writeShader(computeShaderB, "shader-cache-tmp-c.slang");
            writeShader(computeShaderC, "shader-cache-tmp-a.slang");

            // Cache is cold again and we expect 3 misses.
            runStep(
                [this]()
                {
                    runComputePipeline("shader-cache-tmp-a", "main");
                    runComputePipeline("shader-cache-tmp-b", "main");
                    runComputePipeline("shader-cache-tmp-c", "main");

                    SLANG_CHECK(getStats().missCount == 3);
                    SLANG_CHECK(getStats().hitCount == 0);
                    SLANG_CHECK(getStats().entryCount == 6);
                }
            );

            // Cache is hot again and we expect 3 hits.
            runStep(
                [this]()
                {
                    runComputePipeline("shader-cache-tmp-a", "main");
                    runComputePipeline("shader-cache-tmp-b", "main");
                    runComputePipeline("shader-cache-tmp-c", "main");

                    SLANG_CHECK(getStats().missCount == 0);
                    SLANG_CHECK(getStats().hitCount == 3);
                    SLANG_CHECK(getStats().entryCount == 6);
                }
            );
        }
    };

    // Test one shader file on disk with multiple entry points.
    struct ShaderCacheTestEntryPoint : ShaderCacheTest
    {
        void runTests()
        {
            // Cache is cold and we expect 3 misses, one for each entry point.
            runStep(
                [this]()
                {
                    runComputePipeline("shader-cache-multiple-entry-points", "computeA");
                    runComputePipeline("shader-cache-multiple-entry-points", "computeB");
                    runComputePipeline("shader-cache-multiple-entry-points", "computeC");

                    SLANG_CHECK(getStats().missCount == 3);
                    SLANG_CHECK(getStats().hitCount == 0);
                    SLANG_CHECK(getStats().entryCount == 3);
                }
            );

            // Cache is hot and we expect 3 hits.
            runStep(
                [this]()
                {
                    runComputePipeline("shader-cache-multiple-entry-points", "computeA");
                    runComputePipeline("shader-cache-multiple-entry-points", "computeB");
                    runComputePipeline("shader-cache-multiple-entry-points", "computeC");

                    SLANG_CHECK(getStats().missCount == 0);
                    SLANG_CHECK(getStats().hitCount == 3);
                    SLANG_CHECK(getStats().entryCount == 3);
                }
            );            
        }
    };

    // Test cache invalidation due to an import/include file being changed on disk.
    struct ShaderCacheTestImportInclude : ShaderCacheTest
    {
        String importedContentsA = String(
            R"(
            void processElement(RWStructuredBuffer<float> buffer, uint index)
            {
                var input = buffer[index];
                buffer[index] = input + 1.0f;
            }
            )");

        String importedContentsB = String(
            R"(
            void processElement(RWStructuredBuffer<float> buffer, uint index)
            {
                var input = buffer[index];
                buffer[index] = input + 2.0f;
            }
            )");

        String importFile = String(
            R"(
            import shader_cache_tmp_imported;
            
            [shader("compute")]
            [numthreads(4, 1, 1)]
            void main(
                uint3 sv_dispatchThreadID : SV_DispatchThreadID,
                uniform RWStructuredBuffer<float> buffer)
            {
                processElement(buffer, sv_dispatchThreadID.x);
            }
            )");

        String includeFile = String(
            R"(
            #include "shader-cache-tmp-imported.slang"

            [shader("compute")]
            [numthreads(4, 1, 1)]
            void main(
                uint3 sv_dispatchThreadID : SV_DispatchThreadID,
                uniform RWStructuredBuffer<float> buffer)
            {
                processElement(buffer, sv_dispatchThreadID.x);
            })");

        void runTests()
        {
            // Write shader source files.
            writeShader(importedContentsA, "shader-cache-tmp-imported.slang");
            writeShader(importFile, "shader-cache-tmp-import.slang");
            writeShader(includeFile, "shader-cache-tmp-include.slang");

            // Cache is cold and we expect 2 misses.
            runStep(
                [this]()
                {
                    runComputePipeline("shader-cache-tmp-import", "main");
                    runComputePipeline("shader-cache-tmp-include", "main");

                    SLANG_CHECK(getStats().missCount == 2);
                    SLANG_CHECK(getStats().hitCount == 0);
                    SLANG_CHECK(getStats().entryCount == 2);
                }
            );

            // Cache is hot and we expect 2 hits.
            runStep(
                [this]()
                {
                    runComputePipeline("shader-cache-tmp-import", "main");
                    runComputePipeline("shader-cache-tmp-include", "main");

                    SLANG_CHECK(getStats().missCount == 0);
                    SLANG_CHECK(getStats().hitCount == 2);
                    SLANG_CHECK(getStats().entryCount == 2);
                }
            );             

            // Change content of imported/included shader file.
            writeShader(importedContentsB, "shader-cache-tmp-imported.slang");

            // Cache is cold and we expect 2 misses.
            runStep(
                [this]()
                {
                    runComputePipeline("shader-cache-tmp-import", "main");
                    runComputePipeline("shader-cache-tmp-include", "main");

                    SLANG_CHECK(getStats().missCount == 2);
                    SLANG_CHECK(getStats().hitCount == 0);
                    SLANG_CHECK(getStats().entryCount == 4);
                }
            );

            // Cache is hot and we expect 2 hits.
            runStep(
                [this]()
                {
                    runComputePipeline("shader-cache-tmp-import", "main");
                    runComputePipeline("shader-cache-tmp-include", "main");

                    SLANG_CHECK(getStats().missCount == 0);
                    SLANG_CHECK(getStats().hitCount == 2);
                    SLANG_CHECK(getStats().entryCount == 4);
                }
            );             
        }
    };

    // One shader featuring multiple kinds of shader objects that can be bound.
    struct ShaderCacheTestSpecialization : ShaderCacheTest
    {
        slang::ProgramLayout* slangReflection;

        void createComputePipeline()
        {
            ComPtr<IShaderProgram> shaderProgram;

            GFX_CHECK_CALL_ABORT(
                loadComputeProgram(device, shaderProgram, "shader-cache-specialization", "computeMain", slangReflection));

            ComputePipelineStateDesc pipelineDesc = {};
            pipelineDesc.program = shaderProgram.get();
            GFX_CHECK_CALL_ABORT(
                device->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));
        }        

        void createAddTransformer(IShaderObject** transformer)
        {
            slang::TypeReflection* addTransformerType =
                slangReflection->findTypeByName("AddTransformer");
            GFX_CHECK_CALL_ABORT(device->createShaderObject(
                addTransformerType, ShaderObjectContainerType::None, transformer));

            float c = 1.0f;
            ShaderCursor(*transformer).getPath("c").setData(&c, sizeof(float));
        }

        void createMulTransformer(IShaderObject** transformer)
        {
            slang::TypeReflection* mulTransformerType =
                slangReflection->findTypeByName("MulTransformer");
            GFX_CHECK_CALL_ABORT(device->createShaderObject(
                mulTransformerType, ShaderObjectContainerType::None, transformer));

            float c = 1.0f;
            ShaderCursor(*transformer).getPath("c").setData(&c, sizeof(float));
        }

        void dispatchComputePipeline(const char* transformerTypeName)
        {
            Slang::ComPtr<ITransientResourceHeap> transientHeap;
            ITransientResourceHeap::Desc transientHeapDesc = {};
            transientHeapDesc.constantBufferSize = 4096;
            GFX_CHECK_CALL_ABORT(
                device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

            ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
            auto queue = device->createCommandQueue(queueDesc);

            auto commandBuffer = transientHeap->createCommandBuffer();
            auto encoder = commandBuffer->encodeComputeCommands();

            auto rootObject = encoder->bindPipeline(pipelineState);

            Slang::ComPtr<IShaderObject> transformer;
            slang::TypeReflection* transformerType = slangReflection->findTypeByName(transformerTypeName);
            GFX_CHECK_CALL_ABORT(device->createShaderObject(
                transformerType, ShaderObjectContainerType::None, transformer.writeRef()));

            float c = 1.0f;
            ShaderCursor(transformer).getPath("c").setData(&c, sizeof(float));

            ShaderCursor entryPointCursor(rootObject->getEntryPoint(0));
            entryPointCursor.getPath("buffer").setResource(bufferView);
            entryPointCursor.getPath("transformer").setObject(transformer);

            encoder->dispatchCompute(1, 1, 1);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->waitOnHost();
        }

        void runComputePipeline(const char* transformerTypeName)
        {
            createComputeResources();
            createComputePipeline();
            dispatchComputePipeline(transformerTypeName);
            freeComputeResources();
        }        

        void runTests()
        {
            // Cache is cold and we expect 2 misses.
            runStep(
                [this]()
                {
                    runComputePipeline("AddTransformer");
                    runComputePipeline("MulTransformer");

                    SLANG_CHECK(getStats().missCount == 2);
                    SLANG_CHECK(getStats().hitCount == 0);
                    SLANG_CHECK(getStats().entryCount == 2);
                }
            );            

            // Cache is hot and we expect 2 hits.
            runStep(
                [this]()
                {
                    runComputePipeline("AddTransformer");
                    runComputePipeline("MulTransformer");

                    SLANG_CHECK(getStats().missCount == 0);
                    SLANG_CHECK(getStats().hitCount == 2);
                    SLANG_CHECK(getStats().entryCount == 2);
                }
            );            
        }
    };

    // Same gist as the multiple entry point compute shader but with a graphics
    // shader file containing a vertex and fragment shader.
    struct ShaderCacheTestGraphics : ShaderCacheTest
    {
        struct Vertex
        {
            float position[3];
        };

        static const int kWidth = 256;
        static const int kHeight = 256;
        static const Format format = Format::R32G32B32A32_FLOAT;

        ComPtr<IBufferResource> vertexBuffer;
        ComPtr<ITextureResource> colorBuffer;
        ComPtr<IInputLayout> inputLayout;
        ComPtr<IFramebufferLayout> framebufferLayout;
        ComPtr<IRenderPassLayout> renderPass;
        ComPtr<IFramebuffer> framebuffer;

        ComPtr<IBufferResource> createVertexBuffer(IDevice* device)
        {
            const Vertex vertices[] = {
                { 0, 0, 0.5 },
                { 1, 0, 0.5 },
                { 0, 1, 0.5 },
            };

            IBufferResource::Desc vertexBufferDesc;
            vertexBufferDesc.type = IResource::Type::Buffer;
            vertexBufferDesc.sizeInBytes = sizeof(vertices);
            vertexBufferDesc.defaultState = ResourceState::VertexBuffer;
            vertexBufferDesc.allowedStates = ResourceState::VertexBuffer;
            ComPtr<IBufferResource> vertexBuffer = device->createBufferResource(vertexBufferDesc, vertices);
            SLANG_CHECK_ABORT(vertexBuffer != nullptr);
            return vertexBuffer;
        }

        ComPtr<ITextureResource> createColorBuffer(IDevice* device)
        {
            gfx::ITextureResource::Desc colorBufferDesc;
            colorBufferDesc.type = IResource::Type::Texture2D;
            colorBufferDesc.size.width = kWidth;
            colorBufferDesc.size.height = kHeight;
            colorBufferDesc.size.depth = 1;
            colorBufferDesc.numMipLevels = 1;
            colorBufferDesc.format = format;
            colorBufferDesc.defaultState = ResourceState::RenderTarget;
            colorBufferDesc.allowedStates = { ResourceState::RenderTarget, ResourceState::CopySource };
            ComPtr<ITextureResource> colorBuffer = device->createTextureResource(colorBufferDesc, nullptr);
            SLANG_CHECK_ABORT(colorBuffer != nullptr);
            return colorBuffer;
        }

        void createGraphicsResources()
        {
            VertexStreamDesc vertexStreams[] = {
                { sizeof(Vertex), InputSlotClass::PerVertex, 0 },
            };

            InputElementDesc inputElements[] = {
                // Vertex buffer data
                { "POSITION", 0, Format::R32G32B32_FLOAT, offsetof(Vertex, position), 0 },
            };
            IInputLayout::Desc inputLayoutDesc = {};
            inputLayoutDesc.inputElementCount = SLANG_COUNT_OF(inputElements);
            inputLayoutDesc.inputElements = inputElements;
            inputLayoutDesc.vertexStreamCount = SLANG_COUNT_OF(vertexStreams);
            inputLayoutDesc.vertexStreams = vertexStreams;
            inputLayout = device->createInputLayout(inputLayoutDesc);
            SLANG_CHECK_ABORT(inputLayout != nullptr);

            vertexBuffer = createVertexBuffer(device);
            colorBuffer = createColorBuffer(device);

            IFramebufferLayout::TargetLayout targetLayout;
            targetLayout.format = format;
            targetLayout.sampleCount = 1;

            IFramebufferLayout::Desc framebufferLayoutDesc;
            framebufferLayoutDesc.renderTargetCount = 1;
            framebufferLayoutDesc.renderTargets = &targetLayout;
            framebufferLayout = device->createFramebufferLayout(framebufferLayoutDesc);
            SLANG_CHECK_ABORT(framebufferLayout != nullptr);

            IRenderPassLayout::Desc renderPassDesc = {};
            renderPassDesc.framebufferLayout = framebufferLayout;
            renderPassDesc.renderTargetCount = 1;
            IRenderPassLayout::TargetAccessDesc renderTargetAccess = {};
            renderTargetAccess.loadOp = IRenderPassLayout::TargetLoadOp::Clear;
            renderTargetAccess.storeOp = IRenderPassLayout::TargetStoreOp::Store;
            renderTargetAccess.initialState = ResourceState::RenderTarget;
            renderTargetAccess.finalState = ResourceState::CopySource;
            renderPassDesc.renderTargetAccess = &renderTargetAccess;
            GFX_CHECK_CALL_ABORT(device->createRenderPassLayout(renderPassDesc, renderPass.writeRef()));

            gfx::IResourceView::Desc colorBufferViewDesc;
            memset(&colorBufferViewDesc, 0, sizeof(colorBufferViewDesc));
            colorBufferViewDesc.format = format;
            colorBufferViewDesc.renderTarget.shape = gfx::IResource::Type::Texture2D;
            colorBufferViewDesc.type = gfx::IResourceView::Type::RenderTarget;
            auto rtv = device->createTextureView(colorBuffer, colorBufferViewDesc);

            gfx::IFramebuffer::Desc framebufferDesc;
            framebufferDesc.renderTargetCount = 1;
            framebufferDesc.depthStencilView = nullptr;
            framebufferDesc.renderTargetViews = rtv.readRef();
            framebufferDesc.layout = framebufferLayout;
            GFX_CHECK_CALL_ABORT(device->createFramebuffer(framebufferDesc, framebuffer.writeRef()));
        }

        void freeGraphicsResources()
        {
            inputLayout = nullptr;
            framebufferLayout = nullptr;
            renderPass = nullptr;
            framebuffer = nullptr;
            vertexBuffer = nullptr;
            colorBuffer = nullptr;
            pipelineState = nullptr;
        }

        void createGraphicsPipeline()
        {
            ComPtr<IShaderProgram> shaderProgram;
            slang::ProgramLayout* slangReflection;
            GFX_CHECK_CALL_ABORT(
                loadGraphicsProgram(device, shaderProgram, "shader-cache-graphics", "vertexMain", "fragmentMain", slangReflection));

            GraphicsPipelineStateDesc pipelineDesc = {};
            pipelineDesc.program = shaderProgram.get();
            pipelineDesc.inputLayout = inputLayout;
            pipelineDesc.framebufferLayout = framebufferLayout;
            pipelineDesc.depthStencil.depthTestEnable = false;
            pipelineDesc.depthStencil.depthWriteEnable = false;
            GFX_CHECK_CALL_ABORT(
                device->createGraphicsPipelineState(pipelineDesc, pipelineState.writeRef()));
        }

        void dispatchGraphicsPipeline()
        {
            ComPtr<ITransientResourceHeap> transientHeap;
            ITransientResourceHeap::Desc transientHeapDesc = {};
            transientHeapDesc.constantBufferSize = 4096;
            GFX_CHECK_CALL_ABORT(
                device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

            ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
            auto queue = device->createCommandQueue(queueDesc);
            auto commandBuffer = transientHeap->createCommandBuffer();

            auto encoder = commandBuffer->encodeRenderCommands(renderPass, framebuffer);
            auto rootObject = encoder->bindPipeline(pipelineState);

            gfx::Viewport viewport = {};
            viewport.maxZ = 1.0f;
            viewport.extentX = (float)kWidth;
            viewport.extentY = (float)kHeight;
            encoder->setViewportAndScissor(viewport);

            encoder->setVertexBuffer(0, vertexBuffer);
            encoder->setPrimitiveTopology(PrimitiveTopology::TriangleList);

            encoder->draw(3);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->waitOnHost();
        }

        void runGraphicsPipeline()
        {
            createGraphicsResources();
            createGraphicsPipeline();
            dispatchGraphicsPipeline();
            freeGraphicsResources();
        }

        void runTests()
        {
            // Cache is cold and we expect 2 misses (2 entry points).
            runStep(
                [this]()
                {
                    runGraphicsPipeline();

                    SLANG_CHECK(getStats().missCount == 2);
                    SLANG_CHECK(getStats().hitCount == 0);
                    SLANG_CHECK(getStats().entryCount == 2);
                }
            );            

            // Cache is hot and we expect 2 hits.
            runStep(
                [this]()
                {
                    runGraphicsPipeline();

                    SLANG_CHECK(getStats().missCount == 0);
                    SLANG_CHECK(getStats().hitCount == 2);
                    SLANG_CHECK(getStats().entryCount == 2);
                }
            ); 
        }
    };

    // Same as ShaderCacheTestGraphics, but instead of having a singular file containing both a vertex and fragment shader, we
    // now have two separate shader files, one containing the vertex shader and the other the fragment with the same
    // names, with the expectation that we should record cache misses for both fetches.
    //
    // This test is intended to guard against the case where vertex/fragment/geometry shaders are split across
    // multiple files with the same entry point name in each file and are loaded as three separate ComponentType objects.
    // In this case, the current method for cache key generation will hash in the exact same modules, file dependencies,
    // entry point names, and entry point name overrides, resulting in the same dependency hash being returned for all three
    // and consequently, the wrong shader code being provided when the shaders are being created.
    //
    // We do not actively test geometry shaders here, but it is simply an extension of this test and should be expected
    // to behave similarly.
    struct ShaderCacheTestGraphicsSplit : ShaderCacheTestGraphics
    {
        void createGraphicsPipeline()
        {
            ComPtr<slang::ISession> slangSession;
            GFX_CHECK_CALL_ABORT(device->getSlangSession(slangSession.writeRef()));

            slang::IModule* vertexModule = slangSession->loadModule("shader-cache-graphics-vertex");
            SLANG_CHECK_ABORT(vertexModule);
            slang::IModule* fragmentModule = slangSession->loadModule("shader-cache-graphics-fragment");
            SLANG_CHECK_ABORT(fragmentModule);

            ComPtr<slang::IEntryPoint> vertexEntryPoint;
            GFX_CHECK_CALL_ABORT(
                vertexModule->findEntryPointByName("main", vertexEntryPoint.writeRef()));

            ComPtr<slang::IEntryPoint> fragmentEntryPoint;
            GFX_CHECK_CALL_ABORT(
                fragmentModule->findEntryPointByName("main", fragmentEntryPoint.writeRef()));

            Slang::List<slang::IComponentType*> componentTypes;
            componentTypes.add(vertexModule);
            componentTypes.add(fragmentModule);

            Slang::ComPtr<slang::IComponentType> composedProgram;
            GFX_CHECK_CALL_ABORT(
                slangSession->createCompositeComponentType(
                    componentTypes.getBuffer(),
                    componentTypes.getCount(),
                    composedProgram.writeRef()));

            slang::ProgramLayout* slangReflection = composedProgram->getLayout();

            Slang::List<slang::IComponentType*> entryPoints;
            entryPoints.add(vertexEntryPoint);
            entryPoints.add(fragmentEntryPoint);

            gfx::IShaderProgram::Desc programDesc = {};
            programDesc.slangGlobalScope = composedProgram.get();
            programDesc.linkingStyle = gfx::IShaderProgram::LinkingStyle::SeparateEntryPointCompilation;
            programDesc.entryPointCount = 2;
            programDesc.slangEntryPoints = entryPoints.getBuffer();

            ComPtr<IShaderProgram> shaderProgram = device->createProgram(programDesc);

            GraphicsPipelineStateDesc pipelineDesc = {};
            pipelineDesc.program = shaderProgram.get();
            pipelineDesc.inputLayout = inputLayout;
            pipelineDesc.framebufferLayout = framebufferLayout;
            pipelineDesc.depthStencil.depthTestEnable = false;
            pipelineDesc.depthStencil.depthWriteEnable = false;
            GFX_CHECK_CALL_ABORT(
                device->createGraphicsPipelineState(pipelineDesc, pipelineState.writeRef()));
        }

        void runGraphicsPipeline()
        {
            createGraphicsResources();
            createGraphicsPipeline();
            dispatchGraphicsPipeline();
            freeGraphicsResources();
        }        

        void runTests()
        {
            // Cache is cold and we expect 2 misses (2 entry points).
            runStep(
                [this]()
                {
                    runGraphicsPipeline();

                    SLANG_CHECK(getStats().missCount == 2);
                    SLANG_CHECK(getStats().hitCount == 0);
                    SLANG_CHECK(getStats().entryCount == 2);
                }
            );            

            // Cache is hot and we expect 2 hits.
            runStep(
                [this]()
                {
                    runGraphicsPipeline();

                    SLANG_CHECK(getStats().missCount == 0);
                    SLANG_CHECK(getStats().hitCount == 2);
                    SLANG_CHECK(getStats().entryCount == 2);
                }
            );         }
    };

    // Test caching of shaders that are compiled from source strings instead of files.
    struct ShaderCacheTestSourceString : ShaderCacheTest
    {
        void createComputePipeline(Slang::String shaderSource)
        {
            ComPtr<IShaderProgram> shaderProgram;
            GFX_CHECK_CALL_ABORT(loadComputeProgramFromSource(device, shaderProgram, shaderSource));

            ComputePipelineStateDesc pipelineDesc = {};
            pipelineDesc.program = shaderProgram.get();
            GFX_CHECK_CALL_ABORT(
                device->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));
        }

        void runComputePipeline(Slang::String shaderSource)
        {
            createComputeResources();
            createComputePipeline(shaderSource);
            dispatchComputePipeline();
            freeComputeResources();
        }

        void runTests()
        {
            // Cache is cold and we expect 3 misses.
            runStep(
                [this]()
                {
                    runComputePipeline(computeShaderA);
                    runComputePipeline(computeShaderB);
                    runComputePipeline(computeShaderC);

                    SLANG_CHECK(getStats().missCount == 3);
                    SLANG_CHECK(getStats().hitCount == 0);
                    SLANG_CHECK(getStats().entryCount == 3);
                }
            );

            // Cache is hot and we expect 3 hits.
            runStep(
                [this]()
                {
                    runComputePipeline(computeShaderA);
                    runComputePipeline(computeShaderB);
                    runComputePipeline(computeShaderC);

                    SLANG_CHECK(getStats().missCount == 0);
                    SLANG_CHECK(getStats().hitCount == 3);
                    SLANG_CHECK(getStats().entryCount == 3);
                }
            );
        }
    };

    template<typename T>
    void runTest(UnitTestContext* context, Slang::RenderApiFlag::Enum api)
    {
        T test;
        test.run(context, api);
    }

    SLANG_UNIT_TEST(shaderCacheBasicD3D12)
    {
        runTest<ShaderCacheTestBasic>(unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(shaderCacheBasicVulkan)
    {
        runTest<ShaderCacheTestBasic>(unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(shaderCacheEntryPointD3D12)
    {
        runTest<ShaderCacheTestEntryPoint>(unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(shaderCacheEntryPointVulkan)
    {
        runTest<ShaderCacheTestEntryPoint>(unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(shaderCacheImportIncludeD3D12)
    {
        runTest<ShaderCacheTestImportInclude>(unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(shaderCacheImportIncludeVulkan)
    {
        runTest<ShaderCacheTestImportInclude>(unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(shaderCacheSpecializationD3D12)
    {
        runTest<ShaderCacheTestSpecialization>(unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(shaderCacheSpecializationVulkan)
    {
        runTest<ShaderCacheTestSpecialization>(unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(shaderCacheGraphicsD3D12)
    {
        runTest<ShaderCacheTestGraphics>(unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(shaderCacheGraphicsVulkan)
    {
        runTest<ShaderCacheTestGraphics>(unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(shaderCacheGraphicsSplitD3D12)
    {
        runTest<ShaderCacheTestGraphicsSplit>(unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(shaderCacheGraphicsSplitVulkan)
    {
        runTest<ShaderCacheTestGraphicsSplit>(unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(shaderCacheSourceStringD3D12)
    {
        runTest<ShaderCacheTestSourceString>(unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(shaderCacheSourceStringVulkan)
    {
        runTest<ShaderCacheTestSourceString>(unitTestContext, Slang::RenderApiFlag::Vulkan);
    }
}
