#include "../../source/core/slang-io.h"
#include "../../source/slang-record-replay/replay-stream-decoder.h"

#include <memory>
#include <stdio.h>

struct Options
{
    bool convertToJson{false};
    bool decode{false};
    Slang::String recordFileName;
    Slang::String outputFileName;
};

void printUsage()
{
    printf("Usage: slang-replay [options] <record-file>\n");
    printf("Options:\n");
    printf(
        "  --convert-json, -cj: Convert the record file to a JSON file in the same directory with record file.\n\
                       When this option is set, it won't replay the record file.\n");
    printf("  --decode, -d: Decode the binary stream.bin file to human-readable text.\n");
    printf("  --output, -o <file>: Write decoded output to the specified file instead of stdout.\n");
}

Options parseOption(int argc, char* argv[])
{
    Options option;
    char const* arg{};
    if (argc <= 1)
    {
        printUsage();
        exit(1);
    }

    int argIndex = 1;
    while (argIndex < argc)
    {
        arg = argv[argIndex];

        // For anything not starting with a '-', it is a file name
        if (arg[0] != '-')
        {
            option.recordFileName = arg;
            argIndex++;
        }
        else if ((strcmp("--convert-json", arg) == 0) || (strcmp("-cj", arg) == 0))
        {
            option.convertToJson = true;
            argIndex++;
        }
        else if ((strcmp("--decode", arg) == 0) || (strcmp("-d", arg) == 0))
        {
            option.decode = true;
            argIndex++;
        }
        else if ((strcmp("--output", arg) == 0) || (strcmp("-o", arg) == 0))
        {
            argIndex++;
            if (argIndex >= argc)
            {
                printf("Error: --output requires a filename argument\n");
                printUsage();
                exit(1);
            }
            option.outputFileName = argv[argIndex];
            argIndex++;
        }
        else if ((strcmp("--help", arg) == 0) || (strcmp("-h", arg) == 0))
        {
            printUsage();
            exit(0);
        }
        else
        {
            // Unknown option
            printf("Unknown option: %s\n", arg);
            printUsage();
            exit(1);
        }
    }

    if (option.recordFileName.getLength() == 0)
    {
        printUsage();
        exit(1);
    }

    return option;
}

int main(int argc, char* argv[])
{
    Options options = parseOption(argc, argv);

    if (options.decode)
    {
        // Decode the binary stream to human-readable text
        try
        {
            Slang::String decoded = SlangRecord::ReplayStreamDecoder::decodeFile(
                options.recordFileName.getBuffer());
            
            if (options.outputFileName.getLength() > 0)
            {
                // Write to file
                SlangResult res = Slang::File::writeAllText(
                    options.outputFileName.getBuffer(), 
                    decoded.getUnownedSlice());
                if (SLANG_FAILED(res))
                {
                    fprintf(stderr, "Error writing to file: %s\n", options.outputFileName.getBuffer());
                    return 1;
                }
            }
            else
            {
                // Write to stdout
                printf("%s", decoded.getBuffer());
            }
            return 0;
        }
        catch (const Slang::Exception& e)
        {
            fprintf(stderr, "Error decoding file: %s\n", e.Message.getBuffer());
            return 1;
        }
    }

    // TODO: Add replay functionality

    return 0;
}
