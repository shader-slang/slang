// slang-artifact.cpp
#include "slang-artifact.h"

#include "slang-artifact-info.h"

#include "../core/slang-type-text-util.h"
#include "../core/slang-io.h"
#include "../core/slang-array-view.h"

namespace Slang {

namespace { // anonymous

struct HierarchicalEnumEntry
{
    Index value;
    Index parent;
    const char* name;
};

static bool _isHierarchicalEnumOk(ConstArrayView<HierarchicalEnumEntry> entries, Count countOf)
{
    // All values should be set
    if (entries.getCount() != countOf)
    {
        return false;
    }

    List<uint8_t> isUsed;
    isUsed.setCount(countOf);
    ::memset(isUsed.getBuffer(), 0, countOf);

    for (const auto& entry : entries)
    {
        const auto value = entry.value;
        // Must be in range
        if (value < 0 || value >= countOf)
        {
            return false;
        }

        if (isUsed[value] != 0)
        {
            return false;
        }
        // Mark as used
        isUsed[value]++;
    }

    // There can't be any gaps
    for (auto v : isUsed)
    {
        if (v == 0)
        {
            return false;
        }
    }

    // Okay, looks reasonable..
    return true;
}

template <typename T>
struct HierarchicalEnumTable
{
    HierarchicalEnumTable(ConstArrayView<HierarchicalEnumEntry> entries)
    {
        // Remove warnings around this not being used.
        {
            const auto unused = _isHierarchicalEnumOk;
            SLANG_UNUSED(unused);
        }

        SLANG_COMPILE_TIME_ASSERT(Index(T::Invalid) < Index(T::Base));
        SLANG_ASSERT(entries.getCount() == Count(T::CountOf));

        SLANG_ASSERT(_isHierarchicalEnumOk(entries, Count(T::CountOf)));

        ::memset(&m_parents, 0, sizeof(m_parents));

        for (const auto& entry : entries)
        {
            const auto value = entry.value;
            m_parents[value] = T(entry.parent);
            m_names[value] = UnownedStringSlice(entry.name);
        }

        // TODO(JS): NOTE! If we wanted to use parent to indicate if a value was *invalid* 
        // we would want the Parent of Base to be Base.
        //
        // Base parent should be invalid
        SLANG_ASSERT(getParent(T::Base) == T::Invalid);
        // Invalids parent should be invalid
        SLANG_ASSERT(getParent(T::Invalid) == T::Invalid);
    }

    T getParent(T kind) const
    {
        return (kind >= T::CountOf) ?
            T::Invalid :
            m_parents[Index(kind)];
    }
    UnownedStringSlice getName(T kind) const
    {
        return (kind >= T::CountOf) ?
            UnownedStringSlice() :
            m_names[Index(kind)];
    }

    bool isDerivedFrom(T type, T base) const
    {
        if (Index(type) >= Index(T::CountOf))
        {
            return false;
        }

        do
        {
            if (type == base)
            {
                return true;
            }
            type = m_parents[Index(type)];
        } while (Index(type) > Index(T::Base));

        return false;
    }

protected:
    T m_parents[Count(T::CountOf)];
    UnownedStringSlice m_names[Count(T::CountOf)];
};

} // anonymous

// Macro utils to create "enum hierarchy" tables

#define SLANG_HIERARCHICAL_ENUM_GET_VALUES(ENUM_TYPE, ENUM_TYPE_MACRO, ENUM_ENTRY_MACRO) \
static ConstArrayView<HierarchicalEnumEntry> _getEntries##ENUM_TYPE() \
{ \
    static const HierarchicalEnumEntry values[] = { ENUM_TYPE_MACRO(ENUM_ENTRY_MACRO) }; \
    return makeConstArrayView(values); \
}

#define SLANG_HIERARCHICAL_ENUM(ENUM_TYPE, ENUM_TYPE_MACRO, ENUM_VALUE_MACRO) \
SLANG_HIERARCHICAL_ENUM_GET_VALUES(ENUM_TYPE, ENUM_TYPE_MACRO, ENUM_VALUE_MACRO) \
\
static const HierarchicalEnumTable<ENUM_TYPE> g_table##ENUM_TYPE(_getEntries##ENUM_TYPE()); \
\
ENUM_TYPE getParent(ENUM_TYPE kind) { return g_table##ENUM_TYPE.getParent(kind); } \
UnownedStringSlice getName(ENUM_TYPE kind) { return g_table##ENUM_TYPE.getName(kind); } \
bool isDerivedFrom(ENUM_TYPE kind, ENUM_TYPE base) { return g_table##ENUM_TYPE.isDerivedFrom(kind, base); }

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactKind !!!!!!!!!!!!!!!!!!!!!!! */

#define SLANG_ARTIFACT_KIND(x) \
    x(Invalid, Invalid) \
    x(Base, Invalid) \
        x(None, Base) \
        x(Unknown, Base) \
        x(Container, Base) \
            x(Zip, Container) \
            x(Riff, Container) \
        x(Text, Base) \
            x(HumanText, Text) \
            x(Source, Text) \
            x(Assembly, Text) \
        x(BinaryLike, Base) \
            x(ObjectCode, BinaryLike) \
            x(Library, BinaryLike) \
            x(Executable, BinaryLike) \
            x(SharedLibrary, BinaryLike) \
            x(HostCallable, BinaryLike) \
        \
        x(DebugInfo, Base) \
        x(Diagnostics, Base)

#define SLANG_ARTIFACT_KIND_ENTRY(TYPE, PARENT) { Index(ArtifactKind::TYPE), Index(ArtifactKind::PARENT), #TYPE },

SLANG_HIERARCHICAL_ENUM(ArtifactKind, SLANG_ARTIFACT_KIND, SLANG_ARTIFACT_KIND_ENTRY)

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactPayload !!!!!!!!!!!!!!!!!!!!!!! */

#define SLANG_ARTIFACT_PAYLOAD(x) \
    x(Invalid, Invalid) \
    x(Base, Invalid) \
        x(None, Base) \
        x(Unknown, Base) \
        x(Source, Base) \
            x(C, Source) \
            x(Cpp, Source) \
            x(HLSL, Source) \
            x(GLSL, Source) \
            x(CUDA, Source) \
            x(Metal, Source) \
            x(Slang, Source) \
        x(KernelLike, Base) \
            x(DXIL, KernelLike) \
            x(DXBC, KernelLike) \
            x(SPIRV, KernelLike) \
            x(PTX, KernelLike) \
            x(CuBin, KernelLike) \
            x(MetalAIR, KernelLike) \
        x(CPULike, Base) \
            x(UnknownCPU, CPULike) \
            x(X86, CPULike) \
            x(X86_64, CPULike) \
            x(Aarch, CPULike) \
            x(Aarch64, CPULike) \
            x(HostCPU, CPULike) \
            x(UniversalCPU, CPULike) \
        x(GeneralIR, Base) \
            x(SlangIR, GeneralIR) \
            x(LLVMIR, GeneralIR) \
        x(AST, Base) \
            x(SlangAST, AST)

#define SLANG_ARTIFACT_PAYLOAD_ENTRY(TYPE, PARENT) { Index(ArtifactPayload::TYPE), Index(ArtifactPayload::PARENT), #TYPE },

SLANG_HIERARCHICAL_ENUM(ArtifactPayload, SLANG_ARTIFACT_PAYLOAD, SLANG_ARTIFACT_PAYLOAD_ENTRY)

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactStyle !!!!!!!!!!!!!!!!!!!!!!! */

#define SLANG_ARTIFACT_STYLE(x) \
    x(Invalid, Invalid) \
    x(Base, Invalid) \
        x(Unknown, Base) \
        x(Kernel, Base) \
        x(Host, Base) 

#define SLANG_ARTIFACT_STYLE_ENTRY(TYPE, PARENT) { Index(ArtifactStyle::TYPE), Index(ArtifactStyle::PARENT), #TYPE },

SLANG_HIERARCHICAL_ENUM(ArtifactStyle, SLANG_ARTIFACT_STYLE, SLANG_ARTIFACT_STYLE_ENTRY)


/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactDesc !!!!!!!!!!!!!!!!!!!!!!! */

/* static */ArtifactDesc ArtifactDesc::makeFromCompileTarget(SlangCompileTarget target)
{
    switch (target)
    {
        case SLANG_TARGET_UNKNOWN:                return make(Kind::Unknown, Payload::None, Style::Unknown, 0);
        case SLANG_TARGET_NONE:                   return make(Kind::None, Payload::None, Style::Unknown, 0);
        case SLANG_GLSL_VULKAN:
        case SLANG_GLSL_VULKAN_ONE_DESC:
        case SLANG_GLSL:
        {
            // For the moment we make all just map to GLSL, but we could use flags
            // or some other mechanism to distinguish the types
            return make(Kind::Source, Payload::GLSL, Style::Kernel, 0);
        }
        case SLANG_HLSL:                    return make(Kind::Source, Payload::HLSL, Style::Kernel, 0);
        case SLANG_SPIRV:                   return make(Kind::Executable, Payload::SPIRV, Style::Kernel, 0);
        case SLANG_SPIRV_ASM:               return make(Kind::Assembly, Payload::SPIRV, Style::Kernel, 0);
        case SLANG_DXBC:                    return make(Kind::Executable, Payload::DXBC, Style::Kernel, 0);
        case SLANG_DXBC_ASM:                return make(Kind::Assembly, Payload::DXBC, Style::Kernel, 0);
        case SLANG_DXIL:                    return make(Kind::Executable, Payload::DXIL, Style::Kernel, 0);
        case SLANG_DXIL_ASM:                return make(Kind::Assembly, Payload::DXIL, Style::Kernel, 0);
        case SLANG_C_SOURCE:                return make(Kind::Source, Payload::C, Style::Kernel, 0);
        case SLANG_CPP_SOURCE:              return make(Kind::Source, Payload::Cpp, Style::Kernel, 0);
        case SLANG_HOST_CPP_SOURCE:         return make(Kind::Source, Payload::Cpp, Style::Host, 0);
        case SLANG_HOST_EXECUTABLE:         return make(Kind::Executable, Payload::HostCPU, Style::Host, 0);
        case SLANG_SHADER_SHARED_LIBRARY:   return make(Kind::SharedLibrary, Payload::HostCPU, Style::Kernel, 0);
        case SLANG_SHADER_HOST_CALLABLE:    return make(Kind::HostCallable, Payload::HostCPU, Style::Kernel, 0);
        case SLANG_CUDA_SOURCE:             return make(Kind::Source, Payload::CUDA, Style::Kernel, 0);
        // TODO(JS):
        // Not entirely clear how best to represent PTX here. We could mark as 'Assembly'. Saying it is 
        // 'Executable' implies it is Binary (which PTX isn't). Executable also implies 'complete for executation', 
        // irrespective of it being text.
        case SLANG_PTX:                     return make(Kind::Executable, Payload::PTX, Style::Kernel, 0);
        case SLANG_OBJECT_CODE:             return make(Kind::ObjectCode, Payload::HostCPU, Style::Kernel, 0);
        case SLANG_HOST_HOST_CALLABLE:      return make(Kind::HostCallable, Payload::HostCPU, Style::Host, 0);
        default: break;
    }

    SLANG_UNEXPECTED("Unhandled type");
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactList !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* ArtifactList::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() ||
        guid == ICastable::getTypeGuid() ||
        guid == IArtifactList::getTypeGuid())
    {
        return static_cast<IArtifactList*>(this);
    }
    return nullptr;
}

void* ArtifactList::getObject(const Guid& guid)
{
    // For now we can't cast to an object
    SLANG_UNUSED(guid);
    return nullptr;
}

void* ArtifactList::castAs(const Guid& guid)
{
    if (auto intf = getInterface(guid))
    {
        return intf;
    }
    return getObject(guid);
}

void ArtifactList::add(IArtifact* artifact)
{
    // Must be set
    SLANG_ASSERT(artifact);
    // Can't already be in the list
    SLANG_ASSERT(m_artifacts.indexOf(artifact) < 0);
    // Can't have another owner
    SLANG_ASSERT(artifact->getParent() == nullptr);

    // Set the parent
    artifact->setParent(m_parent);

    // Add
    m_artifacts.add(ComPtr<IArtifact>(artifact));
}

void ArtifactList::removeAt(Index index) 
{
   IArtifact* artifact = m_artifacts[index];
   artifact->setParent(nullptr);
   m_artifacts.removeAt(index); 
}

void ArtifactList::clear()
{
    _setParent(nullptr);
    m_artifacts.clear();
}

void ArtifactList::_setParent(IArtifact* parent)
{
    if (m_parent == parent)
    {
        return;
    }

    for (IArtifact* artifact : m_artifacts)
    {
        artifact->setParent(artifact);
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Artifact !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* Artifact::getInterface(const Guid& uuid)
{
    if (uuid == ISlangUnknown::getTypeGuid() || uuid == IArtifact::getTypeGuid())
    {
        return static_cast<IArtifact*>(this);
    }
    return nullptr;
}

Artifact::~Artifact()
{
    // Remove the temporary
    if (m_pathType == PathType::Temporary)
    {
        File::remove(m_path);
    }
    // If there is a temporary lock path, remove that
    if (m_temporaryLockPath.getLength())
    {
        File::remove(m_temporaryLockPath);
    }
}

bool Artifact::exists()
{
    // If we have a blob it exists
    if (m_blob)
    {
        return true;
    }

    // If we have an associated entry that represents the artifact it exists
    if (findElement(IArtifactInstance::getTypeGuid()))
    {
        return true;
    }

    // If we don't have a path then it can't exist
    if (m_pathType == PathType::None)
    {
        return false;
    }

    // If the file exists we assume it exists
    return File::exists(m_path);
}

ISlangUnknown* Artifact::findElement(const Guid& guid)
{
    for (auto const& element : m_elements)
    {
        ISlangUnknown* value = element.value;

        ISlangUnknown* intf = nullptr;
        if (SLANG_SUCCEEDED(value->queryInterface(guid, (void**)&intf)) && intf)
        {
            // NOTE! This assumes we *DONT* need to ref count to keep an interface in scope
            // (as strict COM requires so as to allow on demand interfaces).
            intf->release();
            return intf;
        }
    }

    return nullptr;
}

void Artifact::addElement(const Desc& desc, ISlangUnknown* intf) 
{ 
    SLANG_ASSERT(intf); 
    Element element{ desc, ComPtr<ISlangUnknown>(intf) };
    m_elements.add(element); 
}

void Artifact::removeElementAt(Index i)
{
    m_elements.removeAt(i);
}

void* Artifact::findElementObject(const Guid& classGuid)
{
    ComPtr<IArtifactInstance> instance;
    for (const auto& element : m_elements)
    {
        ISlangUnknown* value = element.value;

        if (SLANG_SUCCEEDED(value->queryInterface(IArtifactInstance::getTypeGuid(), (void**)instance.writeRef())) && instance)
        {
            void* classInstance = instance->castAs(classGuid);
            if (classInstance)
            {
                return classInstance;
            }
        }
    }

    return nullptr;
}

SlangResult Artifact::requireFileLike(Keep keep)
{
    // If there is no path set and no blob we still need a name. 
    // If the artifact is a library we can assume it's a system level library, 
    // or it can be found by appropriate search paths. 
    if (m_pathType == PathType::None && 
        m_blob == nullptr && 
        (m_desc.kind == ArtifactKind::Library || 
         m_desc.kind == ArtifactKind::SharedLibrary))
    {
        if (m_name.getLength() > 0)
        {
            return SLANG_OK;
        }

        // TODO(JS): If we could serialize, we could turn some other representation into a file, and therefore 
        // a name, but currently that's not supported
        return SLANG_E_NOT_FOUND;
    }

    // Will turn into a file if necessary
    SLANG_RETURN_ON_FAIL(requireFile(keep));
    return SLANG_OK;
}

SlangResult Artifact::requireFile(Keep keep)
{
    if (m_pathType != PathType::None)
    {
        return SLANG_OK;
    }

    ComPtr<ISlangBlob> blob;

    // Get the contents as a blob. If we can't do that, then we can't write anything...
    SLANG_RETURN_ON_FAIL(loadBlob(getIntermediateKeep(keep), blob.writeRef()));

    // If we have a name, make the generated name based on that name
    // Else just use 'slang-generated' the basis

    UnownedStringSlice nameBase;
    if (m_name.getLength() > 0)
    {
        nameBase = m_name.getUnownedSlice();
    }
    else
    {
        nameBase = UnownedStringSlice::fromLiteral("slang-generated");
    }

    // TODO(JS): NOTE! This isn't strictly correct, as the generated filename is not guarenteed to be unique
    // if we change it with an extension (or prefix).
    // This doesn't change the previous behavior though.
    String temporaryLockPath;
    SLANG_RETURN_ON_FAIL(File::generateTemporary(nameBase, temporaryLockPath));

    String path = temporaryLockPath;

    if (ArtifactInfoUtil::isCpuBinary(m_desc) && 
        (m_desc.kind == ArtifactKind::SharedLibrary ||
         m_desc.kind == ArtifactKind::Library))
    {
        const bool isSharedLibraryPrefixPlatform = SLANG_LINUX_FAMILY || SLANG_APPLE_FAMILY;
        if (isSharedLibraryPrefixPlatform)
        {
            StringBuilder buf;
            buf << "lib";
            buf << Path::getFileName(path);

            auto parentDir = Path::getParentDirectory(path);
            if (parentDir.getLength())
            {
                // Combine the name with path if their is a parent 
                path = Path::combine(parentDir, buf);
            }
            else
            {
                // Just use the name as is
                path = buf;
            }
        }
    }

    // If there is an extension append it
    const UnownedStringSlice ext = ArtifactInfoUtil::getDefaultExtension(m_desc);

    if (ext.getLength())
    {
        path.appendChar('.');
        path.append(ext);
    }

    // If the final path is different from the lock path save that path
    if (path != temporaryLockPath)
    {
        m_temporaryLockPath = temporaryLockPath;
    }

    // Write the contents
    SLANG_RETURN_ON_FAIL(File::writeAllBytes(path, blob->getBufferPointer(), blob->getBufferSize()));

    // Okay we can now add this as temporary path too
    _setPath(PathType::Temporary, path);

    return SLANG_OK;
}

SlangResult Artifact::loadBlob(Keep keep, ISlangBlob** outBlob)
{
    ComPtr<ISlangBlob> blob(m_blob);

    if (!blob)
    {
        if (m_pathType != PathType::None)
        {
            // Read into a blob
            ScopedAllocation alloc;
            SLANG_RETURN_ON_FAIL(File::readAllBytes(m_path, alloc));

            // Create as a blob
            blob = RawBlob::moveCreate(alloc);
        }
        else
        {
            // Look for a representation that we can serialize into a blob
            for (const auto& element : m_elements)
            {
                ISlangUnknown* intf = element.value;

                ComPtr<IArtifactInstance> inst;
                if (SLANG_SUCCEEDED(intf->queryInterface(IArtifactInstance::getTypeGuid(), (void**)inst.writeRef())) && inst)
                {
                    SlangResult res = inst->writeToBlob(blob.writeRef());
                    if (SLANG_SUCCEEDED(res) && blob)
                    {
                        break;
                    }
                }
            }
        }

        // Wasn't able to construct
        if (!blob)
        {
            return SLANG_E_NOT_FOUND;
        }

        // Put in cache 
        if (canKeep(keep))
        {
            setBlob(blob);
        }
    }

    *outBlob = blob.detach();
    return SLANG_OK;
}

} // namespace Slang
