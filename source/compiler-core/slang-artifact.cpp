// slang-artifact.cpp
#include "slang-artifact.h"

#include "slang-artifact-info.h"

#include "../core/slang-file-system.h"

#include "../core/slang-type-text-util.h"
#include "../core/slang-io.h"
#include "../core/slang-array-view.h"

#include "slang-artifact-util.h"

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

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FileArtifactRepresentation !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* FileArtifactRepresentation::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() ||
        guid == ICastable::getTypeGuid() ||
        guid == IArtifactRepresentation::getTypeGuid() ||
        guid == IFileArtifactRepresentation::getTypeGuid())
    {
        return static_cast<IFileArtifactRepresentation*>(this);
    }
    return nullptr;
}

void* FileArtifactRepresentation::getObject(const Guid& guid)
{
    SLANG_UNUSED(guid);
    return nullptr;
}

ISlangMutableFileSystem* FileArtifactRepresentation::_getFileSystem()
{
    return m_fileSystem ? m_fileSystem : OSFileSystem::getMutableSingleton();
}

void* FileArtifactRepresentation::castAs(const Guid& guid)
{
    if (auto intf = getInterface(guid))
    {
        return intf;
    }
    return getObject(guid);
}

SlangResult FileArtifactRepresentation::writeToBlob(ISlangBlob** blob)
{
    if (m_kind == Kind::NameOnly)
    {
        // If it's referenced by a name only, it's a file that *can't* be loaded as a blob in general.
        return SLANG_E_NOT_AVAILABLE;
    }

    auto fileSystem = _getFileSystem();
    return fileSystem->loadFile(m_path.getBuffer(), blob);
}

bool FileArtifactRepresentation::exists()
{
    // TODO(JS):
    // If it's a name only it's hard to know what exists should do. It can't *check* because it relies on the 'system' doing 
    // the actual location. We could ask the IArtifactUtil, and that could change the behavior.
    // For now we just assume it does.
    if (m_kind == Kind::NameOnly)
    {
        return true;
    }

    auto fileSystem = _getFileSystem();

    SlangPathType pathType;
    const auto res = fileSystem->getPathType(m_path.getBuffer(), &pathType);

    // It exists if it is a file
    return SLANG_SUCCEEDED(res) && pathType == SLANG_PATH_TYPE_FILE;
}

FileArtifactRepresentation::~FileArtifactRepresentation()
{
    if (m_kind == Kind::Owned)
    {
        auto fileSystem = _getFileSystem();
        fileSystem->remove(m_path.getBuffer());
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! LockFile !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* LockFile::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() ||
        guid == ICastable::getTypeGuid() ||
        guid == ILockFile::getTypeGuid())
    {
        return static_cast<ILockFile*>(this);
    }
    return nullptr;
}

void* LockFile::getObject(const Guid& guid)
{
    SLANG_UNUSED(guid);
    return nullptr;
}

ISlangMutableFileSystem* LockFile::_getFileSystem()
{
    return m_fileSystem ? m_fileSystem : OSFileSystem::getMutableSingleton();
}

void* LockFile::castAs(const Guid& guid)
{
    if (auto intf = getInterface(guid))
    {
        return intf;
    }
    return getObject(guid);
}

const char* LockFile::getPath()
{
    return (m_path.getLength() > 0) ? m_path.getBuffer() : nullptr;
}

ISlangMutableFileSystem* LockFile::getFileSystem()
{
    return m_fileSystem;
}

LockFile::~LockFile()
{
    if (m_path.getLength() > 0)
    {
        auto fileSystem = _getFileSystem();
        fileSystem->remove(m_path.getBuffer());
    }
}

void LockFile::disown()
{
    m_path = String();
    m_fileSystem.setNull();
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

bool Artifact::exists()
{
    for (ISlangUnknown* item : m_items)
    {
        ComPtr<ICastable> castable;

        if (SLANG_SUCCEEDED(item->queryInterface(ICastable::getTypeGuid(), (void**)castable.writeRef())) && castable)
        {
            auto rep = as<IArtifactRepresentation>(castable);
            if (rep)
            {
                // It is a rep and it exists
                if (rep->exists())
                {
                    return true;
                }
                continue;
            }
            // Associated types don't encapsulate an artifact representation, so don't signal existance
            if (as<IArtifactAssociated>(castable))
            {
                continue;
            }
        }
        
        // It can't be IArtifactRepresentation or IArtifactAssociated, so we assume means it exists
        return true;
    }

    return false;
}

void Artifact::addItem(ISlangUnknown* intf) 
{ 
    SLANG_ASSERT(intf);
    // Can't already be in there
    SLANG_ASSERT(m_items.indexOf(intf) < 0);
    // Add it
    m_items.add(ComPtr<ISlangUnknown>(intf));
}

void Artifact::removeItemAt(Index i)
{
    m_items.removeAt(i);
}

void* Artifact::findItemInterface(const Guid& guid)
{
    for (ISlangUnknown* intf : m_items)
    {
        ISlangUnknown* cast = nullptr;
        if (SLANG_SUCCEEDED(intf->queryInterface(guid, (void**)&cast)) && cast)
        {
            // NOTE! This assumes we *DONT* need to ref count to keep an interface in scope
            // (as strict COM requires so as to allow on demand interfaces).
            cast->release();
            return cast;
        }
    }
    return nullptr;
}

void* Artifact::findItemObject(const Guid& classGuid)
{
    for (ISlangUnknown* intf : m_items)
    {
        ComPtr<ICastable> castable;
        if (SLANG_SUCCEEDED(intf->queryInterface(ICastable::getTypeGuid(), (void**)castable.writeRef())) && castable)
        {
            void* obj = castable->castAs(classGuid);

            // NOTE! This assumes we *DONT* need to ref count to keep an interface in scope
            // (as strict COM requires so as to allow on demand interfaces).
            
            // If could cast return the result
            if (obj)
            {
                return obj;
            }
        }
    }
    return nullptr;
}

SlangResult Artifact::requireFile(Keep keep, IFileArtifactRepresentation** outFileRep)
{
    auto util = ArtifactUtilImpl::getSingleton();
    return util->requireFile(this, keep, outFileRep);
}

SlangResult Artifact::loadBlob(Keep keep, ISlangBlob** outBlob)
{
    // If we have a blob just return it
    if (auto blob = findItem<ISlangBlob>(this))
    {
        blob->addRef();
        *outBlob = blob;
        return SLANG_OK;
    }

    ComPtr<ISlangBlob> blob;

    // Look for a representation that we can serialize into a blob
    for (ISlangUnknown* intf : m_items)
    {
        ComPtr<IArtifactRepresentation> rep;
        if (SLANG_SUCCEEDED(intf->queryInterface(IArtifactRepresentation::getTypeGuid(), (void**)rep.writeRef())) && rep)
        {
            SlangResult res = rep->writeToBlob(blob.writeRef());
            if (SLANG_SUCCEEDED(res) && blob)
            {
                break;
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
        addItem(blob);
    }

    *outBlob = blob.detach();
    return SLANG_OK;
}

} // namespace Slang
