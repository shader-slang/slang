#pragma once

#include "slang-gfx.h"
#include "core/slang-basic.h"
#include "core/slang-com-object.h"
#include "renderer-shared.h"

namespace gfx
{
    class ShaderObjectLayoutBase;

    template<typename T>
    class VersionedObjectPool
    {
    public:
        struct ObjectVersion
        {
            Slang::RefPtr<T> object;
            Slang::RefPtr<TransientResourceHeapBase> transientHeap;
            uint64_t transientHeapVersion;
            bool canRecycle()
            {
                return (transientHeap->getVersion() != transientHeapVersion);
            }
        };
        Slang::List<ObjectVersion> objects;
        SlangInt lastAllocationIndex = -1;
        ObjectVersion& allocate(TransientResourceHeapBase* currentTransientHeap)
        {
            for (SlangInt i = 0; i < objects.getCount(); i++)
            {
                auto& object = objects[i];
                if (object.canRecycle())
                {
                    object.transientHeap = currentTransientHeap;
                    object.transientHeapVersion = currentTransientHeap->getVersion();
                    lastAllocationIndex = i;
                    return object;
                }
            }
            ObjectVersion v;
            v.transientHeap = currentTransientHeap;
            v.transientHeapVersion = currentTransientHeap->getVersion();
            objects.add(v);
            lastAllocationIndex = objects.getCount() - 1;
            return objects.getLast();
        }
        ObjectVersion& getLastAllocation() { return objects[lastAllocationIndex]; }
    };

    class MutableShaderObjectData
    {
    public:
        // Any "ordinary" / uniform data for this object
        Slang::List<char> m_ordinaryData;

        bool m_dirty = true;

        Slang::Index getCount() { return m_ordinaryData.getCount(); }
        void setCount(Slang::Index count) { m_ordinaryData.setCount(count); }
        char* getBuffer() { return m_ordinaryData.getBuffer(); }
        void markDirty() { m_dirty = true; }

        // We don't actually create any GPU buffers here, since they will be handled
        // by the immutable shader objects once the user calls `getCurrentVersion`.
        ResourceViewBase* getResourceView(
            RendererBase* device,
            slang::TypeLayoutReflection* elementLayout,
            slang::BindingType bindingType)
        {
            return nullptr;
        }
    };

    template<typename TShaderObject, typename TShaderObjectLayoutImpl>
    class MutableShaderObject : public ShaderObjectBaseImpl<
        TShaderObject,
        TShaderObjectLayoutImpl,
        MutableShaderObjectData>
    {
        typedef ShaderObjectBaseImpl<
            TShaderObject,
            TShaderObjectLayoutImpl,
            MutableShaderObjectData> Super;
    protected:
        Slang::OrderedDictionary<ShaderOffset, Slang::RefPtr<ResourceViewBase>> m_resources;
        Slang::OrderedDictionary<ShaderOffset, Slang::RefPtr<SamplerStateBase>> m_samplers;
        Slang::OrderedHashSet<ShaderOffset> m_objectOffsets;
        VersionedObjectPool<ShaderObjectBase> m_shaderObjectVersions;
        bool m_dirty = true;
        bool isDirty()
        {
            if (m_dirty) return true;
            if (this->m_data.m_dirty) return true;
            for (auto& object : this->m_objects)
            {
                if (object && object->isDirty())
                    return true;
            }
            return false;
        }
        
        void markDirty()
        {
            m_dirty = true;
        }
    public:
        Result init(RendererBase* device, ShaderObjectLayoutBase* layout)
        {
            this->m_device = device;
            auto layoutImpl = static_cast<TShaderObjectLayoutImpl*>(layout);
            this->m_layout = layoutImpl;
            Slang::Index subObjectCount = layoutImpl->getSubObjectCount();
            this->m_objects.setCount(subObjectCount);
            return SLANG_OK;
        }
    public:
        virtual SLANG_NO_THROW Result SLANG_MCALL setData(ShaderOffset const& offset, void const* data, size_t size) override
        {
            if (!size) return SLANG_OK;
            if (SlangInt(offset.uniformOffset + size) > this->m_data.getCount())
                this->m_data.setCount(offset.uniformOffset + size);
            memcpy(this->m_data.getBuffer() + offset.uniformOffset, data, size);
            this->m_data.markDirty();
            markDirty();
            return SLANG_OK;
        }

        virtual SLANG_NO_THROW Result SLANG_MCALL
            setObject(ShaderOffset const& offset, IShaderObject* object) override
        {
            Super::setObject(offset, object);
            m_objectOffsets.Add(offset);
            markDirty();
            return SLANG_OK;
        }

        virtual SLANG_NO_THROW Result SLANG_MCALL setResource(ShaderOffset const& offset, IResourceView* resourceView) override
        {
            m_resources[offset] = static_cast<ResourceViewBase*>(resourceView);
            markDirty();
            return SLANG_OK;
        }

        virtual SLANG_NO_THROW Result SLANG_MCALL setSampler(ShaderOffset const& offset, ISamplerState* sampler) override
        {
            m_samplers[offset] = static_cast<SamplerStateBase*>(sampler);
            markDirty();
            return SLANG_OK;
        }

        virtual SLANG_NO_THROW Result SLANG_MCALL setCombinedTextureSampler(ShaderOffset const& offset, IResourceView* textureView, ISamplerState* sampler) override
        {
            m_samplers[offset] = static_cast<SamplerStateBase*>(sampler);
            m_resources[offset] = static_cast<ResourceViewBase*>(textureView);
            markDirty();
            return SLANG_OK;
        }

        virtual SLANG_NO_THROW Result SLANG_MCALL getCurrentVersion(
            ITransientResourceHeap* transientHeap, IShaderObject** outObject) override
        {
            if (!isDirty())
            {
                returnComPtr(outObject, getLastAllocatedShaderObject());
                return SLANG_OK;
            }

            RefPtr<ShaderObjectBase> object =
                allocateShaderObject(static_cast<TransientResourceHeapBase*>(transientHeap));
            SLANG_RETURN_ON_FAIL(object->setData(ShaderOffset(), m_data.getBuffer(), m_data.getCount()));
            for (auto res : m_resources)
                SLANG_RETURN_ON_FAIL(object->setResource(res.Key, res.Value));
            for (auto sampler : m_samplers)
                SLANG_RETURN_ON_FAIL(object->setSampler(sampler.Key, sampler.Value));
            for (auto offset : m_objectOffsets)
            {
                if (offset.bindingRangeIndex < 0)
                    return SLANG_E_INVALID_ARG;
                auto layout = this->getLayout();
                if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
                    return SLANG_E_INVALID_ARG;
                auto bindingRange = layout->getBindingRange(offset.bindingRangeIndex);

                auto subObject = this->m_objects[bindingRange.subObjectIndex + offset.bindingArrayIndex];
                if (subObject)
                {
                    ComPtr<IShaderObject> subObjectVersion;
                    SLANG_RETURN_ON_FAIL(subObject->getCurrentVersion(transientHeap, subObjectVersion.writeRef()));
                    SLANG_RETURN_ON_FAIL(object->setObject(offset, subObjectVersion));
                }
            }
            m_dirty = false;
            m_data.m_dirty = false;
            returnComPtr(outObject, object);
            return SLANG_OK;
        }
    public:
        Slang::RefPtr<ShaderObjectBase> allocateShaderObject(TransientResourceHeapBase* transientHeap)
        {
            auto& version = m_shaderObjectVersions.allocate(transientHeap);
            if (!version.object)
            {
                ComPtr<IShaderObject> shaderObject;
                SLANG_RETURN_NULL_ON_FAIL(this->m_device->createShaderObject(this->m_layout, shaderObject.writeRef()));
                version.object = static_cast<ShaderObjectBase*>(shaderObject.get());
            }
            return version.object;
        }
        Slang::RefPtr<ShaderObjectBase> getLastAllocatedShaderObject()
        {
            return m_shaderObjectVersions.getLastAllocation().object;
        }
    };
}
