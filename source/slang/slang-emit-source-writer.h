// slang-source-stream.h
#ifndef SLANG_EMIT_SOURCE_WRITER_H
#define SLANG_EMIT_SOURCE_WRITER_H

#include "../core/slang-basic.h"

#include "slang-compiler.h"

namespace Slang
{

/* Class that encapsulates a stream of source. Facilities provided...

* Management of the buffer that holds the source content as it is constructed
* output line directives
  + Supports GLSL as well as C/CPP/HLSL style directives
* Support for line indention */
class SourceWriter
{
public:
        /// Emits without span without any extra processing
    void emitRawTextSpan(char const* textBegin, char const* textEnd);
    void emitRawText(char const* text);

        /// Emit different types into the stream
    void emit(char const* textBegin, char const* textEnd);
    void emit(char const* text);
    void emit(const String& text);
    void emit(const UnownedStringSlice& text);
    void emit(Name* name);
    void emit(const NameLoc& nameAndLoc);
    void emit(const StringSliceLoc& nameAndLoc);
    void emit(IntegerLiteralValue value);

    void emitUInt64(uint64_t value);
    void emitInt64(int64_t value);

    void emit(UInt value);
    void emit(int value);
    void emit(double value);

    void emitChar(char c);

        /// Emit names (doing so can also advance to a new source location)
    void emitName(const StringSliceLoc& nameAndLoc);
    void emitName(const NameLoc& nameAndLoc);
    void emitName(Name* name, const SourceLoc& loc);
    void emitName(Name* name);


        /// Indent the text
    void indent();
        /// Dedent (the opposite of indenting) the text
    void dedent();

        /// Move the current source location to that specified
    void advanceToSourceLocation(const SourceLoc& sourceLocation);
        /// Only advances if the sourceLocation is valid
    void advanceToSourceLocationIfValid(const SourceLoc& sourceLocation);

        /// Get the content as a string
    String getContent() { return m_builder.ProduceString(); }
        /// Clear the content
    void clearContent() { m_builder.Clear(); }
        /// Get the content as a string and clear the internal representation
    String getContentAndClear();

        /// Get the line directive mode used
    LineDirectiveMode getLineDirectiveMode() const { return m_lineDirectiveMode; }
        /// Get the source manager user
    SourceManager* getSourceManager() const { return m_sourceManager; }

        /// Ctor
    SourceWriter(SourceManager* sourceManager, LineDirectiveMode lineDirectiveMode);

protected:
    void _emitTextSpan(char const* textBegin, char const* textEnd);
    void _flushSourceLocationChange();

        // Emit a `#line` directive to the output, and also
        // ensure that source location tracking information
        // is correct based on the directive we just output.
    void _emitLineDirectiveAndUpdateSourceLocation(const HumaneSourceLoc& sourceLocation);

    void _emitLineDirectiveIfNeeded(const HumaneSourceLoc& sourceLocation);

        // Emit a `#line` directive to the output.
        // Doesn't update state of source-location tracking.
    void _emitLineDirective(const HumaneSourceLoc& sourceLocation);

    // The string of code we've built so far.
    // TODO(JS): We could store the text in chunks, and then only sew together into one buffer
    // when we are done. Doing so would not require copies/reallocs until the full buffer has been
    // produced. A downside to doing this is that it won't be so simple to debug by trying to
    // look at the current contents of the buffer
    StringBuilder m_builder;

    // Current source position for tracking purposes...
    HumaneSourceLoc m_loc;

    SourceLoc m_nextSourceLoc;
    HumaneSourceLoc m_nextHumaneSourceLocation;
    
    bool m_needToUpdateSourceLocation = false;
   
    // Are we at the start of a line, so that we should indent
    // before writing any other text?
    bool m_isAtStartOfLine = true;

    // How far are we indented?
    Int m_indentLevel = 0;

    SourceManager* m_sourceManager = nullptr;

    // For GLSL output, we can't emit traditional `#line` directives
    // with a file path in them, so we maintain a map that associates
    // each path with a unique integer, and then we output those
    // instead.
    Dictionary<String, int> m_mapGLSLSourcePathToID;
    int m_glslSourceIDCount = 0;

    LineDirectiveMode m_lineDirectiveMode;
};

}
#endif
