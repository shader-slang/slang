// emit.cpp
#include "slang-source-sink.h"

// Disable warnings about sprintf
#ifdef _WIN32
#   pragma warning(disable:4996)
#endif

// Note: using C++ stdio just to get a locale-independent
// way to format floating-point values.
//
// TODO: Go ahead and implement teh Dragon4 algorithm so
// that we can print floating-point values to arbitrary
// precision as needed.
#include <sstream>

namespace Slang {

SourceSink::SourceSink(SourceManager* sourceManager, LineDirectiveMode lineDirectiveMode)
{
    m_lineDirectiveMode = lineDirectiveMode;
    this->sourceManager = sourceManager;
}

// Low-level emit logic

void SourceSink::emitRawTextSpan(char const* textBegin, char const* textEnd)
{
    // TODO(tfoley): Need to make "corelib" not use `int` for pointer-sized things...
    auto len = textEnd - textBegin;
    sb.Append(textBegin, len);
}

void SourceSink::emitRawText(char const* text)
{
    emitRawTextSpan(text, text + strlen(text));
}

void SourceSink::emitTextSpan(char const* textBegin, char const* textEnd)
{
    // Don't change anything given an empty string
    if (textBegin == textEnd)
        return;

    // If the source location has changed in a way that required update,
    // do it now!
    flushSourceLocationChange();

    // Note: we don't want to emit indentation on a line that is empty.
    // The logic in `Emit(textBegin, textEnd)` below will have broken
    // the text into lines, so we can simply check if a line consists
    // of just a newline.
    if (isAtStartOfLine && *textBegin != '\n')
    {
        // We are about to emit text (other than a newline)
        // at the start of a line, so we will emit the proper
        // amount of indentation to keep things looking nice.
        isAtStartOfLine = false;
        for (Int ii = 0; ii < indentLevel; ++ii)
        {
            char const* indentString = "    ";
            size_t indentStringSize = strlen(indentString);
            emitRawTextSpan(indentString, indentString + indentStringSize);

            // We will also update our tracking location, just in
            // case other logic needs it.
            //
            // TODO: We may need to have a switch that controls whether
            // we are in "pretty-printing" mode or "follow the locations
            // in the original code" mode.
            loc.column += indentStringSize;
        }
    }

    // Emit the raw text
    emitRawTextSpan(textBegin, textEnd);

    // Update our logical position
    auto len = int(textEnd - textBegin);
    loc.column += len;
}

void SourceSink::indent()
{
    indentLevel++;
}

void SourceSink::dedent()
{
    indentLevel--;
}

void SourceSink::Emit(char const* textBegin, char const* textEnd)
{
    char const* spanBegin = textBegin;
    char const* spanEnd = spanBegin;
    for (;;)
    {
        if (spanEnd == textEnd)
        {
            // We have a whole range of text waiting to be flushed
            emitTextSpan(spanBegin, spanEnd);
            return;
        }

        auto c = *spanEnd++;

        if (c == '\n')
        {
            // At the end of a line, we need to update our tracking
            // information on code positions
            emitTextSpan(spanBegin, spanEnd);
            loc.line++;
            loc.column = 1;
            isAtStartOfLine = true;

            // Start a new span for emit purposes
            spanBegin = spanEnd;
        }
    }
}

void SourceSink::Emit(char const* text)
{
    Emit(text, text + strlen(text));
}

void SourceSink::emit(String const& text)
{
    Emit(text.begin(), text.end());
}

void SourceSink::emit(UnownedStringSlice const& text)
{
    Emit(text.begin(), text.end());
}

void SourceSink::emit(Name* name)
{
    emit(getText(name));
}

void SourceSink::emit(const NameLoc& nameAndLoc)
{
    advanceToSourceLocation(nameAndLoc.loc);
    emit(getText(nameAndLoc.name));
}

void SourceSink::emitName(Name* name, const SourceLoc& locIn)
{
    advanceToSourceLocation(locIn);
    emit(name);
}

void SourceSink::emitName(const NameLoc& nameAndLoc)
{
    emitName(nameAndLoc.name, nameAndLoc.loc);
}

void SourceSink::emitName(Name* name)
{
    emitName(name, SourceLoc());
}

void SourceSink::Emit(IntegerLiteralValue value)
{
    char buffer[32];
    sprintf(buffer, "%lld", (long long int)value);
    Emit(buffer);
}

void SourceSink::Emit(UInt value)
{
    char buffer[32];
    sprintf(buffer, "%llu", (unsigned long long)(value));
    Emit(buffer);
}

void SourceSink::Emit(int value)
{
    char buffer[16];
    sprintf(buffer, "%d", value);
    Emit(buffer);
}

void SourceSink::Emit(double value)
{
    // There are a few different requirements here that we need to deal with:
    //
    // 1) We need to print something that is valid syntax in the target language
    //    (this means that hex floats are off the table for now)
    //
    // 2) We need our printing to be independent of the current global locale in C,
    //    so that we don't depend on the application leaving it as the default,
    //    and we also don't revert any changes they make.
    //    (this means that `sprintf` and friends are off the table)
    //
    // 3) We need to be sure that floating-point literals specified by the user will
    //    "round-trip" and turn into the same value when parsed back in. This means
    //    that we need to print a reasonable number of digits of precision.
    //
    // For right now, the easiest option that can balance these is to use
    // the C++ standard library `iostream`s, because they support an explicit locale,
    // and can (hopefully) print floating-point numbers accurately.
    //
    // Eventually, the right move here would be to implement proper floating-point
    // number formatting ourselves, but that would require extensive testing to
    // make sure we get it right.

    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(20);
    stream << value;

    Emit(stream.str().c_str());
}

void SourceSink::advanceToSourceLocation(const SourceLoc& sourceLocation)
{
    advanceToSourceLocation(getSourceManager()->getHumaneLoc(sourceLocation));
}

void SourceSink::advanceToSourceLocation(const HumaneSourceLoc& sourceLocation)
{
    // Skip invalid locations
    if (sourceLocation.line <= 0)
        return;

    needToUpdateSourceLocation = true;
    nextSourceLocation = sourceLocation;
}

void SourceSink::flushSourceLocationChange()
{
    if (!needToUpdateSourceLocation)
        return;

    // Note: the order matters here, because trying to update
    // the source location may involve outputting text that
    // advances the location, and outputting text is what
    // triggers this flush operation.
    needToUpdateSourceLocation = false;
    emitLineDirectiveIfNeeded(nextSourceLocation);
}

void SourceSink::emitLineDirectiveAndUpdateSourceLocation(const HumaneSourceLoc& sourceLocation)
{
    emitLineDirective(sourceLocation);

    HumaneSourceLoc newLoc = sourceLocation;
    newLoc.column = 1;

    loc = newLoc;
}

void SourceSink::emitLineDirectiveIfNeeded(const HumaneSourceLoc& sourceLocation)
{
    // Don't do any of this work if the user has requested that we
    // not emit line directives.
    auto mode = getLineDirectiveMode();
    switch (mode)
    {
        case LineDirectiveMode::None:
            return;

        case LineDirectiveMode::Default:
        default:
            break;
    }

    // Ignore invalid source locations
    if (sourceLocation.line <= 0)
        return;

    // If we are currently emitting code at a source location with
    // a differnet file or line, *or* if the source location is
    // somehow later on the line than what we want to emit,
    // then we need to emit a new `#line` directive.
    if (sourceLocation.pathInfo.foundPath != loc.pathInfo.foundPath
        || sourceLocation.line != loc.line
        || sourceLocation.column < loc.column)
    {
        // Special case: if we are in the same file, and within a small number
        // of lines of the target location, then go ahead and output newlines
        // to get us caught up.
        enum { kSmallLineCount = 3 };
        auto lineDiff = sourceLocation.line - loc.line;
        if (sourceLocation.pathInfo.foundPath == loc.pathInfo.foundPath
            && sourceLocation.line > loc.line
            && lineDiff <= kSmallLineCount)
        {
            for (int ii = 0; ii < lineDiff; ++ii)
            {
                Emit("\n");
            }
            SLANG_RELEASE_ASSERT(sourceLocation.line == loc.line);
        }
        else
        {
            // Go ahead and output a `#line` directive to get us caught up
            emitLineDirectiveAndUpdateSourceLocation(sourceLocation);
        }
    }
}

void SourceSink::emitLineDirective(const HumaneSourceLoc& sourceLocation)
{
    emitRawText("\n#line ");

    char buffer[16];
    sprintf(buffer, "%llu", (unsigned long long)sourceLocation.line);
    emitRawText(buffer);

    // Only emit the path part of a `#line` directive if needed
    if (sourceLocation.pathInfo.foundPath != loc.pathInfo.foundPath)
    {
        emitRawText(" ");

        auto mode = getLineDirectiveMode();
        switch (mode)
        {
            default:
            case LineDirectiveMode::None:
                SLANG_UNEXPECTED("should not be trying to emit '#line' directive");
                return;
            case LineDirectiveMode::GLSL:
            {
                auto path = sourceLocation.pathInfo.foundPath;

                // GLSL doesn't support the traditional form of a `#line` directive without
                // an extension. Rather than depend on that extension we will output
                // a directive in the traditional GLSL fashion.
                //
                // TODO: Add some kind of configuration where we require the appropriate
                // extension and then emit a traditional line directive.

                int id = 0;
                if (!mapGLSLSourcePathToID.TryGetValue(path, id))
                {
                    id = glslSourceIDCount++;
                    mapGLSLSourcePathToID.Add(path, id);
                }

                sprintf(buffer, "%d", id);
                emitRawText(buffer);
                break;
            }
            case LineDirectiveMode::Default:
            case LineDirectiveMode::Standard:
            {
                // The simple case is to emit the path for the current source
                // location. We need to be a little bit careful with this,
                // because the path might include backslash characters if we
                // are on Windows, and we want to canonicalize those over
                // to forward slashes.
                //
                // TODO: Canonicalization like this should be done centrally
                // in a module that tracks source files.

                emitRawText("\"");
                const auto& path = sourceLocation.pathInfo.foundPath;
                for (auto c : path)
                {
                    char charBuffer[] = { c, 0 };
                    switch (c)
                    {
                        default:
                            emitRawText(charBuffer);
                            break;

                            // The incoming file path might use `/` and/or `\\` as
                            // a directory separator. We want to canonicalize this.
                            //
                            // TODO: should probably canonicalize paths to not use backslash somewhere else
                            // in the compilation pipeline...
                        case '\\':
                            emitRawText("/");
                            break;
                    }
                }
                emitRawText("\"");
                break;
            }
        }
    }

    emitRawText("\n");
}

} // namespace Slang
