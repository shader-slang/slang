// slang-hgbc-disassembler.h
#pragma once

#include "../core/slang-basic.h"

#include <span>
struct ISlangBlob;
struct ISlangWriter;

namespace Slang
{

class DiagnosticSink;
namespace HGBC
{
struct Program;
}

struct HGBCDisassemblyOptions
{
    bool useFriendlyIds = true;
    bool showOffsets = false;
    bool showBinaryLiteralBytes = false;
};

using HGBCDisassemblySymbolCallback = void (*)(
    void* userData,
    const char* name,
    size_t nameLength,
    const char* sectionName,
    size_t sectionNameLength,
    const char* typeName,
    size_t typeNameLength,
    const char* definition,
    size_t definitionLength);

using HGBCDisassemblyInstructionCallback =
    void (*)(void* userData, const char* instruction, size_t instructionLength);

SLANG_API SlangResult disassembleHGBC(
    ISlangBlob* blob,
    ISlangWriter* writer,
    DiagnosticSink* sink,
    HGBCDisassemblyOptions const& options = HGBCDisassemblyOptions());

/// Writes a structured document for presenting HGBC in a source-code editor.
///
/// Function and block delimiters are rendered as braces so indentation and folding work without
/// requiring an editor to understand HGBC's binary record structure. Symbol strings are borrowed
/// for the duration of the callback; the receiver must copy any strings it retains.
SLANG_API SlangResult writeHGBCDisassemblyDocument(
    ISlangBlob* blob,
    ISlangWriter* writer,
    HGBCDisassemblySymbolCallback symbolCallback,
    void* userData,
    DiagnosticSink* sink,
    HGBCDisassemblyOptions const& options = HGBCDisassemblyOptions());

/// Formats selected records with the same ID naming and instruction syntax as `disassembleHGBC`.
///
/// This entry point expects record indices from the supplied decoded program. It is used by
/// debugger clients that need individual instruction rows rather than a complete textual module.
/// Each formatted instruction is borrowed for the duration of the callback.
SLANG_API void enumerateHGBCRecordDisassembly(
    const HGBC::Program& program,
    std::span<const size_t> recordIndices,
    HGBCDisassemblyInstructionCallback instructionCallback,
    void* userData,
    HGBCDisassemblyOptions const& options = HGBCDisassemblyOptions());

} // namespace Slang
