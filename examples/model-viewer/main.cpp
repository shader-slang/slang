// main.cpp

//
// This example is much more involved than the `hello-world` example,
// so readers are encouraged to work through the simpler code first
// before diving into this application. We will gloss over parts of
// the code that are similar to the code in `hello-world`, and
// instead focus on the new code that is required to use Slang in
// more advanced ways.
//

// We still need to include the Slang header to use the Slang API
//
#include <slang.h>

// We will again make use of a simple graphics API abstraction
// layer, just to keep the examples short and to the point.
//
#include "gfx/model.h"
#include "gfx/render.h"
#include "gfx/render-d3d11.h"
#include "gfx/vector-math.h"
#include "gfx/window.h"
using namespace gfx;

// We will use a few utilities from the C++ standard library,
// just to keep the code short. Note that the Slang API does
// not use or require any C++ standard library features.
//
#include <memory>
#include <vector>

// A larger application will typically want to load/compile
// multiple modules/files of shader code. When using the
// Slang API, some one-time setup work can be amortized
// across multiple modules by using a single Slang
// "session" across multiple compiles.
//
// To that end, our application will use a function-`static`
// variable to create a session on demand and re-use it
// for the duration of the application.
//
SlangSession* getSlangSession()
{
    static SlangSession* slangSession = spCreateSession(NULL);
    return slangSession;
}

// This application is going to build its own layered
// application-specific abstractions on top of Slang,
// so it will have its own notion of a shader "module,"
// which comprises the results of a Slang compilation,
// including the reflection information.
//
struct ShaderModule : RefObject
{
    // The file that the module was loaded from.
    std::string                 inputPath;

    // Slang compile request and reflection data.
    SlangCompileRequest*        slangRequest;
    slang::ShaderReflection*    slangReflection;

    // Reference to the renderer, used to service requests
    // that load graphics API objects based on the module.
    RefPtr<gfx::Renderer>       renderer;
};
//
// In order to load a shader module from a `.slang` file on
// disk, we will use a Slang compile session, much like
// how the earlier Hello World example loaded shader code.
//
// We will point out major differences between the earlier
// example's `loadShaderProgram()` function, and how this function
// loads a module for reflection purposes.
//
RefPtr<ShaderModule> loadShaderModule(Renderer* renderer, char const* inputPath)
{
    auto slangSession = getSlangSession();
    SlangCompileRequest* slangRequest = spCreateCompileRequest(slangSession);

    // When *loading* the shader library, we will request that concrete
    // kernel code *not* be generated, because the module might have
    // unspecialized generic parameters. Instead, we will generate kernels
    // on demand at runtime.
    //
    spSetCompileFlags(
        slangRequest,
        SLANG_COMPILE_FLAG_NO_CODEGEN);

    // The main logic for specifying target information and loading source
    // code is the same as before with the notable change that we are *not*
    // specifying specific vertex/fragment entry points to compile here.
    //
    // Instead, the `[shader(...)]` attributes used in `shaders.slang` will
    // identify the entry points in the shader library to the compiler with
    // specific action needing to be taken in the application.
    //
    int targetIndex = spAddCodeGenTarget(slangRequest, SLANG_DXBC);
    spSetTargetProfile(slangRequest, targetIndex, spFindProfile(slangSession, "sm_4_0"));
    int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
    spAddTranslationUnitSourceFile(slangRequest, translationUnitIndex, inputPath);
    int compileErr = spCompile(slangRequest);
    if(auto diagnostics = spGetDiagnosticOutput(slangRequest))
    {
        reportError("%s", diagnostics);
    }
    if(compileErr)
    {
        spDestroyCompileRequest(slangRequest);
        spDestroySession(slangSession);
        return nullptr;
    }
    auto slangReflection = (slang::ShaderReflection*) spGetReflection(slangRequest);

    // We will not destroy the Slang compile request here, because we want to
    // keep it around to service reflection quries made from the application code.
    //
    RefPtr<ShaderModule> module = new ShaderModule();
    module->renderer = renderer;
    module->inputPath = inputPath;
    module->slangRequest = slangRequest;
    module->slangReflection = slangReflection;
    return module;
}

// Once a shader moduel has been loaded, it is possible to look up
// individual entry points by their name to get reflection information,
// including the stage for which the entry point was compiled.
//
// As with `ShaderModule` above, the `EntryPoint` type is the application's
// wrapper around a Slang entry point. In this case it caches the
// identity of the target stage as encoded for the graphics API.
//
struct EntryPoint : RefObject
{
    // Name of the entry point function
    std::string name;

    // Stage targetted by the entry point (Slang version)
    SlangStage  slangStage;

    // Stage targetted by the entry point (graphics API version)
    gfx::StageType   apiStage;
};
//
// Loading an entry point from a module is a straightforward
// application of the Slang reflection API.
//
RefPtr<EntryPoint> loadEntryPoint(
    ShaderModule*   module,
    char const*     name)
{
    auto slangReflection = module->slangReflection;

    // Look up the Slang entry point based on its name, and bail
    // out with an error if it isn't found.
    //
    auto slangEntryPoint = slangReflection->findEntryPointByName(name);
    if(!slangEntryPoint) return nullptr;

    // Extract the stage of the entry point using the Slang API,
    // and then try to map it to the corresponding stage as
    // exposed by the graphics API.
    //
    auto slangStage = slangEntryPoint->getStage();
    StageType apiStage = StageType::Unknown;
    switch(slangStage)
    {
    default:
        return nullptr;

    case SLANG_STAGE_VERTEX:    apiStage = gfx::StageType::Vertex;   break;
    case SLANG_STAGE_FRAGMENT:  apiStage = gfx::StageType::Fragment; break;
    }

    // Allocate an application object to hold on to this entry point
    // so that we can use it in later specialization steps.
    //
    RefPtr<EntryPoint> entryPoint = new EntryPoint();
    entryPoint->name = name;
    entryPoint->slangStage = slangEntryPoint->getStage();
    entryPoint->apiStage = apiStage;
    return entryPoint;
}

// In this application a `Program` represents a combination of entry
// points that will be used together (e.g., matching vertex and fragment
// entry points).
//
// Along with the entry points themselves, the `Program` object will
// cache information gleaned from Slang's reflection interface. Notably:
//
// * The number of `ParamterBlock`s that the program uses
// * Information about generic (type) parameters
//
struct Program : RefObject
{
    // The shader module that the program was loaded from.
    RefPtr<ShaderModule> shaderModule;

    // The entry points that comprise the program
    // (e.g., both a vertex and a fragment entry point).
    std::vector<RefPtr<EntryPoint>> entryPoints;

    // The number of parameter blocks that are used by the shader
    // program. This will be used by our rendering code later to
    // decide how many descriptor set bindings should affect
    // specialization/execution using this program.
    //
    int parameterBlockCount;

    // We will store information about the generic (type) parameters
    // of the program. In particular, for each generic parameter
    // we are going to find a parameter block that uses that
    // generic type parameter.
    //
    // E.g., given input code like:
    //
    //      type_param A;
    //      type_param B;
    //
    //      ParameterBlock<B>   x; // block 0
    //      ParameterBlock<Foo> y; // block 1
    //      ParameterBlock<A>   z; // block 2
    //
    // We would have two `GenericParam` entries. The first one,
    // for `A`, would store a `parameterBlockIndex` of `2`, because
    // `A` is used as the type of the `x` parameter block.
    //
    // This information will be used later when we want to specialize
    // shader code, because if `z` is bound using a `ParameterBlock<Bar>`
    // then we can infer that `A` should be bound to `Bar`.
    //
    struct GenericParam
    {
        int parameterBlockIndex;
    };
    std::vector<GenericParam>       genericParams;
};
//
// As with entry points, loading a program is done with
// the help of Slang's reflection API.
//
RefPtr<Program> loadProgram(
    ShaderModule*       module,
    int                 entryPointCount,
    const char* const*  entryPointNames)
{
    auto slangReflection = module->slangReflection;

    RefPtr<Program> program = new Program();
    program->shaderModule = module;

    // We will loop over the entry point names that were requested,
    // loading each and adding it to our program.
    //
    for(int ee = 0; ee < entryPointCount; ++ee)
    {
        auto entryPoint = loadEntryPoint(module, entryPointNames[ee]);
        if(!entryPoint)
            return nullptr;
        program->entryPoints.push_back(entryPoint);
    }

    // Next, we will look at the reflection information to see how
    // many generic type parameters were declared, and allocate
    // space in the `genericParams` array for them.
    //
    // We don't yet have enough information to fill in the
    // `parameterBlockIndex` field.
    //
    auto genericParamCount = slangReflection->getTypeParameterCount();
    for(unsigned int pp = 0; pp < genericParamCount; ++pp)
    {
        auto slangGenericParam = slangReflection->getTypeParameterByIndex(pp);

        Program::GenericParam genericParam  = {};
        program->genericParams.push_back(genericParam);
    }

    // We want to specialize our shaders based on what gets bound
    // in parameter blocks, so we will scan the shader parameters
    // looking for `ParameterBlock<G>` where `G` is one of our
    // generic type parameters.
    //
    // We do this by iterating over *all* the global shader paramters,
    // and looking for those that happen to be parameter blocks, and
    // of those the ones where the "element type" of the parameter block
    // is a generic type parameter.
    //
    auto paramCount = slangReflection->getParameterCount();
    int parameterBlockCounter = 0;
    for(unsigned int pp = 0; pp < paramCount; ++pp)
    {
        auto slangParam = slangReflection->getParameterByIndex(pp);

        // Is it a parameter block? If not, skip it.
        if(slangParam->getType()->getKind() != slang::TypeReflection::Kind::ParameterBlock)
            continue;

        // Okay, we've found another parameter block, so we can compute its zero-based index.
        int parameterBlockIndex = parameterBlockCounter++;

        // Get the element type of the parameter block, and if it isn't a generic type
        // parameter, then skip it.
        auto slangElementTypeLayout = slangParam->getTypeLayout()->getElementTypeLayout();
        if(slangElementTypeLayout->getKind() != slang::TypeReflection::Kind::GenericTypeParameter)
            continue;

        // At this point we've found a `ParameterBlock<G>` where `G` is a `type_param`,
        // so we can store the index of the parameter block back into our array of
        // generic type parameter info.
        //
        auto genericParamIndex = slangElementTypeLayout->getGenericParamIndex();
        program->genericParams[genericParamIndex].parameterBlockIndex = parameterBlockIndex;
    }

    // The above loop over the global shader parameters will have found all the
    // parameter blocks that were specified in the shader code, so now we know
    // how many parameter blocks are expected to be bound when this program is used.
    //
    program->parameterBlockCount = parameterBlockCounter;

    return program;
}
//
// As a convenience, we will define a simple wrapper around `loadProgram` for the case
// where we have just two entry points, since that is what the application actually uses.
//
RefPtr<Program> loadProgram(ShaderModule* module, char const* entryPoint0, char const* entryPoint1)
{
    char const* entryPointNames[] = { entryPoint0, entryPoint1 };
    return loadProgram(module, 2, entryPointNames);
}

// The `ParameterBlock<T>` type is supported by the Slang language and compiler,
// but it is up to each application to map it down to whatever graphics API
// abstraction is most fitting.
//
// For our application, a parameter block will be implemented as a combination
// of Slang type reflection information (to determine the layout) plus a
// graphics API descriptor set object.
//
// Note: the example graphics API abstraction we are using exposes descriptor sets
// similar to those in Vulkan, and then maps these down to efficient alternatives
// on other APIs including D3D12, D3D11, and OpenGL.
//
// Every parameter block is allocated based on a particular layout, and we
// can share the same layout across multiple blocks:
//
struct ParameterBlockLayout : RefObject
{
    // The graphics API device that should be used to allocate parameter
    // block instances.
    //
    RefPtr<gfx::Renderer>                renderer;

    // The Slang type layout information that will be used to decide
    // how much space is needed in instances of this layout.
    //
    // If the user declares a `ParameterBlock<Batman>` parameter, then
    // this will be the type layout information for `Batman`.
    //
    slang::TypeLayoutReflection*    slangTypeLayout;

    // The size of the "primary" constant buffer that will hold any
    // "ordinary" (not-resource) fields in the `slangTypeLayout` above.
    //
    size_t                          primaryConstantBufferSize;

    // API-specific layout information computes from `slangTypelayout`.
    //
    RefPtr<gfx::DescriptorSetLayout>     descriptorSetLayout;
};
//
// A parameter block layout can be computed for any `struct` type
// declared in the user's shade code. We extract the relevant
// information from the type using the Slang reflection API.
//
RefPtr<ParameterBlockLayout> getParameterBlockLayout(
    ShaderModule*   module,
    char const*     name)
{
    auto slangReflection = module->slangReflection;
    auto renderer = module->renderer;

    // Look up the type with the given name, and bail out
    // if no such type is found in the module.
    //
    auto type = slangReflection->findTypeByName(name);
    if(!type) return nullptr;

    // Request layout information for the type. Note that a single
    // type might be laid out differently for different compilation
    // targets, or based on how it is used (e.g., as a `cbuffer`
    // field vs. in a `StructuredBuffer`).
    //
    auto typeLayout = slangReflection->getTypeLayout(type);
    if(!typeLayout) return nullptr;

    // If the type that is going in the parameter block has
    // any ordinary data in it (as opposed to resources), then
    // a constant buffer will be needed to hold that data.
    //
    // In turn any resource parameters would need to go into
    // the descriptor set *after* this constant buffer.
    //
    size_t primaryConstantBufferSize = typeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM);

    // We need to use the Slang reflection information to
    // create a graphics-API-level descriptor-set layout that
    // is compatible with the original declaration.
    //
    std::vector<gfx::DescriptorSetLayout::SlotRangeDesc> slotRanges;

    // If the type has any ordinary data, then the descriptor set
    // will need a constant buffer to be the first thing it stores.
    //
    // Note: for a renderer only targetting D3D12, it might make
    // sense to allocate this "primary" constant buffer as a root
    // descriptor instead of inside the descriptor set (or at least
    // do this *if* there are no non-uniform parameters). Policy
    // decisions like that are up to the application, not Slang.
    // This example application just does something simple.
    //
    if(primaryConstantBufferSize)
    {
        slotRanges.push_back(
            gfx::DescriptorSetLayout::SlotRangeDesc(
                gfx::DescriptorSlotType::UniformBuffer));
    }

    // Next, the application will recursively walk
    // the structure of `typeLayout` to figure out what resource
    // binding ranges are required for the target API.
    //
    // TODO: This application doesn't yet use any resource parameters,
    // so we are skipping this step, but it is obviously needed
    // for a fully fleshed-out example.

    // Now that we've collected the graphics-API level binding
    // information, we can construct a graphics API descriptor set
    // layout.
    gfx::DescriptorSetLayout::Desc descriptorSetLayoutDesc;
    descriptorSetLayoutDesc.slotRangeCount = slotRanges.size();
    descriptorSetLayoutDesc.slotRanges = slotRanges.data();
    auto descriptorSetLayout = renderer->createDescriptorSetLayout(descriptorSetLayoutDesc);
    if(!descriptorSetLayout) return nullptr;

    RefPtr<ParameterBlockLayout> parameterBlockLayout = new ParameterBlockLayout();
    parameterBlockLayout->renderer = renderer;
    parameterBlockLayout->primaryConstantBufferSize = primaryConstantBufferSize;
    parameterBlockLayout->slangTypeLayout = typeLayout;
    parameterBlockLayout->descriptorSetLayout = descriptorSetLayout;
    return parameterBlockLayout;
}

// A `ParameterBlock` abstracts over the allocated storage
// for a descriptor set, based on some `ParameterBlockLayout`
//
struct ParameterBlock : RefObject
{
    // The graphics API device used to allocate this block.
    RefPtr<gfx::Renderer>           renderer;

    // The associated parameter block layout.
    RefPtr<ParameterBlockLayout>    layout;

    // The (optional) constant buffer that holds the values
    // for any ordinay fields. This will be null if
    // `layout->primaryConstantBufferSize` is zero.
    RefPtr<BufferResource>          primaryConstantBuffer;

    // The graphics-API descriptor set that provides storage
    // for any resource fields.
    RefPtr<gfx::DescriptorSet>           descriptorSet;

    // Map/unmap operations are provided to access the
    // contents of the primary constant buffer.
    void* map();
    void unmap();

    // A a convenience, `pb->mapAs<X>()` map be used as
    // a declaration of intent, instead of `(X*) pb->map()`
    template<typename T>
    T* mapAs() { return (T*)map(); }
};

// Allocating a parameter block is mostly a matter of allocating
// the required graphics API objects.
//
RefPtr<ParameterBlock> allocateParameterBlockImpl(
    ParameterBlockLayout*   layout)
{
    auto renderer = layout->renderer;

    // A descriptor set is then used to provide the storage for all
    // resource parameters (including the primary constant buffer, if any).
    //
    auto descriptorSet = renderer->createDescriptorSet(
        layout->descriptorSetLayout);

    // If the parameter block has any ordinary data, then it requires
    // a "primary" constant buffer to hold that data.
    //
    RefPtr<gfx::BufferResource> primaryConstantBuffer = nullptr;
    if(auto primaryConstantBufferSize = layout->primaryConstantBufferSize)
    {
        gfx::BufferResource::Desc bufferDesc;
        bufferDesc.init(primaryConstantBufferSize);
        bufferDesc.setDefaults(gfx::Resource::Usage::ConstantBuffer);
        bufferDesc.cpuAccessFlags = gfx::Resource::AccessFlag::Write;
        primaryConstantBuffer = renderer->createBufferResource(
            gfx::Resource::Usage::ConstantBuffer,
            bufferDesc);

        // The primary constant buffer will always be the first thing
        // stored in the descriptor set for a parameter block.
        //
        descriptorSet->setConstantBuffer(0, 0, primaryConstantBuffer);
    }

    // Now that we've allocated the graphics API objects, we can just
    // allocate our application-side wrapper object to tie everything
    // together.
    //
    RefPtr<ParameterBlock> parameterBlock = new ParameterBlock();
    parameterBlock->renderer = renderer;
    parameterBlock->layout = layout;
    parameterBlock->primaryConstantBuffer = primaryConstantBuffer;
    parameterBlock->descriptorSet = descriptorSet;
    return parameterBlock;
}

// A full-featured high-performance application would likely draw
// a distinction between "persistent" parameter blocks that are
// filled in once and then used over many frames, and "transient"
// blocks that are allocated, filled in, and discarded within
// a single frame.
//
// These two cases warrant very different allocation strategies,
// but for now we are using the same logic in both cases.
//
RefPtr<ParameterBlock> allocatePersistentParameterBlock(
    ParameterBlockLayout*   layout)
{
    return allocateParameterBlockImpl(layout);
}
RefPtr<ParameterBlock> allocateTransientParameterBlock(
    ParameterBlockLayout*   layout)
{
    return allocateParameterBlockImpl(layout);
}

// As described earlier, it is convenient to be able
// to easily map the primary constant buffer of a parameter
// block, since this will hold the values for any ordinary fields.
//
void* ParameterBlock::map()
{
    return renderer->map(
        primaryConstantBuffer,
        MapFlavor::WriteDiscard);
}
void ParameterBlock::unmap()
{
    renderer->unmap(primaryConstantBuffer);
}

// Our application code has a rudimentary material system,
// to match the `IMaterial` abstraction used in the shade code.
//
struct Material : RefObject
{
    // The key feature of a matrial in our application is that
    // it can provide a parameter block that describes it and
    // its parameters. The contents of the parameter block will
    // be any colors, textures, etc. that the material needs,
    // while the Slang type that was used to allocate the
    // block will be an implementation of `IMaterial` that
    // provides the evaluation logic for the material.

    // Each subclass of `Material` will provide a routine to
    // create a parameter block of its chosen type/layout.
    virtual RefPtr<ParameterBlock> createParameterBlock() = 0;

    // The parameter block for a material will be stashed here
    // after it is created.
    RefPtr<ParameterBlock> parameterBlock;
};

// For now we have only a single implementation of `Material`,
// which corresponds to the `SimpleMaterial` type in our shader
// code.
//
struct SimpleMaterial : Material
{
    // The `SimpleMaterial` shader type has only uniform data,
    // so we declare a `struct` type for that data here.
    struct Uniforms
    {
        glm::vec3   diffuseColor;
        float       pad;
    };
    Uniforms uniforms;

    // When asked to create a parameter block, the `SimpleMaterial`
    // type will allocate a block based on the corresponding
    // shader type, and fill it in based on the data in the C++
    // object.
    //
    RefPtr<ParameterBlock> createParameterBlock() override
    {
        auto parameterBlockLayout = gParameterBlockLayout;
        auto parameterBlock = allocatePersistentParameterBlock(
            parameterBlockLayout);

        if(auto u = parameterBlock->mapAs<Uniforms>())
        {
            *u = uniforms;
            parameterBlock->unmap();
        }

        return parameterBlock;
    }

    // We cache the corresponding parameter block layout for
    // `SimpleMaterial` in a static variable so that we don't
    // load it more than once.
    //
    static RefPtr<ParameterBlockLayout> gParameterBlockLayout;
};
RefPtr<ParameterBlockLayout> SimpleMaterial::gParameterBlockLayout;

// With the `Material` abstraction defined, we can go on to define
// the representation for loaded models that we will use.
//
// A `Model` will own vertex/index buffers, along with a list of meshes,
// while each `Mesh` will own a material and a range of indices.
// For this example we will be loading models from `.obj` files, but
// that is just a simple lowest-common-denominator choice.
//
struct Mesh : RefObject 
{
    RefPtr<Material>    material;
    int                 firstIndex;
    int                 indexCount;
};
struct Model : RefObject
{
    typedef ModelLoader::Vertex Vertex;

    RefPtr<BufferResource>      vertexBuffer;
    RefPtr<BufferResource>      indexBuffer;
    PrimitiveTopology           primitiveTopology;
    int                         vertexCount;
    int                         indexCount;
    std::vector<RefPtr<Mesh>>   meshes;
};
//
// Loading a model from disk is done with the help of some utility
// code for parsing the `.obj` file format, so that the application
// mostly just registers some callbacks to allocate the objects
// used for its representation.
//
RefPtr<Model> loadModel(
    Renderer*               renderer,
    char const*             inputPath,
    ModelLoader::LoadFlags  loadFlags = 0,
    float                   scale = 1.0f)
{
    // The model loading interface using a C++ interface of
    // callback functions to handle creating the application-specific
    // representation of meshes, materials, etc.
    //
    struct Callbacks : ModelLoader::ICallbacks
    {
        void* createMaterial(MaterialData const& data) override
        {
            SimpleMaterial* material = new SimpleMaterial();
            material->uniforms.diffuseColor = data.diffuseColor;

            material->parameterBlock = material->createParameterBlock();

            return material;
        }

        void* createMesh(MeshData const& data) override
        {
            Mesh* mesh = new Mesh();
            mesh->firstIndex = data.firstIndex;
            mesh->indexCount = data.indexCount;
            mesh->material = (Material*)data.material;
            return mesh;
        }

        void* createModel(ModelData const& data) override
        {
            Model* model = new Model();
            model->vertexBuffer = data.vertexBuffer;
            model->indexBuffer = data.indexBuffer;
            model->primitiveTopology = data.primitiveTopology;
            model->vertexCount = data.vertexCount;
            model->indexCount = data.indexCount;

            int meshCount = data.meshCount;
            for(int ii = 0; ii < meshCount; ++ii)
                model->meshes.push_back((Mesh*)data.meshes[ii]);

            return model;
        }
    };
    Callbacks callbacks;

    // We instantiate a model loader object and then use it to
    // try and load a model from the chosen path.
    //
    ModelLoader loader;
    loader.renderer = renderer;
    loader.loadFlags = loadFlags;
    loader.scale = scale;
    loader.callbacks = &callbacks;
    Model* model = nullptr;
    if(SLANG_FAILED(loader.load(inputPath, (void**)&model)))
    {
        log("failed to load '%s'\n", inputPath);
        return nullptr;
    }

    return model;
}

// The core of our application's rendering abstraction is
// the notion of an "effect," which ties together a particular
// set of shader entry points (as a `Program`), with graphics
// API state objects for the fixed-function parts of the pipeline.
//
// Note that the program here is an *unspecialized* program,
// which might have unbound global `type_param`s. Thus the
// `Effect` type here is not one-to-one with a "pipeline state
// object," because the same effect could be used to instantiate
// multiple pipeline state objects based on how things get
// specialized.
//
struct Effect : RefObject
{
    // The shader program entry point(s) to execute
    RefPtr<Program>     program;

    // Additional state corresponding to the data needed
    // to create a graphics-API pipeline state object.
    RefPtr<gfx::InputLayout>    inputLayout;
    Int                         renderTargetCount;
};

// In order to render using the `Effect` abstraction, our
// application will be creating various specialized
// shader kernels and pipeline states on-demand.
//
// We'll start with the representation of a specialized
// "variant" of an effect.
//
struct EffectVariant : RefObject
{
    // The graphics API pipeline layout and state
    // that need to be bound in order to use this
    // effect.
    //
    RefPtr<gfx::PipelineLayout>  pipelineLayout;
    RefPtr<gfx::PipelineState>   pipelineState;
};
//
// A specialized variant is created based on a base effect
// and the types that will be bound to its parameter blocks.
//
RefPtr<EffectVariant> createEffectVaraint(
    Effect*                         effect,
    UInt                            parameterBlockCount,
    ParameterBlockLayout* const*    parameterBlockLayouts)
{
    // One note to make at the very start is that the creation
    // of a specialized variant is based on the *layout* of
    // the parameter blocks in use and not on the particular
    // parameter blocks themselves. This is important because
    // it means that, e.g., two materials that use the same code,
    // but different parameter values (different textures, colors,
    // etc.) do *not* require switching between different
    // shader code or specialized PSOs.

    // We'll start by extracting some of the pieces of
    // information taht we need into local variables,
    // just to simplify the remaining code.
    //
    auto program = effect->program;
    auto shaderModule = program->shaderModule;
    auto renderer = shaderModule->renderer;

    // Our specialized effect is going to need a few things:
    //
    // 1. A specialized pipeline layout, based on the layout
    // of the bound parameter blocks.
    //
    // 2. Specialized shader kernels, based on "plugging in"
    // the parameter block types for generic type parameters
    // as needed.
    //
    // 3. A specialized pipeline state object that ties the
    // above items together with the fixed-function state
    // already specified in the effect.
    //
    // We will now go through these steps in order.

    // (1) The pipline layout (aka D3D12 "root signature") will
    // be determined based on the descriptor-set layouts
    // already cached in the given parameter block layouts.
    //
    std::vector<PipelineLayout::DescriptorSetDesc> descriptorSets;
    for(UInt pp = 0; pp < parameterBlockCount; ++pp)
    {
        descriptorSets.emplace_back(
            parameterBlockLayouts[pp]->descriptorSetLayout);
    }
    PipelineLayout::Desc pipelineLayoutDesc;
    pipelineLayoutDesc.renderTargetCount = 1;
    pipelineLayoutDesc.descriptorSetCount = descriptorSets.size();
    pipelineLayoutDesc.descriptorSets = descriptorSets.data();
    auto pipelineLayout = renderer->createPipelineLayout(pipelineLayoutDesc);

    // (2) The final shader kernels to bind will be computed
    // from the kernels we extracted into an application `EntryPoint`
    // plus the types of the bound paramter blocks, as needed.
    //
    // We will "infer" a type argument for each of the generic
    // parameters of our shader program by looking for a
    // parameter block that is declared using that generic
    // type.
    //
    std::vector<const char*> genericArgs;
    for(auto gp : program->genericParams)
    {
        int parameterBlockIndex = gp.parameterBlockIndex;
        auto typeName = parameterBlockLayouts[parameterBlockIndex]->slangTypeLayout->getName();
        genericArgs.push_back(typeName);
    }

    // Now that we are ready to generate specialized shader code,
    // we wil invoke the Slang compiler again. This time we leave
    // full code generation turned on, and we also specify the
    // entry points that we want explicitly (so that we don't
    // generate code for any other entry points).
    //
    auto slangSession = getSlangSession();
    SlangCompileRequest* slangRequest = spCreateCompileRequest(slangSession);
    int targetIndex = spAddCodeGenTarget(slangRequest, SLANG_DXBC);
    spSetTargetProfile(slangRequest, targetIndex, spFindProfile(slangSession, "sm_4_0"));
    int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
    spAddTranslationUnitSourceFile(slangRequest, translationUnitIndex, program->shaderModule->inputPath.c_str());

    int entryPointCont = program->entryPoints.size();
    for(int ii = 0; ii < entryPointCont; ++ii)
    {
        auto entryPoint = program->entryPoints[ii];

        // We are using the `spAddEntryPointEx` API so that we
        // can specify the type names to use for the generic
        // type parameters of the program.
        //
        spAddEntryPointEx(
            slangRequest,
            translationUnitIndex,
            entryPoint->name.c_str(),
            entryPoint->slangStage,
            genericArgs.size(),
            genericArgs.data());
    }

    // We expect compilation to go through without a hitch, because the
    // code was already statically checked back in `loadShaderModule()`.
    // It is still possible for errors to arise if, e.g., the application
    // tries to specialize code based on a type that doesn't implement
    // a required interface.
    //
    int compileErr = spCompile(slangRequest);
    if(auto diagnostics = spGetDiagnosticOutput(slangRequest))
    {
        reportError("%s", diagnostics);
    }
    if(compileErr)
    {
        spDestroyCompileRequest(slangRequest);
        assert(!"unexected");
        return nullptr;
    }

    // Once compilation is done we can extract the kernel code
    // for each of the entry points, and set them up for passing
    // to the graphics APIs loading logic.
    //
    std::vector<ISlangBlob*> kernelBlobs;
    std::vector<gfx::ShaderProgram::KernelDesc> kernelDescs;
    for(int ii = 0; ii < entryPointCont; ++ii)
    {
        auto entryPoint = program->entryPoints[ii];

        ISlangBlob* blob = nullptr;
        spGetEntryPointCodeBlob(slangRequest, ii, 0, &blob);

        kernelBlobs.push_back(blob);

        ShaderProgram::KernelDesc kernelDesc;

        char const* codeBegin = (char const*) blob->getBufferPointer();
        char const* codeEnd = codeBegin + blob->getBufferSize();

        kernelDesc.stage = entryPoint->apiStage;
        kernelDesc.codeBegin = codeBegin;
        kernelDesc.codeEnd = codeEnd;

        kernelDescs.push_back(kernelDesc);
    }

    // Once we've extracted the "blobs" of compiled code,
    // we are done with the Slang compilation request.
    //
    // Note that all of our reflection was performed on the unspecialized
    // shader code at load time, but we know that information is still
    // applicable to specialized kernels because of the guarantees
    // the Slang compiler makes about type layout.
    //
    spDestroyCompileRequest(slangRequest);

    // We use the graphics API to load a program into the GPU
    gfx::ShaderProgram::Desc programDesc;
    programDesc.pipelineType = gfx::PipelineType::Graphics;
    programDesc.kernels = kernelDescs.data();
    programDesc.kernelCount = kernelDescs.size();
    auto specializedProgram = renderer->createProgram(programDesc);

    // Then we unload our "blobs" of kernel code once the graphics
    // API is doen with their data.
    //
    for(auto blob : kernelBlobs)
    {
        blob->release();
    }

    // (3) We construct a full graphics API pipeline state
    // object that combines our new program and pipeline layout
    // with the other state objects from the `Effect`.
    //
    gfx::GraphicsPipelineStateDesc pipelineStateDesc = {};
    pipelineStateDesc.program = specializedProgram;
    pipelineStateDesc.pipelineLayout = pipelineLayout;
    pipelineStateDesc.inputLayout = effect->inputLayout;
    pipelineStateDesc.renderTargetCount = effect->renderTargetCount;
    auto pipelineState = renderer->createGraphicsPipelineState(pipelineStateDesc);

    RefPtr<EffectVariant> variant = new EffectVariant();
    variant->pipelineLayout = pipelineLayout;
    variant->pipelineState = pipelineState;
    return variant;
}

// A more advanced application might add logic to
// pre-populate the shader cache with shader variants
// that were compiled offline.
//
struct ShaderCache : RefObject
{
    struct VariantKey
    {
        Effect*                 effect;
        UInt                    parameterBlockCount;
        ParameterBlockLayout*   parameterBlockLayouts[8];

        // In order to be used as a hash-table key, our
        // variant key representation must support
        // equality comparison and a matching hashin function.

        bool operator==(VariantKey const& other) const
        {
            if(effect != other.effect) return false;
            if(parameterBlockCount != other.parameterBlockCount) return false;
            for( UInt ii = 0; ii < parameterBlockCount; ++ii )
            {
                if(parameterBlockLayouts[ii] != other.parameterBlockLayouts[ii]) return false;
            }
            return true;
        }

        UInt GetHashCode() const
        {
            auto hash = ::GetHashCode(effect);
            hash = combineHash(hash, ::GetHashCode(parameterBlockCount));
            for( UInt ii = 0; ii < parameterBlockCount; ++ii )
            {
                hash = combineHash(hash, ::GetHashCode(parameterBlockLayouts[ii]));
            }
            return hash;
        }
    };

    // The shader cache is mostly just a dictionary mapping
    // variant keys to the associated variant, generated on-demand.
    //
    // TODO: A more advanced application might support removing
    // entries from the shader cache when effects get unloaded,
    // or in order to respond to operations like a "hot reload"
    // key in a development build (e.g., just clear the
    // cache of variants and allow the ordinary loading logic
    // to re-populate it).
    //
    Dictionary<VariantKey, RefPtr<EffectVariant> > variants;

    // Getting a variant is just a matter of looking for an
    // existing entry in the dictionary, and creating one
    // on demand in case of a miss.
    //
    RefPtr<EffectVariant> getEffectVariant(
        VariantKey const&   key)
    {
        RefPtr<EffectVariant> variant;
        if(variants.TryGetValue(key, variant))
            return variant;

        variant = createEffectVaraint(
            key.effect,
            key.parameterBlockCount,
            key.parameterBlockLayouts);

        variants.Add(key, variant);
        return variant;
    }
};


// In order to render using the `Effect` abstraction, our
// application will use its own rendering context type
// to manage the state that it is binding. This layer
// performs a small amount of shadowing on top of the
// underlying graphics API.
//
// Note: for the purposes of our examples the "graphcis API"
// in a cross-platform abstraction over multiple APIs, but
// we do not actually advocate that real applications should
// be built in terms of distinct layers for cross-platform
// GPU API abstraction and "effect" state management.
//
// A high-performance application built on top of this approach
// would instead implement the concepts like `ParameterBlock`
// and `RenderContext` on a per-API basis, making use of
// whatever is most efficeint on that API without any
// additional abstraction layers in between.
//
// We've done things differently in this example program in
// order to avoid getting bogged down in the specifics of
// any one GPU API.
//
// With that disclaimer out of the way, let's talk through
// the `RenderContext` type in this application.
//
struct RenderContext
{
private:
    // The `RenderContext` type is used to wrap the graphics
    // API "context" or "command list" type for submission.
    // Our current abstraction layer lumps this all together
    // with the "device."
    //
    RefPtr<gfx::Renderer>               renderer;

    // We also retain a pointer to the shader cache, which
    // will be used to implement lookup of the right
    // effect variant to execute based on bound parameter
    // blocks.
    //
    RefPtr<ShaderCache>     shaderCache;

    // We will establish a small upper bound on how many
    // parameter blocks can be used simultaneously. In
    // practice, most shaders won't need more than about
    // four parameter blocks, and attempting to use more
    // than that under Vulkan can cause portability issues.
    //
    enum { kMaxParameterBlocks = 8 };

    // The overall "state" of the rendering context consists of:
    //
    // * The currently selected "effect"
    // * The parameter blocks that are used to specialize and
    //   provide parameters for that effects.
    //
    RefPtr<Effect>                  effect;
    RefPtr<ParameterBlock>          parameterBlocks[kMaxParameterBlocks];

    // Along with the retained state above, we also store
    // state in exactly the form required for looking up
    // an effect variant in our shader cache, to minimize
    // the work that needs to be done when looking up state.
    //
    ShaderCache::VariantKey variantKey;

    // When state gets changed, we track a few dirty flags rather than
    // flush changes to the GPU right away.

    // Tracks whether any state has changed in a way that requires computing
    // and binding a new GPU pipeline state object (PSO).
    //
    // E.g., changing the current effect would set this flag, but changing
    // a parameter block binding to one with a new layout would also set the flag.
    bool                    pipelineStateDirty = true;

    // The `minDirtyBlockBinding` flag tracks the lowest-numbered parameter
    // block binding that needs to be flushed to the GPU. That is, if
    // parameters blocks [0,N) have been bound to the GPU, and then the user
    // tries to set block K, then the range [0,K-1) will be left alone,
    // while the range [K,N) needs to be set again.
    //
    // This is an optimization that can be exploited on the Vulkan API
    // (and potentially others) if switching pipeline layouts doesn't invalidate
    // all currently-bound descriptor sets.
    //
    int                     minDirtyBlockBinding = 0;

    // Finally, we cache the specialized effect variant that has been
    // most recently bound to the GPU state, so that we can use the
    // information it stores (specifically the pipeline layout) when
    // binding descriptor sets.
    //
    RefPtr<EffectVariant>   currentEffectVariant;

public:
    // Initializing a render context just sets its pointer to the GPU API device
    RenderContext(
        gfx::Renderer*  renderer,
        ShaderCache*    shaderCache)
        : renderer(renderer)
        , shaderCache(shaderCache)
    {}

    void setEffect(
        Effect* inEffect)
    {
        // Bail out if nothing is changing.
        if( inEffect == effect )
            return;

        effect = inEffect;
        variantKey.effect = effect;
        variantKey.parameterBlockCount = effect->program->parameterBlockCount;

        // Binding a new effect invalidates the current state object, since
        // it will be a specialization of some other effect.
        //
        pipelineStateDirty = true;
    }

    void setParameterBlock(
        int             index,
        ParameterBlock* parameterBlock)
    {
        // Bail out if nothing is changing.
        if(parameterBlock == parameterBlocks[index])
            return;

        parameterBlocks[index] = parameterBlock;

        // This parameter block needs to be bound to the GPU, and any
        // parameter blocks after it in the list will also get re-bound
        // (even if they haven't changed). This is a reasonable choice
        // if parameter blocks are ordered based on expected frequency
        // of update (so that lower-numbered blocks change less often).
        //
        minDirtyBlockBinding = std::min(index, minDirtyBlockBinding);

        // Next, check if the layout for the block we just bound
        // is different than the one that was in place before,
        // as stored in the "variant key"
        //
        auto layout = parameterBlock->layout;
        if(layout.Ptr() == variantKey.parameterBlockLayouts[index])
            return;

        variantKey.parameterBlockLayouts[index] = layout;

        // Changing the layout of a parameter block (which includes
        // the underlying Slang type) requires computing a new
        // pipeline state object, because it may lead to differently
        // specialized code being generated.
        //
        pipelineStateDirty = true;
    }

    void flushState()
    {
        // The `flushState()` operation must be used by the application
        // any time it binds a different effect or parameter block(s),
        // to ensure that the GPU state is fully configured for rendering.
        // It is thus important that this function do as little work
        // as possible, especially in the common case where state
        // doesn't actually need to change.
        //
        // The first check we do is to see if any change might require
        // a different set of shader kernels.
        //
        if(pipelineStateDirty)
        {
            pipelineStateDirty = false;

            // Almost all of the logic for retrieving or creating
            // a new pipeline state with specialized kernels is
            // handled by our shader cache.
            //
            // In the common case, the desired variant will already
            // be present in the cache, and this function returns
            // without much effort.
            //
            auto variant = shaderCache->getEffectVariant(variantKey);

            // In order to adapt to a change in shader variant,
            // we simply bind its PSO into the GPU state, and
            // remember the variant we've selected.
            //
            renderer->setPipelineState(PipelineType::Graphics, variant->pipelineState);
            currentEffectVariant = variant;
        }

        // Even if the current pipeline state was fine, we may need to
        // bind one or more descriptor sets. We do this by walking
        // from our lowest-numbered "dirty" set up to the number
        // of sets expected by the current effect and binding them.
        //
        // If `minDirtyBlockBinding` is greater than or equal to the
        // `parameterBlockCount` of the currently bound effect, then
        // this will be a no-op.
        //
        // The common case in a tight drawing loop will be that only
        // the last block will be dirty, and we will only execute
        // one iteration of this loop.
        //
        auto program = effect->program;
        auto parameterBlockCount = program->parameterBlockCount;
        auto pipelineLayout = currentEffectVariant->pipelineLayout;
        for(int ii = minDirtyBlockBinding; ii < parameterBlockCount; ++ii)
        {
            renderer->setDescriptorSet(
                PipelineType::Graphics,
                pipelineLayout,
                ii,
                parameterBlocks[ii]->descriptorSet);
        }
        minDirtyBlockBinding = parameterBlockCount;
    }
};

// We will again structure our example application as a C++ `struct`,
// so that we can scope its allocations for easy cleanup, rather than
// use global variables.
//
struct ModelViewer {

Window* gWindow;
RefPtr<gfx::Renderer> gRenderer;
RefPtr<gfx::ResourceView> gDepthTarget;

// We keep a pointer to the one effect we are using (for a forward
// rendering pass), plus the parameter-block layouts for our `PerView`
// and `PerModel` shader types.
//
RefPtr<Effect> gEffect;
RefPtr<ParameterBlockLayout> gPerViewParameterBlockLayout;
RefPtr<ParameterBlockLayout> gPerModelParameterBlockLayout;

RefPtr<ShaderCache> shaderCache;

// Most of the application state is stored in the list of loaded models.
//
std::vector<RefPtr<Model>> gModels;

// During startup the application will load one or more models and
// add them to the `gModels` list.
//
void loadAndAddModel(
    char const*             inputPath,
    ModelLoader::LoadFlags  loadFlags = 0,
    float                   scale = 1.0f)
{
    auto model = loadModel(gRenderer, inputPath, loadFlags, scale);
    if(!model) return;
    gModels.push_back(model);
}

int gWindowWidth = 1024;
int gWindowHeight = 768;

// For this more complex example we will be passing multiple
// parameter blocks into the shader code, and each will
// need its own `struct` type the define the layout of the
// uniform data.
//
struct PerView
{
    glm::mat4x4 viewProjection;

    glm::vec3   lightDir;
    float       pad0;

    glm::vec3   lightColor;
    float       pad1;
};
struct PerModel
{
    glm::mat4x4 modelTransform;
    glm::mat4x4 inverseTransposeModelTransform;
};

// The overall initialization logic is quite similar to
// the earlier example. The biggest difference is that we
// create instances of our application-specific parameter
// block layout and effect types instead of just creating
// raw graphics API objects.
//
Result initialize()
{
    WindowDesc windowDesc;
    windowDesc.title = "Model Viewer";
    windowDesc.width = gWindowWidth;
    windowDesc.height = gWindowHeight;
    gWindow = createWindow(windowDesc);

    gRenderer = createD3D11Renderer();
    Renderer::Desc rendererDesc;
    rendererDesc.width = gWindowWidth;
    rendererDesc.height = gWindowHeight;
    gRenderer->initialize(rendererDesc, getPlatformWindowHandle(gWindow));

    InputElementDesc inputElements[] = {
        {"POSITION", 0, Format::RGB_Float32, offsetof(Model::Vertex, position) },
        {"NORMAL",   0, Format::RGB_Float32, offsetof(Model::Vertex, normal) },
        {"UV",       0, Format::RG_Float32,  offsetof(Model::Vertex, uv) },
    };
    auto inputLayout = gRenderer->createInputLayout(
        &inputElements[0],
        3);
    if(!inputLayout) return SLANG_FAIL;

    // Because we are rendering more than a single triangle this time, we
    // require a depth buffer to resolve visibility.
    //
    TextureResource::Desc depthBufferDesc = gRenderer->getSwapChainTextureDesc();
    depthBufferDesc.format = Format::D_Float32;
    depthBufferDesc.setDefaults(Resource::Usage::DepthWrite);
    auto depthTexture = gRenderer->createTextureResource(
        Resource::Usage::DepthWrite,
        depthBufferDesc);
    if(!depthTexture) return SLANG_FAIL;

    ResourceView::Desc textureViewDesc;
    textureViewDesc.type = ResourceView::Type::DepthStencil;
    auto depthTarget = gRenderer->createTextureView(depthTexture, textureViewDesc);
    if (!depthTarget) return SLANG_FAIL;

    gDepthTarget = depthTarget;

    // Unlike the earlier example, we will not generate final shader kernel
    // code during initialization. Instead, we simply load the shader module
    // so that we can perform reflection and allocate resources.
    //
    auto shaderModule = loadShaderModule(gRenderer, "shaders.slang");
    if(!shaderModule) return SLANG_FAIL;

    // Once the shader code has been loaded, we can look up types declared
    // in the shader code by name and perform reflection on them to determine
    // parameter block layouts, etc.
    //
    // A more advanced application might load this information on-demand
    // and potentially tie into an application-level reflection system
    // that already knows the string names of its types (e.g., to connect
    // the `PerView` type in shader code to the `PerView` type declared
    // in the application code).
    //
    gPerViewParameterBlockLayout = getParameterBlockLayout(
        shaderModule, "PerView");
    gPerModelParameterBlockLayout = getParameterBlockLayout(
        shaderModule, "PerModel");
    //
    // Note how we are able to load the type definition for `SimpleMaterial`
    // from the Slang shader module even though the `SimpleMaterial` type
    // is not actually *used* by any entry point in the file.
    //
    SimpleMaterial::gParameterBlockLayout = getParameterBlockLayout(
        shaderModule, "SimpleMaterial");

    // We also load a shader program based on vertex/fragment shaders in our
    // module, and then use this to create an application-level effect.
    //
    // Note that the `loadProgram` operation here does *not* invoke any
    // Slang compilation, because the shader module was already completely
    // parsed, checked, etc. by the logic in `loadShaderModule()` above.
    //
    auto program = loadProgram(shaderModule, "vertexMain", "fragmentMain");
    if(!program) return SLANG_FAIL;

    RefPtr<Effect> effect = new Effect();
    effect->program = program;
    effect->inputLayout = inputLayout;
    effect->renderTargetCount = 1;
    gEffect = effect;

    // In order to create specialized variants of the effect(s) that
    // get used for rendering, we will use a shader cache.
    //
    shaderCache = new ShaderCache();

    // Once we have created all our graphcis API and application resources,
    // we can start to load models. For now we are keeping things extremely
    // simple by using a trivial `.obj` file that can be checked into source
    // control.
    //
    // Support for loading more interesting/complex models will be added
    // to this example over time (although model loading is *not* the focus).
    //
    loadAndAddModel("cube.obj");

    showWindow(gWindow);

    return SLANG_OK;
}

// With the setup work done, we can look at the per-frame rendering
// logic to see how the application will drive the `RenderContext`
// type to perform both shader parameter binding and code specialization.
//
void renderFrame()
{
    // In order to see that things are rendering properly we need some
    // kind of animation, so we will compute a crude delta-time value here.
    //
    static uint64_t lastTime = getCurrentTime();
    uint64_t currentTime = getCurrentTime();
    float deltaTime = float(currentTime - lastTime) / float(getTimerFrequency());
    lastTime = currentTime;

    // We will use the GLM library to do the matrix math required
    // to set up our various transformation matrices.
    //
    glm::mat4x4 identity = glm::mat4x4(1.0f);

    glm::mat4x4 projection = glm::perspective(
        glm::radians(60.0f),
        float(gWindowWidth) / float(gWindowHeight),
        0.1f,
        1000.0f);

    glm::mat4x4 view = identity;
    view = translate(view, glm::vec3(0, 0, -5));

    glm::mat4x4 viewProjection = projection * view;

    // We set up a light source with a simple animation applied
    // to its direction.
    //
    glm::vec3 lightDir = normalize(glm::vec3(10, 10, -10));
    glm::vec3 lightColor = glm::vec3(1, 1, 1);
    static float angle = 0.0f;
    angle += 0.5f * deltaTime;
    glm::mat4x4 lightTransform = identity;
    lightTransform = rotate(lightTransform, angle, glm::vec3(0, 1, 0));
    lightDir = glm::vec3(lightTransform * glm::vec4(lightDir, 0));

    // Some of the basic rendering setup is identical to the previous example.
    //
    static const float kClearColor[] = { 0.25, 0.25, 0.25, 1.0 };
    gRenderer->setClearColor(kClearColor);
    gRenderer->clearFrame();
    gRenderer->setDepthStencilTarget(gDepthTarget);
    gRenderer->setPrimitiveTopology(PrimitiveTopology::TriangleList);

    // Now we will start in on the more interesting rendering logic,
    // by creating the `RenderContext` we will use for submission.
    //
    // Note: in a multi-threaded submission case, the application would
    // need to use a distinct `RenderContext` on each thread.
    //
    RenderContext context(gRenderer, shaderCache);

    // Next we set the effect that we will use for our forward rendering
    // pass. Note that an example with multiple passes would use a
    // distinct effect for each pass.
    //
    context.setEffect(gEffect);

    // We are only rendering one view, so we can fill in a per-view
    // parameter block once and use it across all draw calls.
    // This parameter block will be different every frame, so we
    // allocate a transient parameter block rather than try to
    // carefully track and re-use an allocation.
    //
    auto viewParameterBlock = allocateTransientParameterBlock(
        gPerViewParameterBlockLayout);
    if(auto perView = viewParameterBlock->mapAs<PerView>())
    {
        perView->viewProjection = viewProjection;
        perView->lightDir = lightDir;
        perView->lightColor = lightColor;

        viewParameterBlock->unmap();
    }
    //
    // Note: the assignment of indices to parameter blocks is driven
    // by their order of declaration in the shader code, so we know
    // that the per-view parameter block has index zero. Alternatively,
    // an application could use reflection API operations to look up
    // the index of a parameter block based on its name.
    //
    context.setParameterBlock(0, viewParameterBlock);

    // The majority of our rendering logic is handled as a loop
    // over the models in the scene, and their meshes.
    //
    for(auto& model : gModels)
    {
        gRenderer->setVertexBuffer(0, model->vertexBuffer, sizeof(Model::Vertex));
        gRenderer->setIndexBuffer(model->indexBuffer, Format::R_UInt32);

        // For each model we provide a parameter
        // block that holds the per-model transformation
        // parameters, corresponding to the `PerModel` type
        // in the shader code.
        //
        // Like the view parameter block, it makes sense
        // to allocate this block as a transient allocation,
        // since its contents would be different on the next
        // frame anyway.
        //
        glm::mat4x4 modelTransform = identity;
        glm::mat4x4 inverseTransposeModelTransform = inverse(transpose(modelTransform));

        auto modelParameterBlock = allocateTransientParameterBlock(
            gPerModelParameterBlockLayout);
        if(auto perModel = modelParameterBlock->mapAs<PerModel>())
        {
            perModel->modelTransform = modelTransform;
            perModel->inverseTransposeModelTransform = inverseTransposeModelTransform;

            modelParameterBlock->unmap();
        }
        context.setParameterBlock(1, modelParameterBlock);

        // Now we loop over the meshes in the model.
        //
        // A more advanced rendering loop would sort things by material
        // rather than by model, to avoid overly frequent state changes.
        // We are just doing something simple for the purposes of an
        // exmple program.
        //
        for(auto& mesh : model->meshes)
        {
            // Each mesh has a material, and each material has its own
            // parameter block that was created at load time, so we
            // can just re-use the persistent parameter block for the
            // chosen material.
            //
            // Note that binding the material parameter block here is
            // both selecting the values to use for various material
            // parameters as well as the *code* to use for material
            // evaluation (based on the concrete shader type that
            // is implementing the `IMaterial` interface).
            //
            context.setParameterBlock(
                2,
                mesh->material->parameterBlock);

            // Once we've set up all the parameter blocks needed
            // for a given drawing operation, we need to flush
            // any pending state changes (e.g., if the type of
            // material changed, a shader switch might be
            // required).
            //
            context.flushState();

            gRenderer->drawIndexed(mesh->indexCount, mesh->firstIndex);
        }
    }

    gRenderer->presentFrame();
}

void finalize()
{
    // Because we've stored a reference to some graphics API objects
    // in a class-static variable (effectively a global) we need
    // to clear those out before tearing down the application so
    // that we aren't relying on C++ global destructors to tear
    // down our application cleanly.
    //
    SimpleMaterial::gParameterBlockLayout = nullptr;
}

};

void innerMain(ApplicationContext* context)
{
    ModelViewer app;
    if(SLANG_FAILED(app.initialize()))
    {
        exitApplication(context, 1);
    }

    while(dispatchEvents(context))
    {
        app.renderFrame();
    }

    app.finalize();
}
GFX_UI_MAIN(innerMain)
