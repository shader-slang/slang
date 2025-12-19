#ifndef SLANG_RICH_DIAGNOSTIC_H
#define SLANG_RICH_DIAGNOSTIC_H

#include "../core/slang-basic.h"
#include "slang-source-loc.h"
#include "slang-diagnostic-sink.h"

namespace Slang
{

/// Represents a labeled span in source code for rich diagnostics
struct DiagnosticLabel
{
    /// The source location span (start)
    SourceLoc loc;

    /// The message to display with this label
    String message;

    /// Whether this is the primary label (true) or secondary (false)
    bool isPrimary = false;

    DiagnosticLabel() = default;

    DiagnosticLabel(SourceLoc inLoc, const String& inMessage, bool inIsPrimary = false)
        : loc(inLoc), message(inMessage), isPrimary(inIsPrimary)
    {
    }
};

/// Represents a rich diagnostic with multiple spans, notes, and helps
struct RichDiagnostic
{
    /// The diagnostic code (e.g., "E30019")
    String code;

    /// Severity level
    Severity severity = Severity::Error;

    /// The main diagnostic message
    String message;

    /// All labels (primary and secondary) for this diagnostic
    List<DiagnosticLabel> labels;

    /// Additional notes that appear after the main diagnostic
    List<String> notes;

    /// Help messages suggesting fixes
    List<String> helps;

    /// Add a primary label
    void addPrimaryLabel(SourceLoc loc, const String& message)
    {
        labels.add(DiagnosticLabel(loc, message, true));
    }

    /// Add a secondary label
    void addSecondaryLabel(SourceLoc loc, const String& message)
    {
        labels.add(DiagnosticLabel(loc, message, false));
    }

    /// Add a note
    void addNote(const String& note) { notes.add(note); }

    /// Add a help message
    void addHelp(const String& help) { helps.add(help); }
};

/// Builder for constructing RichDiagnostic instances
class RichDiagnosticBuilder
{
public:
    RichDiagnosticBuilder& setCode(const String& code)
    {
        m_diagnostic.code = code;
        return *this;
    }

    RichDiagnosticBuilder& setSeverity(Severity severity)
    {
        m_diagnostic.severity = severity;
        return *this;
    }

    RichDiagnosticBuilder& setMessage(const String& message)
    {
        m_diagnostic.message = message;
        return *this;
    }

    RichDiagnosticBuilder& addPrimaryLabel(SourceLoc loc, const String& message)
    {
        m_diagnostic.addPrimaryLabel(loc, message);
        return *this;
    }

    RichDiagnosticBuilder& addSecondaryLabel(SourceLoc loc, const String& message)
    {
        m_diagnostic.addSecondaryLabel(loc, message);
        return *this;
    }

    RichDiagnosticBuilder& addNote(const String& note)
    {
        m_diagnostic.addNote(note);
        return *this;
    }

    RichDiagnosticBuilder& addHelp(const String& help)
    {
        m_diagnostic.addHelp(help);
        return *this;
    }

    RichDiagnostic build() { return std::move(m_diagnostic); }

private:
    RichDiagnostic m_diagnostic;
};

} // namespace Slang

#endif // SLANG_RICH_DIAGNOSTIC_H