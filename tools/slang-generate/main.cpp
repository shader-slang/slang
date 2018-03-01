// main.cpp

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../source/core/secure-crt.h"

#include "../../source/core/list.h"
#include "../../source/core/slang-string.h"

using namespace Slang;

typedef Slang::UnownedStringSlice StringSpan;

struct Node
{
    enum class Flavor
    {
        text,   // Ordinary text to write to output
        escape, // Meta-level code (statements)
        splice, // Meta-level expression to splice into output
    };

    // What sort of node is this?
    Flavor      flavor;

    // The text of this node for `Flavor::text`
    StringSpan  span;

    // The body of this node for other flavors
    Node*       body;

    // The next node in the document
    Node*       next;
};

void addNode(
    Node**&         ioLink,
    Node::Flavor    flavor,
    char const*     spanBegin,
    char const*     spanEnd)
{
    Node* node = new Node();
    node->flavor = flavor;
    node->span = StringSpan(spanBegin, spanEnd);
    node->next = nullptr;

    *ioLink = node;
    ioLink = &node->next;
}

void addNode(
    Node**&         ioLink,
    Node::Flavor    flavor,
    Node*           body)
{
    Node* node = new Node();
    node->flavor = flavor;
    node->body = body;
    node->next = nullptr;

    *ioLink = node;
    ioLink = &node->next;
}

bool isAlpha(int c)
{
    return ((c >= 'a') && (c <= 'z'))
        || ((c >= 'A') && (c <= 'Z'))
        || (c == '_');
}

void addTextSpan(
    Node**&         ioLink,
    char const*     spanBegin,
    char const*     spanEnd)
{
    // Don't add an empty text span.
    if (spanBegin == spanEnd)
        return;

    addNode(ioLink, Node::Flavor::text, spanBegin, spanEnd);
}

void addSpliceSpan(
    Node**&         ioLink,
    Node*           body)
{
    addNode(ioLink, Node::Flavor::splice, body);
}

void addEscapeSpan(
    Node**&         ioLink,
    Node*           body)
{
    addNode(ioLink, Node::Flavor::escape, body);
}

void addEscapeSpan(
    Node**&         ioLink,
    char const*     spanBegin,
    char const*     spanEnd)
{
    Node* body = nullptr;
    Node** link = &body;

    addTextSpan(link, spanBegin, spanEnd);

    return addEscapeSpan(ioLink, body);
}

bool isIdentifierChar(int c)
{
    if (c >= 'a' && c <= 'z')   return true;
    if (c >= 'A' && c <= 'Z')   return true;
    if (c == '_')               return true;

    return false;

}

struct Reader
{
    char const* cursor;
    char const* end;
};

int peek(Reader const& reader)
{
    if (reader.cursor == reader.end)
        return EOF;

    return *reader.cursor;
}

int get(Reader& reader)
{
    if (reader.cursor == reader.end)
        return -1;

    return *reader.cursor++;
}

void handleNewline(Reader& reader, int c)
{
    int d = peek(reader);
    if ((c ^ d) == ('\r' ^ '\n'))
    {
        get(reader);
    }
}

bool isHorizontalSpace(int c)
{
    return (c == ' ') || (c == '\t');
}

void skipHorizontalSpace(Reader& reader)
{
    while (isHorizontalSpace(peek(reader)))
        get(reader);
}

void skipOptionalNewline(Reader& reader)
{
    switch (peek(reader))
    {
    default:
        break;

    case '\r': case '\n':
        {
            int c = get(reader);
            handleNewline(reader, c);
        }
        break;
    }
}

typedef unsigned int NodeReadFlags;
enum
{
    kNodeReadFlag_AllowEscape = 1 << 0,
};

Node* readBody(
    Reader&         reader,
    NodeReadFlags   flags,
    char            openChar,
    int             openCount,
    char            closeChar)
{
    while (peek(reader) == openChar)
    {
        get(reader);
        openCount++;
    }

    Node* nodes = nullptr;
    Node** link = &nodes;

    bool atStartOfLine = true;
    int depth = 0;

    char const* spanBegin = reader.cursor;
    char const* lineBegin = reader.cursor;
    for (;;)
    {
        int c = get(reader);

        switch (c)
        {
        default:
            atStartOfLine = false;
            break;

        case EOF:
            {
                addTextSpan(link, spanBegin, reader.cursor);
                return nodes;
            }

        case '{': case '(':
            if (c == openChar)
            {
                depth++;
            }
            atStartOfLine = false;
            break;

        case ')': case '}':
            if (c == closeChar)
            {
                char const* spanEnd = reader.cursor - 1;

                if (openCount == 1)
                {
                    if (depth == 0)
                    {
                        // We are at the end of the body.
                        addTextSpan(link, spanBegin, spanEnd);
                        return nodes;
                    }

                    depth--;
                }
                else
                {
                    // Count how many closing chars are stacked up

                    int closeCount = 1;
                    while (peek(reader) == closeChar)
                    {
                        get(reader);
                        closeCount++;
                    }

                    if (closeCount == openCount)
                    {
                        // We are at the end of the body.
                        addTextSpan(link, spanBegin, spanEnd);
                        return nodes;
                    }
                }
            }
            atStartOfLine = false;
            break;


        case ' ': case '\t':
            break;

        case '\r': case '\n':
            {
                addTextSpan(link, spanBegin, reader.cursor);

                handleNewline(reader, c);

                lineBegin = reader.cursor;
                spanBegin = reader.cursor;
                atStartOfLine = true;
            }
            break;

        case '$':
            {
                // If this is the start of a splice, then
                // the end of the preceding raw-text space
                // will be the byte before `$`
                char const* spanEnd = reader.cursor - 1;

                if (peek(reader) == '(')
                {
                    // This appears to be an expression splice.
                    //
                    // We must end the preceding span.
                    //
                    addTextSpan(link, spanBegin, spanEnd);

                    Node* body = readBody(
                        reader,
                        0,
                        '(',
                        0,
                        ')');

                    addSpliceSpan(link, body);

                    spanBegin = reader.cursor;
                    atStartOfLine = false;
                }
                else if (peek(reader) == '{')
                {
                    // This is the start of a block-structured escape, which will
                    // end at a matching `}`.

                    addTextSpan(link, spanBegin, lineBegin);

                    Node* body = readBody(
                        reader,
                        0,
                        '{',
                        0,
                        '}');

                    addEscapeSpan(link, body);

                    spanBegin = reader.cursor;
                    atStartOfLine = false;
                }
                else if (atStartOfLine && peek(reader) == ':')
                {
                    // This is a statement escape, which will
                    // continue to the end of the line.
                    //
                    // The spliced text begins *after* the `:`
                    get(reader);
                    char const* spliceBegin = reader.cursor;

                    // The preceding text span will end at the
                    // start of this line.
                    addTextSpan(link, spanBegin, lineBegin);

                    // Any indentation on this line will be ignored.

                    // Read up to end of line.
                    for (;;)
                    {
                        int c = get(reader);
                        switch (c)
                        {
                        default:
                            continue;

                        case EOF:
                            break;

                        case '\r':
                        case '\n':
                            handleNewline(reader, c);
                            break;
                        }

                        break;
                    }

                    addEscapeSpan(link, spliceBegin, reader.cursor);

                    spanBegin = reader.cursor;
                    lineBegin = reader.cursor;
                }
                else if (atStartOfLine && isIdentifierChar(peek(reader)))
                {
                    // This is a statement splice, which will use a {}-enclosed
                    // body for the template to generate.

                    // Consume an optional identifier
                    while (isIdentifierChar(peek(reader)))
                        get(reader);

                    // Consume optional horizontal space
                    skipHorizontalSpace(reader);

                    // Consume an optional `()`-enclosed block (strip
                    // all but the outer-most `()`.

                    // optional space/newline/space before `{`
                    skipHorizontalSpace(reader);
                    skipOptionalNewline(reader);
                    skipHorizontalSpace(reader);

                    throw 99;
                }
                else
                {
                    // Doesn't seem to be a splice at all, just
                    // a literal `$` in the output.
                    atStartOfLine = false;
                }
            }
            break;
        }
    }

}

Node* readInput(
    char const*     inputBegin,
    char const*     inputEnd)
{
    Reader reader;
    reader.cursor = inputBegin;
    reader.end = inputEnd;

    return readBody(
        reader,
        kNodeReadFlag_AllowEscape,
        -2,
        0,
        -2);
}

void emitRaw(
    FILE* stream,
    char const* begin,
    char const* end)
{
    // We will write the raw text to our output file.

    // TODO: need to output `#line` directives as well

    fputs("sb << \"", stream);
    for( char const* cc = begin; cc != end; ++cc )
    {
        int c = *cc;
        switch( c )
        {
        case '\\':
            fputs("\\\\", stream);
            break;

        case '\r': break;
        case '\t': fputs("\\t", stream); break;
        case '\"': fputs("\\\"", stream); break;
        case '\n':
            fputs("\\n\";\n", stream);
            fputs("sb << \"", stream);
            break;

        default:
            if((c >= 32) && (c <= 126))
            {
                fputc(c, stream);
            }
            else
            {
                assert(false);
            }
        }

    }
    fprintf(stream, "\";\n");
}

void emitCode(
    FILE*   stream,
    char const* begin,
    char const* end)
{
    for( auto cc = begin; cc != end; ++cc )
    {
        if(*cc == '\r')
            continue;

        fputc(*cc, stream);
    }
}

void emit(
    FILE*       stream,
    char const* text)
{
    fprintf(stream, "%s", text);
}

void emit(
    FILE*       stream,
    StringSpan const&   span)
{
    fprintf(stream, "%.*s", int(span.end() - span.begin()), span.begin());
}

bool isASCIIPrintable(int c)
{
    return (c >= 0x20) && (c <= 0x7E);
}

void emitStringLiteralText(
    FILE*               stream,
    StringSpan const&   span)
{
    char const* cursor = span.begin();
    char const* end = span.end();

    while (cursor != end)
    {
        int c = *cursor++;
        switch (c)
        {
        case '\r': case '\n':
            fprintf(stream, "\\n");
            break;

        case '\t':
            fprintf(stream, "\\t");
            break;

        case ' ':
            fprintf(stream, " ");
            break;

        case '"':
            fprintf(stream, "\\\"");
            break;

        case '\\':
            fprintf(stream, "\\\\");
            break;

        default:
            if (isASCIIPrintable(c))
            {
                fprintf(stream, "%c", c);
            }
            else
            {
                fprintf(stream, "%03u", c);
            }
            break;
        }
    }
}

void emitSimpleText(
    FILE*               stream,
    StringSpan const&   span)
{
    char const* cursor = span.begin();
    char const* end = span.end();

    while (cursor != end)
    {
        int c = *cursor++;
        switch (c)
        {
        default:
            fprintf(stream, "%c", c);
            break;

        case '\r': case '\n':
            if (cursor != end)
            {
                int d = *cursor;
                if ((c ^ d) == ('\r' ^ '\n'))
                {
                    cursor++;
                }
                fprintf(stream, "\n");
            }
            break;
        }
    }
}

void emitCodeNodes(
    FILE*   stream,
    Node*   node)
{
    for (auto nn = node; nn; nn = nn->next)
    {
        switch (nn->flavor)
        {
        case Node::Flavor::text:
            emitSimpleText(stream, nn->span);
            emit(stream, "\n");
            break;

        default:
            throw "unexpected";
            break;
        }
    }
}

void emitTemplateNodes(
    FILE*   stream,
    Node*   node)
{
    for (auto nn = node; nn; nn = nn->next)
    {
        switch (nn->flavor)
        {
        case Node::Flavor::text:
            emit(stream, "SLANG_RAW(\"");
            emitStringLiteralText(stream, nn->span);
            emit(stream, "\")\n");
            break;

        case Node::Flavor::splice:
            emit(stream, "SLANG_SPLICE(");
            emitCodeNodes(stream, nn->body);
            emit(stream, ")\n");
            break;

        case Node::Flavor::escape:
            emitCodeNodes(stream, nn->body);
            break;
        }
    }
}

void usage(char const* appName)
{
    fprintf(stderr, "usage: %s <input>\n", appName);
}

char* readAllText(char const * fileName)
{
    FILE * f;
    fopen_s(&f, fileName, "rb");
    if (!f)
    {
        return "";
    }
    else
    {
        fseek(f, 0, SEEK_END);
        auto size = ftell(f);
        char * buffer = new char[size + 1];
        memset(buffer, 0, size + 1);
        fseek(f, 0, SEEK_SET);
        fread(buffer, sizeof(char), size, f);
        fclose(f);
        return buffer;
    }
}

void writeAllText(char const *srcFileName, char const* fileName, char* content)
{
    FILE * f = nullptr;
    fopen_s(&f, fileName, "wb");
    if (!f)
    {
        printf("%s(0): error G0001: cannot write file %s\n", srcFileName, fileName);
    }
    else
    {
        fwrite(content, 1, strlen(content), f);
        fclose(f);
    }
}

#define PARSE_HANDLER(NAME) \
    Node* NAME(StringSpan const& text)

typedef PARSE_HANDLER((*ParseHandler));

PARSE_HANDLER(parseTemplateFile)
{
    // Read a template node!
    return readInput(text.begin(), text.end());
}

PARSE_HANDLER(parseCxxFile)
{
    // TODO: "scrape" the source file for metadata
    return nullptr;
}

PARSE_HANDLER(parseUnknownFile)
{
    // Don't process files we don't know how to handle.
    return nullptr;
}

// Information about a source file
struct SourceFile
{
    char const* inputPath;
    StringSpan  text;
    Node*       node;
};

Node* parseSourceFile(SourceFile* file)
{
    auto path = file->inputPath;
    auto text = file->text;

    static const struct
    {
        char const*     extension;
        ParseHandler    handler;
    } kHandlers[] =
    {
        { ".meta.slang",    &parseTemplateFile },
        { ".meta.cpp",      &parseTemplateFile },
        { ".cpp",           &parseCxxFile },
        { "",               &parseUnknownFile },
    };

    for (auto hh : kHandlers)
    {
        if (UnownedTerminatedStringSlice(path).endsWith(hh.extension))
        {
            return hh.handler(text);
        }
    }

    return nullptr;
}



SourceFile* parseSourceFile(char const* path)
{
    FILE* inputStream;
    fopen_s(&inputStream, path, "rb");
    fseek(inputStream, 0, SEEK_END);
    size_t inputSize = ftell(inputStream);
    fseek(inputStream, 0, SEEK_SET);

    char* input = (char*)malloc(inputSize + 1);
    fread(input, inputSize, 1, inputStream);
    input[inputSize] = 0;

    char const* inputEnd = input + inputSize;
    StringSpan span = StringSpan(input, inputEnd);

    SourceFile* sourceFile = new SourceFile();
    sourceFile->inputPath = path;
    sourceFile->text = span;

    Node* node = parseSourceFile(sourceFile);

    sourceFile->node = node;
    return sourceFile;
}

List<SourceFile*> gSourceFiles;

int main(
    int     argc,
    char**  argv)
{
    // Parse command-line arguments.
    char** argCursor = argv;
    char** argEnd = argv + argc;

    char const* appName = "slang-generate";
    if( argCursor != argEnd )
    {
        appName = *argCursor++;
    }

    char** writeCursor = argv;
    char const* const* inputPaths = writeCursor;

    while(argCursor != argEnd)
    {
        *writeCursor++ = *argCursor++;
    }

    size_t inputPathCount = writeCursor - inputPaths;
    if(inputPathCount == 0)
    {
        usage(appName);
        exit(1);
    }

    if( argCursor != argEnd )
    {
        usage(appName);
        exit(1);
    }

    // Read each input file and process it according
    // to the type of treatment it requires.
    for (size_t ii = 0; ii < inputPathCount; ++ii)
    {
        char const* inputPath = inputPaths[ii];
        SourceFile* sourceFile = parseSourceFile(inputPath);
        if (sourceFile)
        {
            gSourceFiles.Add(sourceFile);
        }
    }

    // Once all inputs have been read, we can start
    // to produce output files by expanding templates.
    for (auto sourceFile : gSourceFiles)
    {
        auto inputPath = sourceFile->inputPath;
        auto node = sourceFile->node;

        // write output to a temporary file first
        char outputPath[1024];
        sprintf_s(outputPath, "%s.temp.h", inputPath);

        FILE* outputStream;
        fopen_s(&outputStream, outputPath, "w");

        emitTemplateNodes(outputStream, node);

        fclose(outputStream);

        // update final output only when content has changed
        char outputPathFinal[1024];
        sprintf_s(outputPathFinal, "%s.h", inputPath);

        char * allTextOld = readAllText(outputPathFinal);
        char * allTextNew = readAllText(outputPath);
        if (strcmp(allTextNew, allTextOld) != 0)
        {
            writeAllText(inputPath, outputPathFinal, allTextNew);
        }
        remove(outputPath);
    }

    return 0;
}
