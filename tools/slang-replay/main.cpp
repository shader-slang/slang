#include "../../source/core/slang-io.h"
#include "../../source/slang-record-replay/replay-context.h"
#include "../../source/slang-record-replay/replay-stream-decoder.h"

#include <memory>
#include <stdio.h>

struct Options
{
    bool convertToJson{false};
    bool decode{false};
    bool rawDecode{false};
    bool replay{false};
    bool verbose{false};
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
    printf("  --decode, -d: Decode the binary stream to human-readable text.\n");
    printf("                If given a folder with index.bin, uses structured call-by-call output.\n");
    printf("  --raw, -R: Force raw value-by-value output (ignore index.bin even if present).\n");
    printf("  --replay, -r: Replay the recorded API calls.\n");
    printf("  --verbose, -v: Enable verbose output during replay.\n");
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
        else if ((strcmp("--raw", arg) == 0) || (strcmp("-R", arg) == 0))
        {
            option.rawDecode = true;
            argIndex++;
        }
        else if ((strcmp("--replay", arg) == 0) || (strcmp("-r", arg) == 0))
        {
            option.replay = true;
            argIndex++;
        }
        else if ((strcmp("--verbose", arg) == 0) || (strcmp("-v", arg) == 0))
        {
            option.verbose = true;
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
            Slang::String decoded;
            
            if (options.rawDecode)
            {
                // Raw mode: simple value-by-value dump
                Slang::String inputPath = options.recordFileName;
                
                // If given a folder (check by seeing if stream.bin exists inside), append stream.bin
                Slang::String possibleStreamPath = Slang::Path::combine(inputPath, "stream.bin");
                if (Slang::File::exists(possibleStreamPath))
                {
                    inputPath = possibleStreamPath;
                }
                
                decoded = SlangRecord::ReplayStreamDecoder::decodeFile(inputPath.getBuffer());
            }
            else
            {
                // Default: use index-based structured output if available
                decoded = SlangRecord::ReplayStreamDecoder::decodeWithIndex(
                    options.recordFileName.getBuffer());
            }
            
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

    if (options.replay)
    {
        // Replay the recorded API calls
        try
        {
            auto& ctx = SlangRecord::ReplayContext::get();
            
            // Enable verbose logging if requested
            if (options.verbose)
            {
                ctx.setTtyLogging(true);
            }
            
            // Load the replay file
            // The input can be either a folder containing stream.bin or the stream.bin file directly
            Slang::String streamPath = options.recordFileName;
            if (!streamPath.endsWith(".bin"))
            {
                streamPath = Slang::Path::combine(streamPath, "stream.bin");
            }
            
            if (!Slang::File::exists(streamPath))
            {
                fprintf(stderr, "Error: stream.bin not found at: %s\n", streamPath.getBuffer());
                return 1;
            }
            
            printf("Loading replay from: %s\n", streamPath.getBuffer());
            
            // Load and execute the replay
            SlangResult loadResult = ctx.loadReplay(
                Slang::Path::getParentDirectory(streamPath).getBuffer());
            if (SLANG_FAILED(loadResult))
            {
                fprintf(stderr, "Error loading replay file\n");
                return 1;
            }
            
            printf("Executing replay...\n");
            ctx.executeAll();
            printf("Replay completed successfully.\n");
            
            return 0;
        }
        catch (const Slang::Exception& e)
        {
            fprintf(stderr, "Error during replay: %s\n", e.Message.getBuffer());
            return 1;
        }
    }

    // Default: print usage if no operation specified
    printUsage();
    return 1;
}
