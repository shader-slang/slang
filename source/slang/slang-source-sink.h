// slang-source-sink.h
#ifndef SLANG_SOURCE_SINK_H_INCLUDED
#define SLANG_SOURCE_SINK_H_INCLUDED

#include "../core/basic.h"

#include "compiler.h"

namespace Slang
{

class SourceSink
{
public:

    void emitRawTextSpan(char const* textBegin, char const* textEnd);

    void emitRawText(char const* text);

    void emitTextSpan(char const* textBegin, char const* textEnd);

    void Emit(char const* textBegin, char const* textEnd);

    void Emit(char const* text);

    void emit(String const& text);

    void emit(UnownedStringSlice const& text);

    void emit(Name* name);

    void emit(const NameLoc& nameAndLoc);

    void emitName(Name* name, const SourceLoc& loc);

    void emitName(const NameLoc& nameAndLoc);

    void emitName(Name* name);

    void Emit(IntegerLiteralValue value);

    void Emit(UInt value);

    void Emit(int value);

    void Emit(double value);

    void indent();
    void dedent();

    void advanceToSourceLocation(const SourceLoc& sourceLocation);
    void advanceToSourceLocation(const HumaneSourceLoc& sourceLocation);

        // Emit a `#line` directive to the output, and also
        // ensure that source location tracking information
        // is correct based on the directive we just output.
    void emitLineDirectiveAndUpdateSourceLocation(const HumaneSourceLoc& sourceLocation);

    void emitLineDirectiveIfNeeded(const HumaneSourceLoc& sourceLocation);

        // Emit a `#line` directive to the output.
        // Doesn't update state of source-location tracking.
    void emitLineDirective(const HumaneSourceLoc& sourceLocation);

    void flushSourceLocationChange();

    String getContent() { return sb.ProduceString(); }

    void clearContent() { sb.Clear(); }

        /// Get the line directive mode used
    LineDirectiveMode getLineDirectiveMode() const { return m_lineDirectiveMode; }
        /// Get the source manager user
    SourceManager* getSourceManager() const { return sourceManager; }

        /// Ctor
    SourceSink(SourceManager* sourceManager, LineDirectiveMode lineDirectiveMode);

protected:
    // The string of code we've built so far
    StringBuilder sb;

    // Current source position for tracking purposes...
    HumaneSourceLoc loc;
    HumaneSourceLoc nextSourceLocation;
    bool needToUpdateSourceLocation;

    // Are we at the start of a line, so that we should indent
    // before writing any other text?
    bool isAtStartOfLine = true;

    // How far are we indented?
    Int indentLevel = 0;

    SourceManager* sourceManager;

    // For GLSL output, we can't emit traditional `#line` directives
    // with a file path in them, so we maintain a map that associates
    // each path with a unique integer, and then we output those
    // instead.
    Dictionary<String, int> mapGLSLSourcePathToID;
    int glslSourceIDCount = 0;

    LineDirectiveMode m_lineDirectiveMode;
};

}
#endif
