// main.cpp

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../source/core/secure-crt.h"

struct StringSpan
{
    char const* begin;
    char const* end;
};

StringSpan makeEmptySpan()
{
    StringSpan span = { 0, 0 };
    return span;
}

StringSpan makeSpan(char const* begin, char const* end)
{
    StringSpan span;
    span.begin = begin;
    span.end = end;
    return span;
}

struct Node
{
    // The textual range covered by this node
    // (does not including the opening sigil)
    StringSpan  span;

    // The textual range of the identifier
    // part of this node (if any)
    StringSpan  id;

    // The textual range of the body part of
    // this node (if any)
    StringSpan  body;

    // The parent of this node
    Node*       parent;

    // The first child node of this node
    Node*       firstChild;

    // The next node belonging to the same parent
    Node*       nextSibling;
};

struct NodeBuilder
{
    Node*           node;
    Node**          childLink;
    unsigned int    curlyCount;
    unsigned int    nestedCurlyCount;
};

Node* createNode()
{
    Node* result = (Node*) malloc(sizeof(Node));
    memset(result, 0, sizeof(Node));
    return result;
}

void addNode(
    NodeBuilder*    builder,
    Node*           node)
{
    node->parent = builder->node;

    *builder->childLink = node;
    builder->childLink = &node->nextSibling;
}

bool isAlpha(int c)
{
    return ((c >= 'a') && (c <= 'z'))
        || ((c >= 'A') && (c <= 'Z'))
        || (c == '_');
}

Node* readInput(
    char const* inputBegin,
    char const* inputEnd)
{
    static const int kMaxDepth = 16;
    NodeBuilder nodeStack[kMaxDepth];
    NodeBuilder* nodeStackEnd = &nodeStack[kMaxDepth];

    Node* root = createNode();
    root->span.begin = inputBegin;
    root->span.end = inputEnd;
    root->body = root->span;

    NodeBuilder* builder = &nodeStack[0];

    builder->node = root;
    builder->childLink = &root->firstChild;
    builder->curlyCount = (unsigned int)(-1);
    builder->nestedCurlyCount = 0;

    char const* cursor = inputBegin;

    for(;;)
    {
        int c = *cursor;
        switch(c)
        {
        default:
            // ordinary text, so we continue the current span
            cursor++;
            continue;

        case 0:
            // possible end of input
            if(cursor == inputEnd)
            {
                return root;
            }
            // Otherwise it is just an embedded NULL
            cursor++;
            continue;

        case '$':
            // We've hit our dedicated meta-character, which means
            // we are being asked to do some kind of splicing.
            {
                cursor++;

                switch(*cursor)
                {
                case '$':
                    // This is an escaped single `$`.
                    // We need to create an empty node to
                    // represent it
                    {
                        Node* node = createNode();
                        addNode(builder, node);
                        node->span.begin = cursor;
                        cursor++;
                        node->span.end = cursor;
                        continue;
                    }

                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                case '\'':
                case '\"':
                case ')':
                    // HACK: allow existing usage through
                    cursor++;
                    continue;

                default:
                    break;
                }

                Node* node = createNode();
                addNode(builder, node);

                node->span.begin = cursor-1;

                char const* nodeBegin = cursor;

                // Piece one is an optional "identifier" section
                node->id.begin = cursor;
                while( isAlpha(*cursor) )
                {
                    cursor++;
                }
                node->id.end = cursor;

                // Next we have an optional `{}`-delimeted span
                if( *cursor == '{' )
                {
                    unsigned int count = 0;
                    while( *cursor == '{' )
                    {
                        count++;
                        cursor++;
                    }
                    node->body.begin = cursor;

                    assert(builder != nodeStackEnd);

                    builder++;
                    builder->node = node;
                    builder->childLink = &node->firstChild;
                    builder->curlyCount = count;
                    builder->nestedCurlyCount = 0;
                }
                else
                {
                    node->body.begin = cursor;
                    node->body.end = cursor;
                    node->span.end = cursor;
                }

                continue;
            }
            break;

        case '{':
            builder->nestedCurlyCount++;
            cursor++;
            continue;

        case '}':
            {
                // Possible end of an open span

                unsigned int count = 0;
                char const* cc = cursor;
                while( *cc == '}' )
                {
                    count++;
                    cc++;
                }

                unsigned int expected = builder->curlyCount;

                if( expected == 1 )
                {
                    unsigned int nested = builder->nestedCurlyCount;

                    // The user isn't guarding for unmatched braces,
                    // so some of these braces might go to cancel
                    // out any open braces inside this scope:
                    if( count > nested )
                    {
                        // There are more available braces than our
                        // nesting depth, so we need to close them
                        // out and move on.
                        cursor += builder->nestedCurlyCount;
                        count -= builder->nestedCurlyCount;
                        builder->nestedCurlyCount = 0;
                    }
                    else
                    {
                        // These braces are only being used to close out
                        // nested constructs that were already opened.
                        builder->nestedCurlyCount -= count;
                        cursor += count;
                        continue;
                    }
                }

                if(count >= expected)
                {
                    // There are enough braces there to close out this construct

                    Node* node = builder->node;
                    node->body.end = cursor;

                    cursor += expected;
                    node->span.end = cursor;

                    builder--;
                    continue;
                }
                else
                {
                    cursor += count;
                    continue;
                }
            }
        }

    }
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

void emitNode(
    FILE*   stream,
    Node*   node)
{
    // TODO: need to look at the identifier part of the node in case
    // there are custom instructions there...

    char const* cursor = node->body.begin;

    for( auto nn = node->firstChild; nn; nn = nn->nextSibling )
    {
        emitCode(stream, cursor, nn->span.begin);

        cursor = nn->span.end;
    }

    emitCode(stream, cursor, node->body.end);
}

void emitBody(
    FILE*   stream,
    Node*   node)
{
    char const* cursor = node->body.begin;

    for( auto nn = node->firstChild; nn; nn = nn->nextSibling )
    {
        emitRaw(stream, cursor, nn->span.begin);

        emitNode(stream, nn);

        cursor = nn->span.end;
    }

    emitRaw(stream, cursor, node->body.end);
}

void usage(char const* appName)
{
    fprintf(stderr, "usage: %s <input>\n", appName);
}

int main(
    int     argc,
    char**  argv)
{
    char** argCursor = argv;
    char** argEnd = argv + argc;

    char const* appName = "slang-generate";
    if( argCursor != argEnd )
    {
        appName = *argCursor++;
    }

    char const* inputPath = nullptr;
    if( argCursor != argEnd )
    {
        inputPath = *argCursor++;
    }
    else
    {
        usage(appName);
        exit(1);
    }

    if( argCursor != argEnd )
    {
        usage(appName);
        exit(1);
    }

    // Read the contents o the file and translate it into a "template" file

    FILE* inputStream;
    fopen_s(&inputStream, inputPath, "rb");
    fseek(inputStream, 0, SEEK_END);
    size_t inputSize = ftell(inputStream);
    fseek(inputStream, 0, SEEK_SET);

    char* input = (char*) malloc(inputSize + 1);
    fread(input, inputSize, 1, inputStream);
    input[inputSize] = 0;

    char const* inputEnd = input + inputSize;

    Node* node = readInput(input, inputEnd);

    char outputPath[1024];
    sprintf_s(outputPath, "%s.h", inputPath);

    FILE* outputStream;
    fopen_s(&outputStream, outputPath, "w");

    emitBody(outputStream, node);

    fclose(outputStream);

    return 0;
}
