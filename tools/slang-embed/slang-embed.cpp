// slang-embed.cpp

// This file implements a simple utility for taking an input file
// and embedding into a C++ source file as a `static const` array.

// For now this utility uses plain C stdlib functionality rather
// than depending on any of the utiltiies from the Slang project
// libraries.
//
#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../source/core/slang-list.h"
#include "../../source/core/slang-string.h"
#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-dictionary.h"

// Utility to free pointers on scope exit
struct ScopedMemory
{
    ScopedMemory(void* ptr)
        : ptr(ptr)
    {}

    ~ScopedMemory()
    {
        if(ptr) free(ptr);
    }

    void* ptr;
};

// Utility to close file on scope exit
struct ScopedFile
{
    ScopedFile(FILE* file)
        : file(file)
    {}

    ~ScopedFile()
    {
        if(file) fclose(file);
    }

    FILE* file;
};

// The utility is implemented as a single `struct` type
// that provides a context for the code. We do this as
// an alternative to using global variables for passing
// around options easily.
//
struct App
{
    char const* appName = "slang-embed";
    char const* inputPath = nullptr;
    Slang::HashSet<Slang::String> includedFiles;

    void parseOptions(int argc, char** argv)
    {
        // Options are currently all specified by position,
        // so the parsing logic is simplistic.

        if( argc > 0 )
        {
            appName = *argv++;
            argc--;
        }

        if( argc > 0 )
        {
            inputPath = *argv++;
            argc--;
        }

        if( !inputPath || (argc != 0) )
        {
            fprintf(stderr, "usage: %s <inputPath>\n", appName);
            exit(1);
        }
    }
    size_t charCount = 0;
    bool useNewStringLit = true;
    void processInputFile(FILE* outputFile, Slang::String inputPath)
    {
        using namespace Slang;

        String canonicalPath;
        if (SLANG_SUCCEEDED(Slang::Path::getCanonical(inputPath, canonicalPath)))
        {
            if (!includedFiles.add(canonicalPath))
                return;
        }

        // We open the input file in text mode because we are currently
        // embedding textual source files. If/when this utility gets
        // used for binary files another mode could be called for.
        //
        // (Alternatively, we might always use binary mode, but this
        // could lead to a difference in the embedded bytes based on
        // the line ending convention of the host platform)
        //

        String contents;
        {
            auto res = File::readAllText(inputPath, contents);
            SLANG_ASSERT(SLANG_SUCCEEDED(res));
        }

        LineParser lineReader(contents.getUnownedSlice());

        for (auto line : lineReader)
        {
            auto trimedLine = line.trimStart();
            if (trimedLine.startsWith("#include"))
            {
                auto fileName =
                    Slang::StringUtil::getAtInSplit(trimedLine, ' ', 1);
                if (fileName[0] == '<')
                    goto normalProcess;
                fileName = Slang::UnownedStringSlice(fileName.begin() + 1, fileName.end() - 1);
                auto path =
                    Slang::Path::combine(Slang::Path::getParentDirectory(inputPath), fileName);
                if (!Slang::File::exists(path))
                    goto normalProcess;
                processInputFile(outputFile, path.getUnownedSlice());
                continue;
            }
        normalProcess:;
            if (!useNewStringLit && charCount + line.getLength() > 0x4000)
            {
                charCount = 0;
                useNewStringLit = true;
                fprintf(outputFile, ";\n");
            }
            if (useNewStringLit)
            {
                fprintf(outputFile, "sb << \n\"");
                useNewStringLit = false;
            }
            else
            {
                fprintf(outputFile, "\"");
            }
            charCount += line.getLength();
            for (auto c : line)
            {
                // Based on the byte that we are trying to emit,
                // we may need to emit an escape sequence.
                //
                switch (c)
                {
                // The common C escape sequencs are handled directly.
                //
                case '"':
                    fprintf(outputFile, "\\\"");
                    break;
                case '\n':
                    fprintf(outputFile, "\\n");
                    break;
                case '\t':
                    fprintf(outputFile, "\\t");
                    break;
                case '\\':
                    fprintf(outputFile, "\\\\");
                    break;
                default:
                    // For all other cases, we detect if the byte
                    // is in the printable ASCII range, and emit
                    // it directly if sco.
                    //
                    if (c >= 32 && c <= 126)
                    {
                        fputc(c, outputFile);
                    }
                    else
                    {
                        // Otherwise, we emit the byte as an octal
                        // escape sequence, being sure to emit a
                        // full three digits to avoid errorneous
                        // encoding if the following byte might
                        // represent a digit.
                        //
                        fprintf(outputFile, "\\%03o", c);
                    }
                    break;
                }
            }
            fprintf(outputFile, "\\n\"\n");
        }
    }

    void processInputFile()
    {
        // Note: Eventually we might support multiple input files in a
        // single invocation of the tool, but for now we only have
        // a single file to process.

        // We derive an output path simply by appending `.cpp` to the input path.
        //
        // TODO: If we start adding more complicated options, a `-o` option
        // to specify a desired output path would be an obvious choice.
        //
        char* outputPath = (char*) malloc(strlen(inputPath) + strlen(".cpp") + 1);
        ScopedMemory outputPathCleanup(outputPath);
        strcpy(outputPath, inputPath);
        strcat(outputPath, ".cpp");

        FILE* outputFile = fopen(outputPath, "w");
        ScopedFile outputFileCleanup(outputFile);
        if( !outputFile )
        {
            fprintf(stderr, "%s: error: failed to open '%s' for reading\n", appName, outputPath);
            exit(1);
        }

        // We want to derive a variable name based on the name of
        // the input file we are mbedded. Toward this end, we
        // start by trying to strip off any leading directories
        // in the path. This logic is ad hoc but should suffice,
        // given that we don't plan to give the files we embed
        // unconventional names.
        //
        char const* fileName = inputPath;
        if(auto pos = strrchr(fileName, '\\'))
            fileName = pos+1;
        if(auto pos = strrchr(fileName, '/'))
            fileName = pos+1;

        // The variable name will start as a copy of the file
        // name, although we will immediately drop any extension
        // that comes after a `.` to trim the name further.
        //
        char* variableName = (char*) malloc(strlen(fileName)+1);
        ScopedMemory variableNameCleanup(variableName);
        strcpy(variableName, fileName);
        if(auto pos = strchr(variableName, '.'))
            *pos = 0;

        // We will also replace any `-` in the file name with
        // a `_` in the generate variable name, so that the
        // tool will be compatible with our current naming
        // convention of using `-` as the separator in file names.
        //
        for( auto cursor = variableName; *cursor; ++cursor)
        {
            switch( *cursor )
            {
            default:
                break;
            case '-': *cursor = '_';
            }
        }

        // With all the preliminaries out of the way, the actual
        // task of outputting the generated source file is simple.
        //
        fprintf(outputFile, "// generated code; do not edit\n");
        fprintf(outputFile, "#include \"../source/core/slang-basic.h\"\n");

        fprintf(outputFile, "Slang::String get_%s()\n", variableName);
        fprintf(outputFile, "{\n");
        fprintf(outputFile, "Slang::StringBuilder sb;\n");

        // Note: For now we are embedding the file as a string
        // literal, with full knowledge that this strategy
        // will run into limitations in certain compilers
        // (e.g., some versions of the Visual C++ compiler
        // don't handle string literals larger than 64KB).
        //
        // TODO: Eventually we should replace this logic with
        // code to emit a plain array of `unsigned char` with
        // an array initializer list `{ ... }`. While some
        // compilers have limitations or performance issues
        // with large array literals, the practical limits
        // appear to be higher than they are for string literals.

        processInputFile(outputFile, Slang::UnownedStringSlice(inputPath));

        fprintf(outputFile, ";\n");
        fprintf(outputFile, "return sb.produceString();\n}\n");
    }
};

int main(int argc, char** argv)
{
    App app;
    app.parseOptions(argc, argv);
    app.processInputFile();
    return 0;
}
