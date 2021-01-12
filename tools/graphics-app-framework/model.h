// model.h
#pragma once

#include "tools/gfx/render.h"
#include "vector-math.h"
#include "slang-com-ptr.h"
#include <vector>
#include <string>

namespace gfx {

struct ModelLoader
{
    struct MaterialData
    {
        glm::vec3                   diffuseColor;
        glm::vec3                   specularColor;
        float                       specularity;

        RefPtr<TextureResource>     diffuseMap;
    };

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    typedef uint32_t Index;

    struct MeshData
    {
        int firstIndex;
        int indexCount;

        void*   material;
    };

    struct ModelData
    {
        RefPtr<BufferResource>  vertexBuffer;
        RefPtr<BufferResource>  indexBuffer;
        PrimitiveTopology       primitiveTopology;
        int                     vertexCount;
        int                     indexCount;
        int                     meshCount;
        void* const*            meshes;
    };

    struct ICallbacks
    {
        typedef ModelLoader::MaterialData MaterialData;
        typedef ModelLoader::MeshData MeshData;
        typedef ModelLoader::ModelData ModelData;

        virtual void* createMaterial(MaterialData const& data) = 0;
        virtual void* createMesh(MeshData const& data) = 0;
        virtual void* createModel(ModelData const& data) = 0;
    };

    typedef uint32_t LoadFlags;
    enum LoadFlag : LoadFlags
    {
        FlipWinding = 1 << 0,
    };

    ICallbacks*              callbacks = nullptr;
    Slang::ComPtr<IRenderer> renderer;
    LoadFlags                loadFlags = 0;
    float                    scale = 1.0f;

    Result load(char const* inputPath, void** outModel);
};


} // gfx
