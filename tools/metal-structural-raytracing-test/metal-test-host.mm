#include "metal-test-host.h"

#include "metal-test-scenes.h"

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <cstdint>
#include <cstdio>

namespace
{

struct TraceProgramResources
{
    uint64_t intersectionFunctions;
    uint64_t missFunctions;
    uint64_t closestHitFunctions;
    uint64_t callableFunctions;
    uint64_t records;
};

struct FrameParameters
{
    uint64_t scene;
    uint64_t programResources;
    uint64_t results;
};

struct NativeProgram
{
    id<MTLComputePipelineState> pipeline;
    id<MTLIntersectionFunctionTable> intersectionTable;
    id<MTLVisibleFunctionTable> missTable;
    id<MTLVisibleFunctionTable> closestHitTable;
    id<MTLVisibleFunctionTable> callableTable;
};

struct ProgramDescription
{
    const char* const* intersectionFunctions;
    uint32_t intersectionFunctionCount;
    const char* const* missFunctions;
    uint32_t missFunctionCount;
    const char* const* closestHitFunctions;
    uint32_t closestHitFunctionCount;
    const char* const* callableFunctions;
    uint32_t callableFunctionCount;
};

bool fail(NSString* message)
{
    std::fprintf(stderr, "metal-structural-raytracing-test: %s\n", message.UTF8String);
    return false;
}

NSString* sourcePath(NSString* directory, NSString* testName)
{
    return [directory stringByAppendingPathComponent:[testName stringByAppendingString:@".metal"]];
}

id<MTLLibrary> loadLibrary(id<MTLDevice> device, NSString* path)
{
    NSError* error = nil;
    NSString* source = [NSString stringWithContentsOfFile:path
                                                 encoding:NSUTF8StringEncoding
                                                    error:&error];
    if (!source)
    {
        fail(error.localizedDescription);
        return nil;
    }

    MTLCompileOptions* options = [MTLCompileOptions new];
    options.languageVersion = MTLLanguageVersion3_1;
    id<MTLLibrary> library = [device newLibraryWithSource:source options:options error:&error];
    if (!library)
        fail(error.localizedDescription);
    return library;
}

id<MTLFunction> loadIntersectionFunction(
    id<MTLLibrary> library,
    const char* name,
    NSError** outError)
{
    auto descriptor = [MTLIntersectionFunctionDescriptor new];
    descriptor.name = [NSString stringWithUTF8String:name];
    return [library newIntersectionFunctionWithDescriptor:descriptor error:outError];
}

id<MTLVisibleFunctionTable> createVisibleFunctionTable(
    id<MTLComputePipelineState> pipeline,
    NSArray<id<MTLFunction>>* functions)
{
    auto descriptor = [MTLVisibleFunctionTableDescriptor new];
    descriptor.functionCount = functions.count ? functions.count : 1;
    id<MTLVisibleFunctionTable> table = [pipeline newVisibleFunctionTableWithDescriptor:descriptor];
    for (NSUInteger i = 0; i < functions.count; ++i)
        [table setFunction:[pipeline functionHandleWithFunction:functions[i]] atIndex:i];
    return table;
}

bool createProgram(
    id<MTLDevice> device,
    NSString* path,
    const ProgramDescription& description,
    NativeProgram& outProgram)
{
    id<MTLLibrary> library = loadLibrary(device, path);
    if (!library)
        return false;

    id<MTLFunction> kernel = [library newFunctionWithName:@"main_0"];
    if (!kernel)
        return fail(@"the generated Metal library is missing main_0");

    NSMutableArray<id<MTLFunction>>* allFunctions = [NSMutableArray array];
    NSMutableArray<id<MTLFunction>>* intersectionFunctions = [NSMutableArray array];
    NSMutableArray<id<MTLFunction>>* missFunctions = [NSMutableArray array];
    NSMutableArray<id<MTLFunction>>* closestHitFunctions = [NSMutableArray array];
    NSMutableArray<id<MTLFunction>>* callableFunctions = [NSMutableArray array];

    NSError* error = nil;
    for (uint32_t i = 0; i < description.intersectionFunctionCount; ++i)
    {
        id<MTLFunction> function =
            loadIntersectionFunction(library, description.intersectionFunctions[i], &error);
        if (!function)
            return fail(error.localizedDescription);
        [intersectionFunctions addObject:function];
        [allFunctions addObject:function];
    }

    struct VisibleFunctionGroup
    {
        const char* const* names;
        uint32_t count;
        NSMutableArray<id<MTLFunction>>* functions;
    };
    VisibleFunctionGroup groups[] = {
        {description.missFunctions, description.missFunctionCount, missFunctions},
        {description.closestHitFunctions, description.closestHitFunctionCount, closestHitFunctions},
        {description.callableFunctions, description.callableFunctionCount, callableFunctions},
    };
    for (const auto& group : groups)
    {
        for (uint32_t i = 0; i < group.count; ++i)
        {
            id<MTLFunction> function =
                [library newFunctionWithName:[NSString stringWithUTF8String:group.names[i]]];
            if (!function)
                return fail(@"the generated Metal library is missing a visible function");
            [group.functions addObject:function];
            [allFunctions addObject:function];
        }
    }

    MTLLinkedFunctions* linkedFunctions = [MTLLinkedFunctions new];
    linkedFunctions.functions = allFunctions;
    MTLComputePipelineDescriptor* pipelineDescriptor = [MTLComputePipelineDescriptor new];
    pipelineDescriptor.computeFunction = kernel;
    pipelineDescriptor.linkedFunctions = linkedFunctions;
    outProgram.pipeline = [device newComputePipelineStateWithDescriptor:pipelineDescriptor
                                                                options:MTLPipelineOptionNone
                                                             reflection:nil
                                                                  error:&error];
    if (!outProgram.pipeline)
        return fail(error.localizedDescription);

    outProgram.missTable = createVisibleFunctionTable(outProgram.pipeline, missFunctions);
    outProgram.closestHitTable =
        createVisibleFunctionTable(outProgram.pipeline, closestHitFunctions);
    outProgram.callableTable = createVisibleFunctionTable(outProgram.pipeline, callableFunctions);

    auto intersectionTableDescriptor = [MTLIntersectionFunctionTableDescriptor new];
    intersectionTableDescriptor.functionCount =
        intersectionFunctions.count ? intersectionFunctions.count : 1;
    outProgram.intersectionTable = [outProgram.pipeline
        newIntersectionFunctionTableWithDescriptor:intersectionTableDescriptor];
    for (NSUInteger i = 0; i < intersectionFunctions.count; ++i)
    {
        [outProgram.intersectionTable
            setFunction:[outProgram.pipeline functionHandleWithFunction:intersectionFunctions[i]]
                atIndex:i];
    }

    return outProgram.missTable && outProgram.closestHitTable && outProgram.callableTable &&
           outProgram.intersectionTable;
}

id<MTLBuffer> createRecords(id<MTLDevice> device, const uint32_t* words, uint32_t wordCount)
{
    return [device newBufferWithBytes:words
                               length:sizeof(uint32_t) * wordCount
                              options:MTLResourceStorageModeShared];
}

id<MTLBuffer> createProgramResourceBuffer(
    id<MTLDevice> device,
    const NativeProgram& program,
    id<MTLBuffer> records)
{
    TraceProgramResources resources = {
        program.intersectionTable.gpuResourceID._impl,
        program.missTable.gpuResourceID._impl,
        program.closestHitTable.gpuResourceID._impl,
        program.callableTable.gpuResourceID._impl,
        records.gpuAddress,
    };
    return [device newBufferWithBytes:&resources
                               length:sizeof(resources)
                              options:MTLResourceStorageModeShared];
}

bool dispatch(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    const NativeProgram& program,
    id<MTLAccelerationStructure> scene,
    id<MTLBuffer> programResources,
    id<MTLBuffer> records,
    id<MTLBuffer> results,
    uint32_t threadCount,
    bool nestedFrame,
    bool callableOnly)
{
    id<MTLBuffer> frameBuffer = nil;
    if (nestedFrame)
    {
        FrameParameters frame = {
            scene.gpuResourceID._impl,
            programResources.gpuAddress,
            results.gpuAddress,
        };
        frameBuffer = [device newBufferWithBytes:&frame
                                          length:sizeof(frame)
                                         options:MTLResourceStorageModeShared];
    }

    id<MTLCommandBuffer> commandBuffer = [queue commandBuffer];
    id<MTLComputeCommandEncoder> encoder = [commandBuffer computeCommandEncoder];
    [encoder setComputePipelineState:program.pipeline];
    if (nestedFrame)
    {
        [encoder setBuffer:frameBuffer offset:0 atIndex:0];
    }
    else if (callableOnly)
    {
        [encoder setBuffer:programResources offset:0 atIndex:0];
        [encoder setBuffer:results offset:0 atIndex:1];
    }
    else
    {
        [encoder setAccelerationStructure:scene atBufferIndex:0];
        [encoder setBuffer:programResources offset:0 atIndex:1];
        [encoder setBuffer:results offset:0 atIndex:2];
    }

    if (scene)
        [encoder useResource:scene usage:MTLResourceUsageRead];
    if (frameBuffer)
        [encoder useResource:frameBuffer usage:MTLResourceUsageRead];
    [encoder useResource:programResources usage:MTLResourceUsageRead];
    [encoder useResource:program.intersectionTable usage:MTLResourceUsageRead];
    [encoder useResource:program.missTable usage:MTLResourceUsageRead];
    [encoder useResource:program.closestHitTable usage:MTLResourceUsageRead];
    [encoder useResource:program.callableTable usage:MTLResourceUsageRead];
    [encoder useResource:records usage:MTLResourceUsageRead];
    [encoder useResource:results usage:MTLResourceUsageWrite];
    [encoder dispatchThreads:MTLSizeMake(threadCount, 1, 1)
        threadsPerThreadgroup:MTLSizeMake(threadCount, 1, 1)];
    [encoder endEncoding];
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
    if (commandBuffer.status == MTLCommandBufferStatusError)
        return fail(commandBuffer.error.localizedDescription);
    return true;
}

bool validateResults(
    const char* testName,
    id<MTLBuffer> results,
    const uint32_t* expected,
    uint32_t wordCount)
{
    auto actual = static_cast<const uint32_t*>(results.contents);
    for (uint32_t i = 0; i < wordCount; ++i)
    {
        if (actual[i] != expected[i])
        {
            std::fprintf(
                stderr,
                "%s: result word %u is %u, expected %u\n",
                testName,
                i,
                actual[i],
                expected[i]);
            return false;
        }
    }
    std::printf("%s: passed\n", testName);
    return true;
}

bool runTriangleProgram(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    NSString* directory,
    const char* testName,
    const ProgramDescription& description,
    const uint32_t* recordsWords,
    uint32_t recordWordCount,
    const uint32_t* expected,
    uint32_t expectedWordCount,
    uint32_t threadCount,
    MTLAccelerationStructureInstanceOptions instanceOptions =
        MTLAccelerationStructureInstanceOptionOpaque,
    uint32_t userInstanceID = 0,
    const MTLPackedFloat4x3* transform = nullptr,
    bool nestedFrame = false)
{
    NativeProgram program = {};
    if (!createProgram(
            device,
            sourcePath(directory, [NSString stringWithUTF8String:testName]),
            description,
            program))
        return false;

    MetalRayTracingScene scene = {};
    NSString* sceneError = nil;
    if (!buildMetalTriangleScene(
            device,
            queue,
            instanceOptions,
            userInstanceID,
            transform,
            scene,
            &sceneError))
        return fail(sceneError);

    id<MTLBuffer> records = createRecords(device, recordsWords, recordWordCount);
    id<MTLBuffer> programResources = createProgramResourceBuffer(device, program, records);
    id<MTLBuffer> results = [device newBufferWithLength:expectedWordCount * sizeof(uint32_t)
                                                options:MTLResourceStorageModeShared];
    if (!dispatch(
            device,
            queue,
            program,
            scene.instanceAccelerationStructure,
            programResources,
            records,
            results,
            threadCount,
            nestedFrame,
            false))
        return false;
    return validateResults(testName, results, expected, expectedWordCount);
}

bool runTriangleHitMiss(id<MTLDevice> device, id<MTLCommandQueue> queue, NSString* directory)
{
    static const char* kMiss[] = {"RuntimeMiss_0"};
    static const char* kClosest[] = {"RuntimeClosestHit_0"};
    ProgramDescription description = {nullptr, 0, kMiss, 1, kClosest, 1, nullptr, 0};
    static const uint32_t kRecords[] = {1, 0};
    static const uint32_t kExpected[] = {1, 0, 2, 2, 0xffffffff, 2};
    return runTriangleProgram(
        device,
        queue,
        directory,
        "triangle-hit-miss",
        description,
        kRecords,
        2,
        kExpected,
        6,
        2,
        MTLAccelerationStructureInstanceOptionOpaque,
        0,
        nullptr,
        true);
}

bool runRecursiveTrace(id<MTLDevice> device, id<MTLCommandQueue> queue, NSString* directory)
{
    static const char* kMiss[] = {"RuntimeMiss_0"};
    static const char* kClosest[] = {"RuntimeClosestHit_0"};
    ProgramDescription description = {nullptr, 0, kMiss, 1, kClosest, 1, nullptr, 0};
    static const uint32_t kRecords[] = {1, 0};
    static const uint32_t kExpected[] = {21, 1, 2, 20, 0, 2};
    return runTriangleProgram(
        device,
        queue,
        directory,
        "recursive-trace",
        description,
        kRecords,
        2,
        kExpected,
        6,
        2);
}

bool runMultipleSlots(id<MTLDevice> device, id<MTLCommandQueue> queue, NSString* directory)
{
    static const char* kMiss[] = {"Miss0_0", "Miss1_0"};
    static const char* kClosest[] = {"ClosestHit0_0", "ClosestHit1_0"};
    ProgramDescription description = {nullptr, 0, kMiss, 2, kClosest, 2, nullptr, 0};
    static const uint32_t kRecords[] = {
        4,
        5,
        7,
        9,
        0,
        36,
        40,
        44,
        48,
        100,
        200,
        300,
        400,
    };
    static const uint32_t kExpected[] = {10, 100, 4, 11, 200, 4, 20, 300, 4, 21, 400, 4};
    return runTriangleProgram(
        device,
        queue,
        directory,
        "multiple-slots",
        description,
        kRecords,
        sizeof(kRecords) / sizeof(kRecords[0]),
        kExpected,
        sizeof(kExpected) / sizeof(kExpected[0]),
        4);
}

bool runTriangleAttributesFlags(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    NSString* directory)
{
    static const char* kIntersection[] = {"HitGroup_candidate_0"};
    static const char* kMiss[] = {"RuntimeMiss_0"};
    static const char* kClosest[] = {"RuntimeClosestHit_0"};
    ProgramDescription description = {kIntersection, 1, kMiss, 1, kClosest, 1, nullptr, 0};
    static const uint32_t kRecords[] = {1, 0};
    static const uint32_t kExpected[] = {
        3,  0, 25, 25, 0,  1, 10, 2,  0, 0, 0,  0,  1, 10, 2,  1, 25, 25, 0,  1, 10, 3,  1, 25,
        25, 0, 1,  10, 40, 0, 0,  0,  0, 1, 10, 2,  0, 0,  0,  0, 1,  10, 3,  0, 25, 25, 0, 1,
        10, 2, 0,  0,  0,  0, 1,  10, 3, 1, 25, 25, 0, 1,  10, 3, 0,  25, 25, 0, 1,  10,
    };
    return runTriangleProgram(
        device,
        queue,
        directory,
        "triangle-attributes-flags",
        description,
        kRecords,
        2,
        kExpected,
        sizeof(kExpected) / sizeof(kExpected[0]),
        10,
        MTLAccelerationStructureInstanceOptionNone);
}

bool runStageInputState(id<MTLDevice> device, id<MTLCommandQueue> queue, NSString* directory)
{
    static const char* kMiss[] = {"RuntimeMiss_0"};
    static const char* kClosest[] = {"RuntimeClosestHit_0"};
    ProgramDescription description = {nullptr, 0, kMiss, 1, kClosest, 1, nullptr, 0};
    static const uint32_t kRecords[] = {1, 0};
    static const uint32_t kExpected[] = {
        1, 1, 1000,   50,  100, 25, 100, 200, 50, 0, 0, 0, 17, 1, 0, 2,
        2, 1, 100000, 300, 100, 0,  0,   0,   0,  0, 0, 0, 0,  1, 1, 2,
    };
    MTLPackedFloat4x3 transform = {};
    transform.columns[0] = {2.0f, 0.0f, 0.0f};
    transform.columns[1] = {0.0f, 1.0f, 0.0f};
    transform.columns[2] = {0.0f, 0.0f, 1.0f};
    transform.columns[3] = {0.0f, 0.0f, 0.0f};
    return runTriangleProgram(
        device,
        queue,
        directory,
        "stage-input-state",
        description,
        kRecords,
        2,
        kExpected,
        sizeof(kExpected) / sizeof(kExpected[0]),
        2,
        MTLAccelerationStructureInstanceOptionTriangleFrontFacingWindingCounterClockwise |
            MTLAccelerationStructureInstanceOptionDisableTriangleCulling,
        17,
        &transform);
}

bool runCallableRecord(id<MTLDevice> device, id<MTLCommandQueue> queue, NSString* directory)
{
    static const char* kCallable[] = {"RuntimeCallable_0"};
    ProgramDescription description = {nullptr, 0, nullptr, 0, nullptr, 0, kCallable, 1};
    NativeProgram program = {};
    if (!createProgram(device, sourcePath(directory, @"callable-record"), description, program))
        return false;

    static const uint32_t kRecords[] = {0, 0, 0, 4, 20, 7};
    id<MTLBuffer> records = createRecords(device, kRecords, 6);
    id<MTLBuffer> programResources = createProgramResourceBuffer(device, program, records);
    id<MTLBuffer> results = [device newBufferWithLength:sizeof(uint32_t) * 2
                                                options:MTLResourceStorageModeShared];
    if (!dispatch(device, queue, program, nil, programResources, records, results, 1, false, true))
        return false;
    static const uint32_t kExpected[] = {22, 1};
    return validateResults("callable-record", results, kExpected, 2);
}

bool runProceduralHitFilter(id<MTLDevice> device, id<MTLCommandQueue> queue, NSString* directory)
{
    static const char* kIntersection[] = {"HitGroup_candidate_0"};
    static const char* kMiss[] = {"RuntimeMiss_0"};
    static const char* kClosest[] = {"RuntimeClosestHit_0"};
    ProgramDescription description = {kIntersection, 1, kMiss, 1, kClosest, 1, nullptr, 0};
    NativeProgram program = {};
    if (!createProgram(
            device,
            sourcePath(directory, @"procedural-hit-filter"),
            description,
            program))
        return false;

    MetalRayTracingScene scene = {};
    NSString* sceneError = nil;
    if (!buildMetalBoundingBoxScene(device, queue, scene, &sceneError))
        return fail(sceneError);

    static const uint32_t kRecords[] = {1, 0};
    id<MTLBuffer> records = createRecords(device, kRecords, 2);
    id<MTLBuffer> programResources = createProgramResourceBuffer(device, program, records);
    static const uint32_t kExpected[] = {
        3, 9, 2, 5, 2, 0, 0, 5, 3, 7, 1, 5, 3, 9, 1, 5, 2, 0, 0, 5,
    };
    id<MTLBuffer> results = [device newBufferWithLength:sizeof(kExpected)
                                                options:MTLResourceStorageModeShared];
    if (!dispatch(
            device,
            queue,
            program,
            scene.instanceAccelerationStructure,
            programResources,
            records,
            results,
            5,
            false,
            false))
        return false;
    return validateResults(
        "procedural-hit-filter",
        results,
        kExpected,
        sizeof(kExpected) / sizeof(kExpected[0]));
}

bool runCurveHitFilter(id<MTLDevice> device, id<MTLCommandQueue> queue, NSString* directory)
{
    static const char* kIntersection[] = {"HitGroup_candidate_0"};
    static const char* kMiss[] = {"RuntimeMiss_0"};
    static const char* kClosest[] = {"RuntimeClosestHit_0"};
    ProgramDescription description = {kIntersection, 1, kMiss, 1, kClosest, 1, nullptr, 0};
    NativeProgram program = {};
    if (!createProgram(device, sourcePath(directory, @"curve-hit-filter"), description, program))
        return false;

    MetalRayTracingScene scene = {};
    NSString* sceneError = nil;
    if (!buildMetalCurveScene(device, queue, scene, &sceneError))
        return fail(sceneError);

    static const uint32_t kRecords[] = {1, 0};
    id<MTLBuffer> records = createRecords(device, kRecords, 2);
    id<MTLBuffer> programResources = createProgramResourceBuffer(device, program, records);
    static const uint32_t kExpected[] = {1, 1, 1, 2, 0, 0};
    id<MTLBuffer> results = [device newBufferWithLength:sizeof(kExpected)
                                                options:MTLResourceStorageModeShared];
    if (!dispatch(
            device,
            queue,
            program,
            scene.instanceAccelerationStructure,
            programResources,
            records,
            results,
            2,
            false,
            false))
        return false;
    return validateResults(
        "curve-hit-filter",
        results,
        kExpected,
        sizeof(kExpected) / sizeof(kExpected[0]));
}

bool runMultilevelHit(id<MTLDevice> device, id<MTLCommandQueue> queue, NSString* directory)
{
    static const char* kMiss[] = {"RuntimeMiss_0"};
    static const char* kClosest[] = {"RuntimeClosestHit_0"};
    ProgramDescription description = {nullptr, 0, kMiss, 1, kClosest, 1, nullptr, 0};
    NativeProgram program = {};
    if (!createProgram(device, sourcePath(directory, @"multilevel-hit"), description, program))
        return false;

    MetalRayTracingScene scene = {};
    NSString* sceneError = nil;
    if (!buildMetalMultilevelScene(device, queue, scene, &sceneError))
        return fail(sceneError);

    static const uint32_t kRecords[] = {1, 0, 0};
    id<MTLBuffer> records = createRecords(device, kRecords, 3);
    id<MTLBuffer> programResources = createProgramResourceBuffer(device, program, records);
    static const uint32_t kExpected[] = {1, 2};
    id<MTLBuffer> results = [device newBufferWithLength:sizeof(kExpected)
                                                options:MTLResourceStorageModeShared];
    if (!dispatch(
            device,
            queue,
            program,
            scene.instanceAccelerationStructure,
            programResources,
            records,
            results,
            2,
            false,
            false))
        return false;
    return validateResults("multilevel-hit", results, kExpected, 2);
}

} // namespace

bool runMetalStructuralRayTracingTests(const char* metalSourceDirectory)
{
    @autoreleasepool
    {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (!device)
            return fail(@"no Metal device is available");
        if (![device supportsRaytracing])
            return fail(@"the Metal device does not support ray tracing");
        if (device.argumentBuffersSupport != MTLArgumentBuffersTier2)
            return fail(@"the Metal device does not support tier-2 argument buffers");

        id<MTLCommandQueue> queue = [device newCommandQueue];
        if (!queue)
            return fail(@"failed to create a Metal command queue");

        NSString* directory = [NSString stringWithUTF8String:metalSourceDirectory];
        return runTriangleHitMiss(device, queue, directory) &&
               runProceduralHitFilter(device, queue, directory) &&
               runCallableRecord(device, queue, directory) &&
               runRecursiveTrace(device, queue, directory) &&
               runMultipleSlots(device, queue, directory) &&
               runTriangleAttributesFlags(device, queue, directory) &&
               runStageInputState(device, queue, directory) &&
               runCurveHitFilter(device, queue, directory) &&
               runMultilevelHit(device, queue, directory);
    }
}
