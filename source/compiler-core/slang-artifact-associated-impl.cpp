// slang-artifact-associated-impl.cpp
#include "slang-artifact-associated-impl.h"

#include "../core/slang-file-system.h"

#include "../core/slang-type-text-util.h"
#include "../core/slang-io.h"
#include "../core/slang-array-view.h"

#include "slang-artifact-util.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!!! DiagnosticsImpl !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* DiagnosticsImpl::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() ||
        guid == ICastable::getTypeGuid() ||
        guid == IDiagnostics::getTypeGuid())
    {
        return static_cast<IDiagnostics*>(this);
    }
    return nullptr;
}

void* DiagnosticsImpl::getObject(const Guid& guid)
{
    SLANG_UNUSED(guid);
    return nullptr;
}

void* DiagnosticsImpl::castAs(const Guid& guid)
{
    if (auto intf = getInterface(guid))
    {
        return intf;
    }
    return getObject(guid);
}

void DiagnosticsImpl::reset()
{
    m_diagnostics.clear();
    m_raw = ZeroTerminatedCharSlice();
    m_result = SLANG_OK;

    m_arena.deallocateAll();
}

ZeroTerminatedCharSlice DiagnosticsImpl::_allocateSlice(const Slice<char>& in)
{
    if (in.count == 0)
    {
        return ZeroTerminatedCharSlice("", 0);
    }
    const char* dst = m_arena.allocateString(in.data, in.count);
    return ZeroTerminatedCharSlice(dst, in.count);
}

void DiagnosticsImpl::add(const Diagnostic& inDiagnostic)
{
    Diagnostic diagnostic(inDiagnostic);

    diagnostic.text = _allocateSlice(inDiagnostic.text);
    diagnostic.code = _allocateSlice(inDiagnostic.code);
    diagnostic.filePath = _allocateSlice(inDiagnostic.filePath);

    m_diagnostics.add(diagnostic);
}

void DiagnosticsImpl::setRaw(const ZeroTerminatedCharSlice& slice)
{
    m_raw = _allocateSlice(slice);
}

Count DiagnosticsImpl::getCountAtLeastSeverity(Severity severity) 
{
    Index count = 0;
    for (const auto& msg : m_diagnostics)
    {
        count += Index(Index(msg.severity) >= Index(severity));
    }
    return count;
}

Count DiagnosticsImpl::getCountBySeverity(Severity severity) 
{
    Index count = 0;
    for (const auto& msg : m_diagnostics)
    {
        count += Index(msg.severity == severity);
    }
    return count;
}

bool DiagnosticsImpl::hasOfAtLeastSeverity(Severity severity)
{
    for (const auto& msg : m_diagnostics)
    {
        if (Index(msg.severity) >= Index(severity))
        {
            return true;
        }
    }
    return false;
}

Count DiagnosticsImpl::getCountByStage(Stage stage, Count outCounts[Int(Severity::CountOf)]) 
{
    Int count = 0;
    ::memset(outCounts, 0, sizeof(Index) * Int(Severity::CountOf));
    for (const auto& diagnostic : m_diagnostics)
    {
        if (diagnostic.stage == stage)
        {
            count++;
            outCounts[Index(diagnostic.severity)]++;
        }
    }
    return count++;
}

void DiagnosticsImpl::removeBySeverity(Severity severity) 
{
    Index count = m_diagnostics.getCount();
    for (Index i = 0; i < count; ++i)
    {
        if (m_diagnostics[i].severity == severity)
        {
            m_diagnostics.removeAt(i);
            i--;
            count--;
        }
    }
}

void DiagnosticsImpl::maybeAddNote(const ZeroTerminatedCharSlice& in)
{
    // Don't bother adding an empty line
    if (UnownedStringSlice(in.begin(), in.end()).trim().getLength() == 0)
    {
        return;
    }

    // If there's nothing previous, we'll ignore too, as note should be in addition to
    // a pre-existing error/warning
    if (m_diagnostics.getCount() == 0)
    {
        return;
    }

    // Make it a note on the output
    Diagnostic diagnostic;
    
    diagnostic.severity = Severity::Info;
    diagnostic.text = _allocateSlice(in);
    m_diagnostics.add(diagnostic);
}

void DiagnosticsImpl::requireErrorDiagnostic() 
{
    // If we find an error, we don't need to add a generic diagnostic
    for (const auto& msg : m_diagnostics)
    {
        if (Index(msg.severity) >= Index(Severity::Error))
        {
            return;
        }
    }

    Diagnostic diagnostic;
    diagnostic.severity = Severity::Error;
    diagnostic.text = m_raw;

    // Add the diagnostic
    m_diagnostics.add(diagnostic);
}

/* static */UnownedStringSlice _getSeverityText(IDiagnostics::Severity severity)
{
    typedef IDiagnostics::Severity Severity;
    switch (severity)
    {
        default:                return UnownedStringSlice::fromLiteral("Unknown");
        case Severity::Info:    return UnownedStringSlice::fromLiteral("Info");
        case Severity::Warning: return UnownedStringSlice::fromLiteral("Warning");
        case Severity::Error:   return UnownedStringSlice::fromLiteral("Error");
    }
}

static void _appendCounts(const Index counts[Int(IDiagnostics::Severity::CountOf)], StringBuilder& out)
{
    typedef IDiagnostics::Severity Severity;

    for (Index i = 0; i < Int(Severity::CountOf); i++)
    {
        if (counts[i] > 0)
        {
            out << _getSeverityText(Severity(i)) << "(" << counts[i] << ") ";
        }
    }
}

static void _appendSimplified(const Index counts[Int(IDiagnostics::Severity::CountOf)], StringBuilder& out)
{
    typedef IDiagnostics::Severity Severity;
    for (Index i = 0; i < Int(Severity::CountOf); i++)
    {
        if (counts[i] > 0)
        {
            out << _getSeverityText(Severity(i)) << " ";
        }
    }
}

void DiagnosticsImpl::appendSummary(ISlangBlob** outBlob)
{
    StringBuilder buf;

    Index counts[Int(Severity::CountOf)];
    if (getCountByStage(Stage::Compile, counts) > 0)
    {
        buf << "Compile: ";
        _appendCounts(counts, buf);
        buf << "\n";
    }
    if (getCountByStage(Stage::Link, counts) > 0)
    {
        buf << "Link: ";
        _appendCounts(counts, buf);
        buf << "\n";
    }

    *outBlob = StringBlob::moveCreate(buf).detach();
}

void DiagnosticsImpl::appendSimplifiedSummary(ISlangBlob** outBlob)
{
    StringBuilder buf;

    Index counts[Int(Severity::CountOf)];
    if (getCountByStage(Stage::Compile, counts) > 0)
    {
        buf << "Compile: ";
        _appendSimplified(counts, buf);
        buf << "\n";
    }
    if (getCountByStage(Stage::Link, counts) > 0)
    {
        buf << "Link: ";
        _appendSimplified(counts, buf);
        buf << "\n";
    }

    *outBlob = StringBlob::moveCreate(buf).detach();
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! PostEmitMetadataImpl !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* PostEmitMetadataImpl::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() ||
        guid == ICastable::getTypeGuid() ||
        guid == IPostEmitMetadata::getTypeGuid())
    {
        return static_cast<IPostEmitMetadata*>(this);
    }
    return nullptr;
}

void* PostEmitMetadataImpl::getObject(const Guid& uuid)
{
    if (uuid == getTypeGuid())
    {
        return this;
    }
    return nullptr;
}

void* PostEmitMetadataImpl::castAs(const Guid& guid)
{
    if (auto ptr = getInterface(guid))
    {
        return ptr;
    }
    return getObject(guid);
}

Slice<ShaderBindingRange> PostEmitMetadataImpl::getUsedBindingRanges()
{ 
    return Slice<ShaderBindingRange>(m_usedBindings.getBuffer(), m_usedBindings.getCount()); 
}

} // namespace Slang
