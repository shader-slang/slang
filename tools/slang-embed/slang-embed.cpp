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

// The utility is implemented as a single `struct` type
// that provides a context for the code. We do this as
// an alternative to using global variables for passing
// around options easily.
//
struct App
{
    char const* appName = "slang-embed";
    char const* inputPath = nullptr;

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

    void processInputFile()
    {
        // Note: Eventually we might support multiple input files in a
        // single invocation of the tool, but for now we only have
        // a single file to process.

        // We open the input file in text mode because we are currently
        // embedding textual source files. If/when this utility gets
        // used for binary files another mode could be called for.
        //
        // (Alternatively, we might always use binary mode, but this
        // could lead to a difference in the embedded bytes based on
        // the line ending convention of the host platform)
        //
        FILE* inputFile = fopen(inputPath, "r");
        if( !inputFile )
        {
            fprintf(stderr, "%s: error: failed to open '%s' for reading\n", appName, inputPath);
            exit(1);
        }

        // We derive an output path simply by appending `.cpp` to the input path.
        //
        // TODO: If we start adding more complicated options, a `-o` option
        // to specify a desired output path would be an obvious choice.
        //
        char* outputPath = (char*) malloc(strlen(inputPath) + strlen(".cpp") + 1);
        strcpy(outputPath, inputPath);
        strcat(outputPath, ".cpp");

        FILE* outputFile = fopen(outputPath, "w");
        if( !outputPath )
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
        char* variableName = (char*) malloc(strlen(fileName));
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
        fprintf(outputFile, "const char* %s =\n", variableName);

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

        fprintf(outputFile, "\"");
        for( ;;)
        {
            int c = fgetc(inputFile);
            if( c == EOF )
                break;

            // Based on the byte that we are trying to emit,
            // we may need to emit an escape sequence.
            //
            switch( c )
            {
            // The common C escape sequencs are handled directly.
            //
            case '"':   fprintf(outputFile, "\\\"");        break;
            case '\n':  fprintf(outputFile, "\\n\"\n\"");   break;
            case '\t':  fprintf(outputFile, "\\t");         break;

            default:
                // For all other cases, we detect if the byte
                // is in the printable ASCII range, and emit
                // it directly if sco.
                //
                if( c >= 32 && c <= 126 )
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
        fprintf(outputFile, "\";\n");

        fclose(outputFile);
        fclose(inputFile);
    }
};

int main(int argc, char** argv)
{
    App app;
    app.parseOptions(argc, argv);
    app.processInputFile();
    return 0;
}
