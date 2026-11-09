#include "slang-hgvm-debug-map.h"

#include "slang-hgvm.h"

#include <atomic>

namespace Slang::HGVM
{
DebugMap::DebugMap()
{
    static std::atomic<uint64_t> nextId{1};
    m_programId = nextId.fetch_add(1, std::memory_order_relaxed);
}

const TypeMetadata* DebugMap::findType(HGBCId typeId) const
{
    return m_types.tryGetValue(typeId);
}

HGBCId DebugMap::getParentScope(HGBCId id) const
{
    auto parent = m_parentScopes.tryGetValue(id);
    return parent ? *parent : 0;
}

void DebugMapBuilder::setTypeLayout(HGBCId id, DebugTypeLayout layout)
{
    m_debugMap->m_layouts.add(id, std::move(layout));
}

void DebugMapBuilder::addVariable(
    const HGBC::Record& record,
    uint32_t frameOffset,
    uint32_t wordOffset)
{
    const auto& operands = record.operands;
    DebugVariableInfo variable;
    variable.id = operands[0];
    variable.name = getString(operands[1]);
    variable.type = operands[2];
    variable.scope = operands[6];
    variable.inlinedAt = m_inlineSite;
    variable.function = m_function;
    variable.declaration = wordOffset;
    variable.argumentIndexPlusOne = operands.size() > 7 ? operands[7] : 0;
    variable.frameOffset = frameOffset;
    m_debugMap->m_variables.add(std::move(variable));
}

const SourceLocation* DebugMap::findSourceLocation(uint32_t offset) const
{
    Index index = findSourceRange(offset);
    if (index < 0)
        return nullptr;
    const auto& location = m_sourceRanges[index].location;
    return location.line ? &location : nullptr;
}

Index DebugMap::findSourceRange(uint32_t offset) const
{
    // Find the first range starting after the address. The preceding range owns the address.
    Index first = 0, end = m_sourceRanges.getCount();
    while (first < end)
    {
        Index middle = first + (end - first) / 2;
        if (m_sourceRanges[middle].startWordOffset <= offset)
            first = middle + 1;
        else
            end = middle;
    }
    return first - 1;
}

HGBCId DebugMap::findRecordType(uint32_t offset) const
{
    auto type = m_recordTypes.tryGetValue(offset);
    return type ? *type : 0;
}

const String* DebugMap::findPipelineName(uint32_t descriptorWordOffset) const
{
    return m_pipelineNames.tryGetValue(descriptorWordOffset);
}

const String* DebugMap::findEntryPointName(uint32_t descriptorWordOffset) const
{
    return m_entryPointNames.tryGetValue(descriptorWordOffset);
}

const LambdaLocation* DebugMap::findLambdaLocation(const String& name, const String& moduleName)
    const
{
    auto locations = m_lambdaLocations.tryGetValue(name);
    if (!locations)
        return nullptr;
    const LambdaLocation* result = nullptr;
    for (const auto& location : *locations)
    {
        if (moduleName.getLength() && location.moduleName != moduleName)
            continue;
        if (result && (result->path != location.path || result->line != location.line ||
                       result->column != location.column))
            return nullptr;
        result = &location;
    }
    return result;
}

/// Uses the semantic module, entry point, and argument strings already serialized by HGBC.
/// For example, a specialization of `fill` becomes `kernels::fill<float, 4>`.
String DebugMapBuilder::getEntryPointName(HGBCId id) const
{
    const auto& records = m_sourceProgram->getDecodedProgram().records;
    const auto& index = m_sourceProgram->getIndex().resultRecords;
    const auto& record = records[index.at(id)];
    StringBuilder name;
    if (record.opcode == HGBCOpcode::SpecializeSlangEntryPoint)
    {
        name << getEntryPointName(record.operands[1]) << '<';
        for (size_t i = 2; i < record.operands.size(); i += 2)
        {
            if (i != 2)
                name << ", ";
            name << getString(record.operands[i + 1]);
        }
        name << '>';
    }
    else
    {
        SLANG_RELEASE_ASSERT(record.opcode == HGBCOpcode::SlangEntryPoint);
        const auto& module = records[index.at(record.operands[1])];
        SLANG_RELEASE_ASSERT(module.opcode == HGBCOpcode::SlangModule);
        name << getString(module.operands[1]) << "::" << getString(record.operands[2]);
    }
    return name.produceString();
}

void DebugMapBuilder::setPipelineName(uint32_t wordOffset, const HGBC::Record& record)
{
    String name;
    if (record.opcode == HGBCOpcode::MakeDispatcher)
        name = getEntryPointName(record.operands[3]);
    else
    {
        SLANG_RELEASE_ASSERT(record.opcode == HGBCOpcode::MakeGraphicsPipeline);
        name =
            getEntryPointName(record.operands[2]) + " + " + getEntryPointName(record.operands[3]);
    }
    m_debugMap->m_pipelineNames.add(wordOffset, std::move(name));
}

void DebugMapBuilder::setEntryPointName(uint32_t wordOffset, HGBCId id)
{
    m_debugMap->m_entryPointNames.add(wordOffset, getEntryPointName(id));
}

DebugMapBuilder::DebugMapBuilder(Program* program)
    : m_sourceProgram(program), m_debugMap(new DebugMap)
{
    m_debugMap->m_scopes.add({});
    m_scopeIndices.add(0, 0);
    m_debugMap->m_programName = program->getName().c_str();
    for (const auto& record : program->getDecodedProgram().records)
    {
        if (record.opcode == HGBCOpcode::SlangModule)
        {
            auto name = getString(record.operands[1]);
            if (!m_debugMap->m_moduleNames.contains(name))
                m_debugMap->m_moduleNames.add(name);
        }
        if (record.opcode == HGBCOpcode::DebugLexicalBlock && record.operands.size() >= 2)
            m_debugMap->m_parentScopes.add(record.operands[0], record.operands[1]);
        if (record.opcode == HGBCOpcode::DebugInlinedAt && record.operands.size() >= 6)
            m_debugMap->m_inlineSites.add(
                record.operands[0],
                {record.operands[1], record.operands[5]});
        if (record.opcode == HGBCOpcode::DebugLambda && record.operands.size() >= 6)
        {
            auto name = getString(record.operands[1]);
            LambdaLocation location;
            location.moduleName = getString(record.operands[2]);
            location.path = getSourcePath(record.operands[3]);
            location.line = record.operands[4];
            location.column = record.operands[5];
            if (name.getLength() && location.path.getLength() && location.line)
                m_debugMap->m_lambdaLocations.getOrAddValue(name, {}).add(std::move(location));
            continue;
        }
        if (record.debugInfo.section != HGBCSectionTag::TYPE)
            continue;
        HGBCId id = HGBC::getResultId(record);
        if (!id)
            continue;
        TypeMetadata type;
        type.id = id;
        type.opcode = record.opcode;
        if (auto name = program->getSlangTypeName(id))
            type.qualifiedName = name->c_str();
        else if (auto name = program->findStringDecoration(id, HGBCDecorationKind::NameHint))
            type.qualifiedName = name->c_str();
        if (auto module = program->findStringDecoration(id, HGBCDecorationKind::ModuleName))
            type.moduleName = module->c_str();
        if (type.moduleName.getLength() && !m_debugMap->m_moduleNames.contains(type.moduleName))
            m_debugMap->m_moduleNames.add(type.moduleName);
        m_debugMap->m_types.add(id, std::move(type));
    }
}

const HGBC::Record* DebugMapBuilder::findDebugRecord(
    HGBCId id,
    HGBCOpcode opcode,
    size_t minOperands) const
{
    auto it = m_sourceProgram->getIndex().resultRecords.find(id);
    if (it == m_sourceProgram->getIndex().resultRecords.end())
        return nullptr;
    const auto& record = m_sourceProgram->getDecodedProgram().records[it->second];
    return record.opcode == opcode && record.operands.size() >= minOperands ? &record : nullptr;
}

String DebugMapBuilder::getString(HGBCId id) const
{
    auto text = m_sourceProgram->findString(id);
    return text ? String(text->c_str()) : String();
}

String DebugMapBuilder::getSourcePath(HGBCId id) const
{
    auto source = findDebugRecord(id, HGBCOpcode::DebugSource, 3);
    return source ? getString(source->operands[1]) : String();
}

String DebugMapBuilder::getScopeFunctionName(HGBCId id) const
{
    HashSet<HGBCId> visited;
    while (id && visited.add(id))
    {
        if (auto function = findDebugRecord(id, HGBCOpcode::DebugFunction, 5))
            return getString(function->operands[1]);
        auto scope = findDebugRecord(id, HGBCOpcode::DebugLexicalBlock, 7);
        if (!scope)
            break;
        id = scope->operands[1];
    }
    return String();
}

void DebugMapBuilder::beginFunction(HGBCId functionId, uint32_t wordOffset)
{
    m_function = functionId;
    clearSourceLocation(wordOffset);
    if (auto name = m_sourceProgram->findStringDecoration(functionId, HGBCDecorationKind::NameHint))
        m_location.functionName = name->c_str();
    m_functionName = m_location.functionName;
}

void DebugMapBuilder::endFunction(uint32_t entry, uint32_t end)
{
    m_debugMap->m_functions.add({entry, end, m_functionName, m_function});
}

void DebugMapBuilder::beginBlock(uint32_t wordOffset)
{
    m_debugMap->m_blockStarts.add(wordOffset);
}

uint32_t DebugMapBuilder::getScopeIndex()
{
    uint64_t key = (uint64_t(m_scope) << 32) | m_inlineSite;
    if (auto index = m_scopeIndices.tryGetValue(key))
        return *index;
    uint32_t index = uint32_t(m_debugMap->m_scopes.getCount());
    m_debugMap->m_scopes.add(
        {m_scope, m_inlineSite, m_location.functionName, m_location.inlineSites});
    m_scopeIndices.add(key, index);
    return index;
}

void DebugMapBuilder::updateLocation(uint32_t wordOffset)
{
    m_location.inlineSites.clear();
    HashSet<HGBCId> visited;
    for (HGBCId id = m_inlineSite; id && visited.add(id);)
    {
        auto site = findDebugRecord(id, HGBCOpcode::DebugInlinedAt, 6);
        if (!site)
            break;
        SourcePosition position;
        position.functionName = getScopeFunctionName(site->operands[1]);
        position.path = getSourcePath(site->operands[2]);
        position.line = position.endLine = site->operands[3];
        position.column = position.endColumn = site->operands[4];
        m_location.inlineSites.add(std::move(position));
        id = site->operands[5];
    }
    SourceLocation location =
        m_location.path.getLength() && m_location.line ? m_location : SourceLocation{};
    auto& ranges = m_debugMap->m_sourceRanges;
    if (ranges.getCount())
    {
        auto& last = ranges.getLast();
        SLANG_RELEASE_ASSERT(wordOffset >= last.startWordOffset);
        // Multiple directives before an emitted instruction describe the same range.
        if (last.startWordOffset == wordOffset)
        {
            last.location = std::move(location);
            return;
        }
        if (!last.location.line && !location.line)
            return;
    }
    else if (!location.line)
        return;
    ranges.add({wordOffset, std::move(location)});
}

void DebugMapBuilder::processDebugRecord(const HGBC::Record& record, uint32_t wordOffset)
{
    const auto& operands = record.operands;
    switch (record.opcode)
    {
    case HGBCOpcode::DebugLine:
        m_location.path = operands.size() >= 5 ? getSourcePath(operands[0]) : String();
        if (operands.size() >= 5)
        {
            m_location.line = operands[1];
            m_location.endLine = operands[2];
            m_location.column = operands[3];
            m_location.endColumn = operands[4];
        }
        updateLocation(wordOffset);
        return;
    case HGBCOpcode::DebugScope:
        m_scope = operands.empty() ? 0 : operands[0];
        m_location.functionName = operands.empty() ? String() : getScopeFunctionName(operands[0]);
        m_inlineSite = operands.size() >= 2 ? operands[1] : 0;
        updateLocation(wordOffset);
        return;
    case HGBCOpcode::DebugNoScope:
        clearSourceLocation(wordOffset);
        return;
    default:
        break;
    }
}

void DebugMapBuilder::clearSourceLocation(uint32_t wordOffset)
{
    m_location = {};
    m_inlineSite = 0;
    m_scope = 0;
    updateLocation(wordOffset);
}

void DebugMapBuilder::setRecordType(uint32_t wordOffset, HGBCId typeId)
{
    SLANG_RELEASE_ASSERT(typeId && m_debugMap->findType(typeId));
    m_debugMap->m_recordTypes.add(wordOffset, typeId);
}
} // namespace Slang::HGVM
