#include "../../source/core/slang-io.h"
#include "../../source/slang-record-replay/replay-context.h"
#include "../../source/slang-record-replay/replay-stream-decoder.h"

#if SLANG_ENABLE_SLANG_RHI
#include "driver-replay.h"
#endif

#include <memory>
#include <stdio.h>
#include <string.h>

struct Options
{
    bool decode{false};
    bool rawDecode{false};
    bool replay{false};
    bool verbose{false};
    Slang::String recordFileName;
    Slang::String outputFileName;
#if SLANG_ENABLE_SLANG_RHI
    Slang::String replayDriver;
#endif
};

void printUsage()
{
    printf("Usage: slang-replay [options] <record-file>\n");
    printf("Options:\n");
    printf(
        "  --decode, -d: Decode the binary stream to human-readable text.\n");
    printf(
        "                If given a folder with index.bin, uses structured call-by-call output.\n");
    printf("  --raw, -R: Force raw value-by-value output (ignore index.bin even if present).\n");
    printf("  --replay, -r: Replay the recorded API calls.\n");
    printf("  --verbose, -v: Enable verbose output during replay.\n");
    printf(
        "  --output, -o <file>: Write decoded output to the specified file instead of stdout.\n");
#if SLANG_ENABLE_SLANG_RHI
    printf("  --replay-driver <type>: After replaying each shader compilation, create the\n");
    printf("                          compute pipeline on a real GPU driver via slang-rhi.\n");
    printf("                          Supported types: vulkan, d3d12, d3d11, metal, cuda\n");
    printf("                          Implies --replay.\n");
#endif
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
#if SLANG_ENABLE_SLANG_RHI
        else if (strcmp("--replay-driver", arg) == 0)
        {
            argIndex++;
            if (argIndex >= argc)
            {
                printf("Error: --replay-driver requires a device type argument\n");
                printUsage();
                exit(1);
            }
            option.replayDriver = argv[argIndex];
            option.replay = true;
            argIndex++;
        }
#endif
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

#if SLANG_ENABLE_SLANG_RHI
static rhi::DeviceType parseDeviceType(const char* name)
{
    if (strcmp(name, "vulkan") == 0 || strcmp(name, "vk") == 0)
        return rhi::DeviceType::Vulkan;
    if (strcmp(name, "d3d12") == 0 || strcmp(name, "dx12") == 0)
        return rhi::DeviceType::D3D12;
    if (strcmp(name, "d3d11") == 0 || strcmp(name, "dx11") == 0)
        return rhi::DeviceType::D3D11;
    if (strcmp(name, "metal") == 0)
        return rhi::DeviceType::Metal;
    if (strcmp(name, "cuda") == 0)
        return rhi::DeviceType::CUDA;
    return rhi::DeviceType::Default;
}

static void driverReplayCallback(
    const char* signature,
    uint64_t thisHandle,
    void* userData)
{
    if (strcmp(signature, "ComponentTypeProxy::getEntryPointCode") != 0)
        return;

    auto* driverReplay = static_cast<DriverReplay*>(userData);
    auto& ctx = SlangRecord::ReplayContext::get();

    ISlangUnknown* proxy = ctx.getProxy(thisHandle);
    if (!proxy)
        return;

    // Unwrap the proxy to get the real Slang IComponentType implementation.
    ISlangUnknown* impl = SlangRecord::unwrapObject(proxy);
    if (!impl)
        return;

    slang::IComponentType* componentType = nullptr;
    impl->queryInterface(slang::IComponentType::getTypeGuid(), (void**)&componentType);
    if (!componentType)
        return;
    componentType->release();

    driverReplay->tryCreateComputePipeline(componentType);
}
#endif

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

                // If given a folder (check by seeing if stream.bin exists inside), append
                // stream.bin
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
                    fprintf(
                        stderr,
                        "Error writing to file: %s\n",
                        options.outputFileName.getBuffer());
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
            // The input can be either a folder containing stream.bin or the stream.bin file
            // directly
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

#if SLANG_ENABLE_SLANG_RHI
            // Create the RHI device BEFORE loading the replay. The replay
            // context is still Idle here, so Slang API calls made by the RHI
            // device during initialization won't interfere with the proxy
            // record/replay layer.
            std::unique_ptr<DriverReplay> driverReplay;
            if (options.replayDriver.getLength() > 0)
            {
                rhi::DeviceType deviceType = parseDeviceType(options.replayDriver.getBuffer());
                if (deviceType == rhi::DeviceType::Default)
                {
                    fprintf(
                        stderr,
                        "Error: unknown device type '%s'\n",
                        options.replayDriver.getBuffer());
                    printUsage();
                    return 1;
                }

                driverReplay = std::make_unique<DriverReplay>();
                SlangResult initResult = driverReplay->init(deviceType);
                if (SLANG_FAILED(initResult))
                {
                    fprintf(stderr, "Error: failed to initialize driver replay\n");
                    return 1;
                }
            }
#endif

            // Load and execute the replay
            SlangResult loadResult =
                ctx.loadReplay(Slang::Path::getParentDirectory(streamPath).getBuffer());
            if (SLANG_FAILED(loadResult))
            {
                fprintf(stderr, "Error loading replay file\n");
                return 1;
            }

#if SLANG_ENABLE_SLANG_RHI
            if (driverReplay)
            {
                ctx.setPostCallCallback(driverReplayCallback, driverReplay.get());
            }
#endif

            printf("Executing replay...\n");
            ctx.executeAll();
            printf("Replay completed successfully.\n");

            int exitCode = 0;
#if SLANG_ENABLE_SLANG_RHI
            if (driverReplay)
            {
                ctx.setPostCallCallback(nullptr, nullptr);
                driverReplay->printSummary();
                if (driverReplay->getFailCount() > 0)
                    exitCode = 1;
            }

            // Reset the replay context back to Idle before destroying the
            // RHI device. The device holds proxy-wrapped Slang sessions whose
            // destructors go through the proxy layer; if the context is still
            // in Playback mode those calls would try to read from the stream.
            ctx.reset();
            driverReplay.reset();
#endif

            return exitCode;
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
