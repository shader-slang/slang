#ifndef SLANG_RICH_DIAGNOSTIC_LAYOUT_H
#define SLANG_RICH_DIAGNOSTIC_LAYOUT_H

#include "../core/slang-basic.h"
#include "slang-rich-diagnostic.h"
#include "slang-source-loc.h"

namespace Slang
{

struct SourceManager;

/// Layout engine for rendering rich diagnostics with multi-span support
///
/// This class takes a RichDiagnostic and renders it in a format similar to:
///
/// error[E30019]: cannot convert argument of type `float` to parameter of type `int`
///  --> file.slang:10:5
///   |
/// 3 | void process(int x) { }
///   |              ----- parameter `x` declared as `int` here
///   |
/// 10| process(1.5f);
///   |         ^^^^ expected `int`, found `float`
///   |
///   = note: no implicit conversion exists from `float` to `int`
///   = help: add explicit cast: `(int)1.5f`
///
class RichDiagnosticLayout
{
public:
    /// Configuration options for the layout engine
    struct Options
    {
        /// Maximum width of source lines before truncation
        Index maxLineWidth;

        /// Number of context lines to show before/after labeled lines
        Index contextLines;

        /// Tab size for rendering
        Index tabSize;

        /// Whether to use Unicode box-drawing characters
        bool useUnicode;

        /// Whether to group labels on the same line
        bool groupSameLineLabels;

        Options()
            : maxLineWidth(120)
            , contextLines(0)
            , tabSize(4)
            , useUnicode(false)
            , groupSameLineLabels(true)
        {
        }
    };

    RichDiagnosticLayout(SourceManager* sourceManager, Options options = Options())
        : m_sourceManager(sourceManager), m_options(options)
    {
    }

    /// Render a rich diagnostic to a string
    String render(const RichDiagnostic& diagnostic);

private:
    /// Information about a source line that needs to be displayed
    struct LineInfo
    {
        /// The file path
        String filePath;

        /// Line number (1-based)
        Index lineNumber = 0;

        /// The actual line content
        String content;

        /// Labels that apply to this line
        List<DiagnosticLabel> labels;

        /// Column range for each label (start, end)
        List<std::pair<Index, Index>> labelColumns;
    };

    /// Group labels by file and line
    void collectLineInfo(const RichDiagnostic& diagnostic, List<LineInfo>& outLineInfos);

    /// Render the header line (error[CODE]: message)
    void renderHeader(StringBuilder& sb, const RichDiagnostic& diagnostic);

    /// Render the primary location pointer (--> file:line:col)
    void renderPrimaryLocation(StringBuilder& sb, const LineInfo& lineInfo, Index column);

    /// Render a source line with its labels
    void renderSourceLine(StringBuilder& sb, const LineInfo& lineInfo, Index gutterWidth);

    /// Render the underline/label row beneath a source line
    void renderLabelRow(
        StringBuilder& sb,
        const LineInfo& lineInfo,
        Index gutterWidth,
        Index labelIndex);

    /// Render notes and helps
    void renderNotesAndHelps(
        StringBuilder& sb,
        const RichDiagnostic& diagnostic,
        Index gutterWidth);

    /// Get the gutter width needed for line numbers
    Index calculateGutterWidth(const List<LineInfo>& lineInfos);

    /// Replace tabs with spaces for consistent rendering
    String expandTabs(const UnownedStringSlice& line);

    /// Calculate the display column (accounting for tabs)
    Index calculateDisplayColumn(const UnownedStringSlice& line, Index byteOffset);

    SourceManager* m_sourceManager;
    Options m_options;
};

} // namespace Slang

#endif // SLANG_RICH_DIAGNOSTIC_LAYOUT_H
