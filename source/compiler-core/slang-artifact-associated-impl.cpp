// slang-artifact-associated-impl.cpp
#include "slang-artifact-associated-impl.h"

#include "../core/slang-file-system.h"

#include "../core/slang-type-text-util.h"
#include "../core/slang-io.h"
#include "../core/slang-array-view.h"

#include "../core/slang-char-util.h"

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

    m_allocator.deallocateAll();
}

void DiagnosticsImpl::add(const Diagnostic& inDiagnostic)
{
    Diagnostic diagnostic(inDiagnostic);

    diagnostic.text = m_allocator.allocate(inDiagnostic.text);
    diagnostic.code = m_allocator.allocate(inDiagnostic.code);
    diagnostic.filePath = m_allocator.allocate(inDiagnostic.filePath);

    m_diagnostics.add(diagnostic);
}

void DiagnosticsImpl::setRaw(const ZeroTerminatedCharSlice& slice)
{
    m_raw = m_allocator.allocate(slice);
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

SLANG_FORCE_INLINE static UnownedStringSlice _toUnownedSlice(const ZeroTerminatedCharSlice& in)
{
    return UnownedStringSlice(in.data, in.count);
}

void DiagnosticsImpl::maybeAddNote(const ZeroTerminatedCharSlice& in)
{
    ArtifactDiagnosticsUtil::maybeAddNote(m_allocator, _toUnownedSlice(in), m_diagnostics);
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

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactDiagnosticsUtil !!!!!!!!!!!!!!!!!!!!!!!!!!! */

/* static */UnownedStringSlice ArtifactDiagnosticsUtil::getSeverityText(Severity severity)
{
    switch (severity)
    {
        default:                return UnownedStringSlice::fromLiteral("Unknown");
        case Severity::Info:    return UnownedStringSlice::fromLiteral("Info");
        case Severity::Warning: return UnownedStringSlice::fromLiteral("Warning");
        case Severity::Error:   return UnownedStringSlice::fromLiteral("Error");
    }
}

/* static */SlangResult ArtifactDiagnosticsUtil::splitPathLocation(ArtifactSliceAllocator& allocator, const UnownedStringSlice& pathLocation, ArtifactDiagnostic& outDiagnostic)
{
    const Index lineStartIndex = pathLocation.lastIndexOf('(');
    if (lineStartIndex >= 0)
    {
        outDiagnostic.filePath = allocator.allocate(pathLocation.head(lineStartIndex).trim());

        const UnownedStringSlice tail = pathLocation.tail(lineStartIndex + 1);
        const Index lineEndIndex = tail.indexOf(')');

        if (lineEndIndex >= 0)
        {
            // Extract the location info
            UnownedStringSlice locationSlice(tail.begin(), tail.begin() + lineEndIndex);

            UnownedStringSlice slices[2];
            const Index numSlices = StringUtil::split(locationSlice, ',', 2, slices);

            // NOTE! FXC actually outputs a range of columns in the form of START-END in the column position
            // We don't need to parse here, because we only care about the line number

            Int lineNumber = 0;
            if (numSlices > 0)
            {
                SLANG_RETURN_ON_FAIL(StringUtil::parseInt(slices[0], lineNumber));
            }

            // Store the line
            outDiagnostic.location.line = lineNumber;
        }
    }
    else
    {
        outDiagnostic.filePath = allocator.allocate(pathLocation);
    }
    return SLANG_OK;
}

/* static */SlangResult ArtifactDiagnosticsUtil::splitColonDelimitedLine(const UnownedStringSlice& line, Int pathIndex, List<UnownedStringSlice>& outSlices)
{
    StringUtil::split(line, ':', outSlices);

    // Now we want to fix up a path as might have drive letter, and therefore :
    // If this is the situation then we need to have a slice after the one at the index
    if (outSlices.getCount() > pathIndex + 1)
    {
        const UnownedStringSlice pathStart = outSlices[pathIndex].trim();
        if (pathStart.getLength() == 1 && CharUtil::isAlpha(pathStart[0]))
        {
            // Splice back together
            outSlices[pathIndex] = UnownedStringSlice(outSlices[pathIndex].begin(), outSlices[pathIndex + 1].end());
            outSlices.removeAt(pathIndex + 1);
        }
    }

    return SLANG_OK;
}

/* static */SlangResult ArtifactDiagnosticsUtil::parseColonDelimitedDiagnostics(ArtifactSliceAllocator& allocator, const UnownedStringSlice& inText, Int pathIndex, LineParser lineParser, List<ArtifactDiagnostic>& outDiagnostics)
{
    List<UnownedStringSlice> splitLine;

    UnownedStringSlice text(inText), line;
    while (StringUtil::extractLine(text, line))
    {
        SLANG_RETURN_ON_FAIL(splitColonDelimitedLine(line, pathIndex, splitLine));

        ArtifactDiagnostic diagnostic;
        diagnostic.severity = Severity::Error;
        diagnostic.stage = IDiagnostics::Stage::Compile;
        diagnostic.location.line = 0;
        diagnostic.location.column= 0;

        if (SLANG_SUCCEEDED(lineParser(allocator, line, splitLine, diagnostic)))
        {
            outDiagnostics.add(diagnostic);
        }
        else
        {
            // If couldn't parse, just add as a note
            maybeAddNote(allocator, line, outDiagnostics);
        }
    }

    return SLANG_OK;
}

/* static */void ArtifactDiagnosticsUtil::maybeAddNote(ArtifactSliceAllocator& allocator, const UnownedStringSlice& in, List<ArtifactDiagnostic>& ioDiagnostics)
{
    // Don't bother adding an empty line
    if (in.trim().getLength() == 0)
    {
        return;
    }

    // If there's nothing previous, we'll ignore too, as note should be in addition to
    // a pre-existing error/warning
    if (ioDiagnostics.getCount() == 0)
    {
        return;
    }

    // Make it a note on the output
    ArtifactDiagnostic diagnostic;

    diagnostic.severity = IDiagnostics::Severity::Info;
    diagnostic.text = allocator.allocate(in);
    ioDiagnostics.add(diagnostic);
}

} // namespace Slang
