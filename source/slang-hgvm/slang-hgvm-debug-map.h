#pragma once

#include "core/slang-basic.h"
#include "slang-hgbc-container.h"
#include "slang-hgvm-types.h"

namespace Slang::HGVM
{
class Program;

struct TypeMetadata
{
    HGBCId id = 0;
    HGBCOpcode opcode = HGBCOpcode::VoidType;
    String qualifiedName;
    String moduleName;
};

struct SourcePosition
{
    String path;
    String functionName;
    uint32_t line = 0;
    uint32_t column = 0;
    uint32_t endLine = 0;
    uint32_t endColumn = 0;
};

struct SourceLocation : SourcePosition
{
    /// Lists inline call sites from the innermost caller outward.
    List<SourcePosition> inlineSites;
};

struct LambdaLocation
{
    String moduleName;
    String path;
    uint32_t line = 0;
    uint32_t column = 0;
};

/// Describes a static scope state. Runtime callbacks pass only its index in the debug map.
struct DebugScopeInfo
{
    HGBCId scope = 0;
    HGBCId inlinedAt = 0;
    String functionName;
    List<SourcePosition> inlineSites;
};

struct DebugFunctionRange
{
    uint32_t entry = 0;
    uint32_t end = 0;
    String name;
    HGBCId id = 0;
};

/// Copies the canonical host layout computed by staging, including heap field offsets.
struct DebugTypeLayout
{
    struct Field
    {
        String name;
        HGBCId type = 0;
        uint32_t offset = 0;
    };
    TypeKind kind = TypeKind::Scalar;
    HeapObjectKind heapKind = HeapObjectKind::StructuredBuffer;
    NumericLayout numeric;
    HGBCId valueType = 0;
    HGBCId elementType = 0;
    uint32_t byteSize = 0;
    uint32_t elementStride = 0;
    uint32_t elementCount = 0;
    uint32_t allocationSize = 0;
    List<Field> fields;
    List<uint32_t> objectManagedOffsets;
};

struct DebugVariableInfo
{
    HGBCId id = 0;
    HGBCId type = 0;
    HGBCId scope = 0;
    HGBCId inlinedAt = 0;
    String name;
    HGBCId function = 0;
    uint32_t declaration = 0;
    uint32_t argumentIndexPlusOne = 0;
    uint32_t frameOffset = 0;
};

struct DebugInlineSite
{
    HGBCId scope = 0;
    HGBCId parent = 0;
};

/// Owns immutable descriptions independently of the program's executable and device resources.
/// IDs and staged word offsets are meaningful only within this debug map's program ID.
class DebugMap : public RefObject
{
public:
    struct SourceRange
    {
        uint32_t startWordOffset;
        SourceLocation location;
    };

    const List<SourceRange>& getSourceRanges() const { return m_sourceRanges; }
    const List<DebugFunctionRange>& getFunctions() const { return m_functions; }
    const List<uint32_t>& getBlockStarts() const { return m_blockStarts; }
    const DebugScopeInfo& getScope(uint32_t index) const { return m_scopes[index]; }
    const List<DebugVariableInfo>& getVariables() const { return m_variables; }
    const DebugTypeLayout* findTypeLayout(HGBCId id) const { return m_layouts.tryGetValue(id); }
    const DebugInlineSite* findInlineSite(HGBCId id) const { return m_inlineSites.tryGetValue(id); }
    HGBCId getParentScope(HGBCId id) const;
    Index findSourceRange(uint32_t offset) const;
    uint64_t getProgramId() const { return m_programId; }
    const String& getProgramName() const { return m_programName; }
    /// Returns semantic module names so displays can omit only module qualification.
    const List<String>& getModuleNames() const { return m_moduleNames; }
    const TypeMetadata* findType(HGBCId typeId) const;
    /// Finds the containing source range, or null for an address without line information.
    const SourceLocation* findSourceLocation(uint32_t recordWordOffset) const;
    /// Returns the explicitly registered event type at this exact record address, or zero.
    HGBCId findRecordType(uint32_t recordWordOffset) const;
    /// Returns the specialized entry-point name for a staged pipeline descriptor.
    const String* findPipelineName(uint32_t descriptorWordOffset) const;
    /// Returns a shader's semantic name before native pipeline assembly renames its export.
    const String* findEntryPointName(uint32_t descriptorWordOffset) const;
    /// Returns a definition only when the name identifies one location in the requested module.
    const LambdaLocation* findLambdaLocation(const String& name, const String& moduleName = {})
        const;
    const Dictionary<String, List<LambdaLocation>>& getLambdaLocations() const
    {
        return m_lambdaLocations;
    }

private:
    friend class DebugMapBuilder;
    DebugMap();
    uint64_t m_programId;
    String m_programName;
    List<String> m_moduleNames;
    Dictionary<HGBCId, TypeMetadata> m_types;
    Dictionary<uint32_t, HGBCId> m_recordTypes;
    Dictionary<uint32_t, String> m_pipelineNames;
    Dictionary<uint32_t, String> m_entryPointNames;
    Dictionary<String, List<LambdaLocation>> m_lambdaLocations;
    List<SourceRange> m_sourceRanges;
    List<DebugScopeInfo> m_scopes;
    List<DebugFunctionRange> m_functions;
    List<uint32_t> m_blockStarts;
    List<DebugVariableInfo> m_variables;
    Dictionary<HGBCId, DebugTypeLayout> m_layouts;
    Dictionary<HGBCId, HGBCId> m_parentScopes;
    Dictionary<HGBCId, DebugInlineSite> m_inlineSites;
};

/// Translates optional HGBC debug records while the stager emits native records.
/// Malformed optional debug entries produce unavailable locations, never execution failures.
class DebugMapBuilder
{
public:
    explicit DebugMapBuilder(Program* program);
    DebugMap* getDebugMap() const { return m_debugMap; }
    void beginFunction(HGBCId functionId, uint32_t wordOffset);
    void endFunction(uint32_t entry, uint32_t end);
    void beginBlock(uint32_t wordOffset);
    uint32_t getScopeIndex();
    void addVariable(const HGBC::Record& record, uint32_t frameOffset, uint32_t wordOffset);
    void setTypeLayout(HGBCId id, DebugTypeLayout layout);
    /// Starts a source range at the next staged address when a debug directive changes location.
    void processDebugRecord(const HGBC::Record& record, uint32_t wordOffset);
    /// Ends source attribution at function cleanup or another generated region.
    void clearSourceLocation(uint32_t wordOffset);
    /// Associates a type with one event-producing record, independently of source ranges.
    void setRecordType(uint32_t wordOffset, HGBCId typeId);
    /// Names a validated dispatcher or graphics pipeline at its staged descriptor address.
    void setPipelineName(uint32_t wordOffset, const HGBC::Record& record);
    /// Names an entry point, including specialization arguments, at its staged address.
    void setEntryPointName(uint32_t wordOffset, HGBCId id);

private:
    const HGBC::Record* findDebugRecord(HGBCId id, HGBCOpcode opcode, size_t minOperands) const;
    String getString(HGBCId id) const;
    String getSourcePath(HGBCId id) const;
    String getScopeFunctionName(HGBCId id) const;
    String getEntryPointName(HGBCId id) const;
    void updateLocation(uint32_t wordOffset);

    Program* m_sourceProgram;
    RefPtr<DebugMap> m_debugMap;
    SourceLocation m_location;
    HGBCId m_inlineSite = 0;
    HGBCId m_scope = 0;
    String m_functionName;
    HGBCId m_function = 0;
    Dictionary<uint64_t, uint32_t> m_scopeIndices;
};
} // namespace Slang::HGVM
