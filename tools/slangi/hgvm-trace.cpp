// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "hgvm-trace.h"

#include "core/slang-string-escape-util.h"
#include "core/slang-string-util.h"
#include "slang-hgvm/slang-hgvm-debug-map.h"
#include "slang-hgvm/slang-hgvm-interpreter.h"
#include "slang-hgvm/slang-hgvm-resource.h"
#include "slang-hgvm/slang-hgvm-types.h"

namespace SlangITool
{
using namespace Slang;
using namespace Slang::HGVM;

HGVMTraceSink::HGVMTraceSink(ISlangWriter* writer, const EventOutputOptions& options)
    : m_writer(writer)
    , m_remainingEvents(options.traceEventLimit)
    , m_terminal(writer, SLANG_WRITER_CHANNEL_STD_OUTPUT, options.color)
    , m_display(options.fullNames)
{
}

/// Names the managed wrapper type so heap events can be read without decoding internal tags.
static const char* getHeapKindName(HeapObjectKind kind)
{
    switch (kind)
    {
    case HeapObjectKind::StructuredBuffer:
        return "buffer";
    case HeapObjectKind::Texture2D:
        return "texture";
    case HeapObjectKind::String:
        return "string";
    case HeapObjectKind::ByteStream:
        return "byte-stream";
    case HeapObjectKind::DeferredReadback:
        return "deferred-readback";
    case HeapObjectKind::Timer:
        return "timer";
    case HeapObjectKind::Window:
        return "window";
    case HeapObjectKind::Surface:
        return "surface";
    case HeapObjectKind::Class:
        return "class";
    case HeapObjectKind::Box:
        return "box";
    case HeapObjectKind::RuntimeArray:
        return "runtime-array";
    case HeapObjectKind::RaytracingPipeline:
        return "ray-tracing-pipeline";
    case HeapObjectKind::RayAccelerationStructure:
        return "ray-acceleration-structure";
    }
    SLANG_UNEXPECTED("Unknown HGVM heap object kind");
}

/// Keeps only the low 48 address bits for compact trace output.
static unsigned long long getTraceAddress(const void* address)
{
    return static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(address)) &
           0xffffffffffffULL;
}

/// Decodes the pipeline's own descriptor, which may belong to an earlier invocation.
void HGVMTraceSink::appendPipelineName(
    EventText& line,
    const EventTracker* tracker,
    const void* pipeline) const
{
    auto interpreter = tracker->getInterpreter();
    auto origin = tracker->findPipelineOrigin(pipeline);
    if (!interpreter || !origin)
        return;
    auto map = interpreter->getDebugMap(origin->programId);
    if (auto name = map ? map->findPipelineName(origin->recordWordOffset) : nullptr)
    {
        line.field("name", m_display.getDisplayName(map, *name), EventTextStyle::Type);
    }
}

/// Appends an address as a dim value, retaining the existing compact address spelling.
static void appendAddress(EventText& line, const char* name, const void* address)
{
    line << EventTextStyle::Plain << ' ' << name << '=' << EventTextStyle::Dim;
    line.appendFormat("0x%llx", getTraceAddress(address));
}

/// Appends a measured duration in milliseconds using the shared duration style.
static void appendDuration(EventText& line, const char* name, uint64_t nanoseconds)
{
    line << EventTextStyle::Plain << ' ' << name << '=' << EventTextStyle::Duration;
    line.appendFormat("%.3f", double(nanoseconds) / 1e6);
}

void HGVMTraceSink::onEvent(const TrackingEvent& event) noexcept
{
    if (!m_remainingEvents)
        return;
    if (m_remainingEvents != UINT64_MAX)
        --m_remainingEvents;

    using Style = EventTextStyle;
    EventText line;
    line << Style::Dim << "[hgvm] " << Style::Label;
    auto interpreter = event.tracker->getInterpreter();
    switch (event.kind)
    {
    case TrackingEventKind::HeapAlloc:
    case TrackingEventKind::HeapFree:
        line << (event.kind == TrackingEventKind::HeapAlloc ? "heap-alloc" : "heap-free");
        appendAddress(line, "address", event.heap.address);
        line.field("kind", getHeapKindName(event.heap.objectKind), Style::Plain);
        line.field("bytes", event.heap.byteSize);
        break;
    case TrackingEventKind::PayloadAlloc:
    case TrackingEventKind::PayloadFree:
        line << (event.kind == TrackingEventKind::PayloadAlloc ? "payload-alloc" : "payload-free");
        appendAddress(line, "address", event.payload.address);
        if (event.kind == TrackingEventKind::PayloadAlloc)
        {
            line.field("bytes", event.payload.byteSize);
            line.field("alignment", event.payload.alignment);
        }
        break;
    case TrackingEventKind::ModuleLoad:
        {
            line << "module-load";
            appendAddress(line, "module", event.module.module);
            line.field(
                "format",
                event.module.format == TrackedModuleFormat::IR ? "ir" : "source",
                Style::Plain);
            StringBuilder quoted;
            StringEscapeUtil::appendQuoted(
                StringEscapeUtil::getHandler(StringEscapeUtil::Style::JSON),
                UnownedStringSlice(event.module.name ? event.module.name : ""),
                quoted);
            line.field("name", quoted.getUnownedSlice(), Style::String);
            line.field("bytes", event.module.byteSize);
            appendDuration(line, "duration-ms", event.module.durationNanoseconds);
            break;
        }
    case TrackingEventKind::PipelineCreation:
        line << "pipeline-create";
        appendAddress(line, "pipeline", event.pipeline.pipeline);
        line.field("kind", getTrackedPipelineKindName(event.pipeline.kind), Style::Plain);
        appendPipelineName(line, event.tracker, event.pipeline.pipeline);
        appendDuration(line, "duration-ms", event.pipeline.durationNanoseconds);
        break;
    case TrackingEventKind::BufferCreation:
        line << "buffer-create";
        appendAddress(line, "resource", event.buffer.resource);
        line.field(
            "purpose",
            event.buffer.purpose == TrackedBufferPurpose::Program ? "program" : "readback",
            Style::Plain);
        line.field("usage", getBufferUsageName(event.buffer.usage), Style::Plain);
        line.field("bytes", event.buffer.byteSize);
        line.field("element-size", event.buffer.elementSize);
        appendDuration(line, "duration-ms", event.buffer.durationNanoseconds);
        break;
    case TrackingEventKind::TextureCreation:
        line << "texture-create";
        appendAddress(line, "resource", event.texture.resource);
        line << Style::Plain << " size=" << Style::Number << event.texture.width << 'x'
             << event.texture.height << 'x' << event.texture.depth;
        line.field("format", uint32_t(event.texture.format));
        line.field("mips", event.texture.mipCount);
        line.field("samples", event.texture.sampleCount);
        break;
    case TrackingEventKind::BufferReuse:
    case TrackingEventKind::BufferRecycle:
    case TrackingEventKind::BufferEviction:
        line
            << (event.kind == TrackingEventKind::BufferReuse     ? "buffer-reuse"
                : event.kind == TrackingEventKind::BufferRecycle ? "buffer-recycle"
                                                                 : "buffer-evict");
        appendAddress(line, "resource", event.buffer.resource);
        line.field("bytes", event.buffer.byteSize);
        break;
    case TrackingEventKind::CommandBatchBegin:
    case TrackingEventKind::CommandBatchDiscard:
    case TrackingEventKind::CommandBufferSubmit:
        line
            << (event.kind == TrackingEventKind::CommandBatchBegin     ? "batch-begin"
                : event.kind == TrackingEventKind::CommandBatchDiscard ? "batch-discard"
                                                                       : "command-submit");
        line.field("batch", event.batch.id);
        if (event.kind != TrackingEventKind::CommandBatchBegin)
            line.field("operations", event.batch.operationCount);
        if (event.kind == TrackingEventKind::CommandBufferSubmit)
        {
            appendAddress(line, "command-buffer", event.batch.commandBuffer);
            line.field("fence", event.batch.completionFenceValue);
        }
        break;
    case TrackingEventKind::Draw:
        line << (event.draw.indexed ? "draw-indexed" : "draw");
        line.field("batch", event.draw.batchId);
        line.field("execution", event.draw.executionId);
        appendAddress(line, "pipeline", event.draw.pipeline);
        appendPipelineName(line, event.tracker, event.draw.pipeline);
        line.field("count", event.draw.count);
        line.field("instances", event.draw.instanceCount);
        break;
    case TrackingEventKind::Dispatch:
        line << "dispatch";
        line.field("batch", event.dispatch.batchId);
        line.field("execution", event.dispatch.executionId);
        appendAddress(line, "pipeline", event.dispatch.pipeline);
        appendPipelineName(line, event.tracker, event.dispatch.pipeline);
        line << Style::Plain << " groups=" << Style::Number << event.dispatch.x << 'x'
             << event.dispatch.y << 'x' << event.dispatch.z;
        break;
    case TrackingEventKind::PipelineExecutionComplete:
        line << "pipeline-complete";
        line.field("batch", event.pipelineExecution.batchId);
        line.field("execution", event.pipelineExecution.executionId);
        appendAddress(line, "pipeline", event.pipelineExecution.pipeline);
        appendPipelineName(line, event.tracker, event.pipelineExecution.pipeline);
        if (event.pipelineExecution.hasDuration)
            appendDuration(line, "gpu-duration-ms", event.pipelineExecution.durationNanoseconds);
        break;
    case TrackingEventKind::AccelerationStructureBuild:
    case TrackingEventKind::AccelerationStructureBuildComplete:
        {
            const auto& build = event.accelerationStructureBuild;
            line
                << (event.kind == TrackingEventKind::AccelerationStructureBuild
                        ? "acceleration-structure-build"
                        : "acceleration-structure-build-complete");
            line.field("batch", build.batchId);
            line.field("execution", build.executionId);
            appendAddress(line, "acceleration-structure", build.accelerationStructure);
            line.field(
                "kind",
                build.kind == rhi::AccelerationStructureKind::BottomLevel ? "blas" : "tlas",
                Style::Plain);
            if (build.hasDuration)
                appendDuration(line, "gpu-duration-ms", build.durationNanoseconds);
            break;
        }
    case TrackingEventKind::Blit:
    case TrackingEventKind::Clear:
        line << (event.kind == TrackingEventKind::Blit ? "blit" : "clear");
        line.field("batch", event.textureOperation.batchId);
        appendAddress(line, "destination", event.textureOperation.destination);
        if (event.kind == TrackingEventKind::Blit)
            appendAddress(line, "source", event.textureOperation.source);
        break;
    default:
        SLANG_UNEXPECTED("Unknown HGVM tracking event kind");
    }

    auto allocationMap =
        interpreter ? interpreter->getDebugMap(event.allocationOrigin.programId) : nullptr;
    if ((event.kind == TrackingEventKind::HeapAlloc || event.kind == TrackingEventKind::HeapFree) &&
        event.heap.typeId && allocationMap)
    {
        if (auto type = allocationMap->findType(event.heap.typeId))
        {
            line.field(
                "type",
                m_display.getDisplayName(allocationMap, type->qualifiedName, type->moduleName),
                Style::Type);
            if (m_display.usesFullNames() && type->moduleName.getLength())
                line.field("module", type->moduleName, Style::String);
        }
    }
    line << '\n';
    String rendered = line.render(m_terminal.useColor());
    m_writer->write(rendered.getBuffer(), rendered.getLength());
}
} // namespace SlangITool
