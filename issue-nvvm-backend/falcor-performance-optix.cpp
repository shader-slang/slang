// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Diagnostic-only OptiX table wrapper. No driver files are modified.
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cuda.h>
#include <dlfcn.h>
#include <map>
#include <optix_function_table.h>
#include <string>
static OptixFunctionTable realApi;
static std::map<OptixProgramGroup, std::string> names;
static std::map<OptixPipeline, unsigned> simpleStack;
static bool enabled(const char* name)
{
    const char* v = std::getenv(name);
    return v && std::strcmp(v, "1") == 0;
}
static OptixResult groups(
    OptixDeviceContext context,
    const OptixProgramGroupDesc* desc,
    unsigned count,
    const OptixProgramGroupOptions* options,
    char* log,
    size_t* logSize,
    OptixProgramGroup* out)
{
    auto r = realApi.optixProgramGroupCreate(context, desc, count, options, log, logSize, out);
    if (r == OPTIX_SUCCESS)
        for (unsigned i = 0; i < count; ++i)
        {
            const char* name =
                desc[i].kind == OPTIX_PROGRAM_GROUP_KIND_RAYGEN ? desc[i].raygen.entryFunctionName
                : desc[i].kind == OPTIX_PROGRAM_GROUP_KIND_MISS ? desc[i].miss.entryFunctionName
                : desc[i].kind == OPTIX_PROGRAM_GROUP_KIND_HITGROUP
                    ? desc[i].hitgroup.entryFunctionNameCH
                    : "other";
            bool empty = desc[i].kind == OPTIX_PROGRAM_GROUP_KIND_HITGROUP &&
                         !desc[i].hitgroup.moduleCH && !desc[i].hitgroup.moduleAH &&
                         !desc[i].hitgroup.moduleIS;
            names[out[i]] = name ? name : empty ? "empty" : "unsupported";
        }
    return r;
}
static OptixPipelineCompileOptions pipelineOptions(const OptixPipelineCompileOptions* options)
{
    auto opts = *options;
    if (enabled("PROBE_SINGLE_LEVEL"))
        opts.traversableGraphFlags = OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_SINGLE_LEVEL_INSTANCING;
    if (enabled("PROBE_TRIANGLES_ONLY"))
        opts.usesPrimitiveTypeFlags = OPTIX_PRIMITIVE_TYPE_FLAGS_TRIANGLE;
    return opts;
}
static OptixResult module(
    OptixDeviceContext c,
    const OptixModuleCompileOptions* m,
    const OptixPipelineCompileOptions* p,
    const char* input,
    size_t size,
    char* log,
    size_t* logSize,
    OptixModule* out)
{
    auto opts = pipelineOptions(p);
    return realApi.optixModuleCreate(c, m, &opts, input, size, log, logSize, out);
}
static OptixResult moduleTasks(
    OptixDeviceContext c,
    const OptixModuleCompileOptions* m,
    const OptixPipelineCompileOptions* p,
    const char* input,
    size_t size,
    char* log,
    size_t* logSize,
    OptixModule* out,
    OptixTask* task)
{
    auto opts = pipelineOptions(p);
    return realApi.optixModuleCreateWithTasks(c, m, &opts, input, size, log, logSize, out, task);
}
static OptixResult pipeline(
    OptixDeviceContext ctx,
    const OptixPipelineCompileOptions* options,
    const OptixPipelineLinkOptions* link,
    const OptixProgramGroup* groups,
    unsigned count,
    char* log,
    size_t* logSize,
    OptixPipeline* out)
{
    auto opts = pipelineOptions(options);
    auto r = realApi.optixPipelineCreate(ctx, &opts, link, groups, count, log, logSize, out);
    if (r != OPTIX_SUCCESS)
        return r;
    unsigned rg = 0, ch = 0, ms1 = 0, ms2 = 0, ng = 0, nch = 0, nms = 0;
    bool simple = true;
    for (unsigned i = 0; i < count; ++i)
    {
        OptixStackSizes s = {};
        r = realApi.optixProgramGroupGetStackSize(groups[i], &s, *out);
        if (r != OPTIX_SUCCESS)
            return r;
        const auto& name = names[groups[i]];
        std::fprintf(
            stderr,
            "PROBE_STACK %s RG=%u CH=%u MS=%u AH=%u IS=%u CC=%u DC=%u\n",
            name.c_str(),
            s.cssRG,
            s.cssCH,
            s.cssMS,
            s.cssAH,
            s.cssIS,
            s.cssCC,
            s.dssDC);
        simple &= !s.cssAH && !s.cssIS && !s.cssCC && !s.dssDC;
        if (name.find("__raygen__") == 0)
        {
            rg = std::max(rg, s.cssRG);
            ++ng;
        }
        else if (name.find("scatter_triangle_closest_hit") != std::string::npos)
        {
            ch = std::max(ch, s.cssCH);
            ++nch;
        }
        else if (name.find("scatter_miss") != std::string::npos)
        {
            ms1 = std::max(ms1, s.cssMS);
            ++nms;
        }
        else if (name.find("visibility_miss") != std::string::npos)
        {
            ms2 = std::max(ms2, s.cssMS);
            ++nms;
        }
        else if (name != "empty")
            simple = false;
    }
    std::fprintf(
        stderr,
        "PROBE_PIPELINE payload=%u recursion=%u graph=%u primitives=%u groups=%u\n",
        opts.numPayloadValues,
        link->maxTraceDepth,
        opts.traversableGraphFlags,
        opts.usesPrimitiveTypeFlags,
        count);
    if (enabled("PROBE_SIMPLE_STACK"))
    {
        if (!simple || ng != 1 || nch != 1 || nms != 2 || link->maxTraceDepth != 2)
        {
            std::fprintf(stderr, "PROBE_REJECT unsupported stack graph\n");
            return OPTIX_ERROR_INVALID_VALUE;
        }
        unsigned bytes = rg + std::max(ms1, ch + ms2);
        std::fprintf(stderr, "PROBE_SIMPLE_STACK_REQUIREMENT continuation=%u\n", bytes);
        simpleStack[*out] = bytes;
    }
    return r;
}
static OptixResult setStack(OptixPipeline p, unsigned a, unsigned b, unsigned c, unsigned d)
{
    unsigned changed = c, depth = d;
    if (enabled("PROBE_SIMPLE_STACK"))
    {
        if (!simpleStack.count(p))
            return OPTIX_ERROR_INVALID_VALUE;
        changed = simpleStack[p];
    }
    if (enabled("PROBE_GRAPH_DEPTH2"))
        depth = 2;
    std::fprintf(
        stderr,
        "PROBE_STACK_SETTING traversal=%u state=%u continuation=%u->%u graphDepth=%u->%u\n",
        a,
        b,
        c,
        changed,
        d,
        depth);
    return realApi.optixPipelineSetStackSize(p, a, b, changed, depth);
}
static OptixAccelBuildOptions buildOptions(
    const OptixAccelBuildOptions* options,
    const OptixBuildInput* inputs,
    unsigned count)
{
    auto result = *options;
    bool triangles = count > 0;
    for (unsigned i = 0; i < count; ++i)
        triangles &= inputs[i].type == OPTIX_BUILD_INPUT_TYPE_TRIANGLES;
    if (triangles && enabled("PROBE_FAST_TRACE"))
        result.buildFlags = (result.buildFlags & ~OPTIX_BUILD_FLAG_PREFER_FAST_BUILD) |
                            OPTIX_BUILD_FLAG_PREFER_FAST_TRACE;
    return result;
}
static OptixResult memory(
    OptixDeviceContext c,
    const OptixAccelBuildOptions* o,
    const OptixBuildInput* i,
    unsigned n,
    OptixAccelBufferSizes* s)
{
    auto opts = buildOptions(o, i, n);
    return realApi.optixAccelComputeMemoryUsage(c, &opts, i, n, s);
}
static OptixResult build(
    OptixDeviceContext c,
    CUstream stream,
    const OptixAccelBuildOptions* o,
    const OptixBuildInput* i,
    unsigned n,
    CUdeviceptr temp,
    size_t tempSize,
    CUdeviceptr output,
    size_t outputSize,
    OptixTraversableHandle* handle,
    const OptixAccelEmitDesc* emit,
    unsigned emitCount)
{
    auto opts = buildOptions(o, i, n);
    std::fprintf(
        stderr,
        "PROBE_BUILD type=%u flags=%u->%u inputs=%u\n",
        n ? unsigned(i[0].type) : 0,
        o->buildFlags,
        opts.buildFlags,
        n);
    return realApi.optixAccelBuild(
        c,
        stream,
        &opts,
        i,
        n,
        temp,
        tempSize,
        output,
        outputSize,
        handle,
        emit,
        emitCount);
}
extern "C" OptixResult optixQueryFunctionTable(
    int abi,
    unsigned n,
    OptixQueryFunctionTableOptions* keys,
    const void** values,
    void* table,
    size_t size)
{
    const char* path = std::getenv("NVVM_PERF_OPTIX_LIBRARY");
    if (!path || path[0] != '/')
    {
        std::fprintf(
            stderr,
            "Set NVVM_PERF_OPTIX_LIBRARY to the absolute real driver library path.\n");
        return OPTIX_ERROR_LIBRARY_NOT_FOUND;
    }
    static void* library = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!library)
    {
        std::fprintf(stderr, "PROBE_LOAD_ERROR %s\n", dlerror());
        return OPTIX_ERROR_LIBRARY_NOT_FOUND;
    }
    auto query =
        reinterpret_cast<OptixQueryFunctionTable_t*>(dlsym(library, "optixQueryFunctionTable"));
    if (!query)
        return OPTIX_ERROR_ENTRY_SYMBOL_NOT_FOUND;
    auto r = query(abi, n, keys, values, table, size);
    if (r != OPTIX_SUCCESS)
        return r;
    if (abi != OPTIX_ABI_VERSION || size != sizeof(OptixFunctionTable))
    {
        std::fprintf(stderr, "PROBE_ABI_FORWARD %d %zu\n", abi, size);
        return r;
    }
    realApi = *static_cast<OptixFunctionTable*>(table);
    auto& t = *static_cast<OptixFunctionTable*>(table);
    t.optixModuleCreate = module;
    t.optixModuleCreateWithTasks = moduleTasks;
    t.optixPipelineSetStackSize = setStack;
    t.optixProgramGroupCreate = groups;
    t.optixPipelineCreate = pipeline;
    t.optixAccelComputeMemoryUsage = memory;
    t.optixAccelBuild = build;
    std::fprintf(
        stderr,
        "PROBE_ACTIVE stack=%d fastTrace=%d triangles=%d\n",
        enabled("PROBE_SIMPLE_STACK"),
        enabled("PROBE_FAST_TRACE"),
        enabled("PROBE_TRIANGLES_ONLY"));
    return r;
}
