// slang-artifact-associated-impl.cpp
#include "slang-artifact-associated-impl.h"

#include "../core/slang-file-system.h"

#include "../core/slang-type-text-util.h"
#include "../core/slang-io.h"
#include "../core/slang-array-view.h"

#include "slang-artifact-util.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactDiagnostics !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* ArtifactDiagnostics::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() ||
        guid == ICastable::getTypeGuid() ||
        guid == IDiagnostics::getTypeGuid())
    {
        return static_cast<IDiagnostics*>(this);
    }
    return nullptr;
}

void* ArtifactDiagnostics::getObject(const Guid& guid)
{
    SLANG_UNUSED(guid);
    return nullptr;
}

void* ArtifactDiagnostics::castAs(const Guid& guid)
{
    if (auto intf = getInterface(guid))
    {
        return intf;
    }
    return getObject(guid);
}

ZeroTerminatedCharSlice ArtifactDiagnostics::_allocateSlice(const Slice<char>& in)
{
    if (in.count == 0)
    {
        return ZeroTerminatedCharSlice("", 0);
    }
    const char* dst = m_arena.allocateString(in.data, in.count);
    return ZeroTerminatedCharSlice(dst, in.count);
}

void ArtifactDiagnostics::add(const Diagnostic& inDiagnostic)
{
    Diagnostic diagnostic(inDiagnostic);

    diagnostic.text = _allocateSlice(inDiagnostic.text);
    diagnostic.code = _allocateSlice(inDiagnostic.code);
    diagnostic.filePath = _allocateSlice(inDiagnostic.filePath);

    m_diagnostics.add(diagnostic);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! PostEmitMetadata !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* PostEmitMetadata::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() ||
        guid == ICastable::getTypeGuid() ||
        guid == IPostEmitMetadata::getTypeGuid())
    {
        return static_cast<IPostEmitMetadata*>(this);
    }
    return nullptr;
}

void* PostEmitMetadata::getObject(const Guid& uuid)
{
    if (uuid == getTypeGuid())
    {
        return this;
    }
    return nullptr;
}

void* PostEmitMetadata::castAs(const Guid& guid)
{
    if (auto ptr = getInterface(guid))
    {
        return ptr;
    }
    return getObject(guid);
}

Slice<ShaderBindingRange> PostEmitMetadata::getBindingRanges()
{ 
    return Slice<ShaderBindingRange>(m_usedBindings.getBuffer(), m_usedBindings.getCount()); 
}

} // namespace Slang
