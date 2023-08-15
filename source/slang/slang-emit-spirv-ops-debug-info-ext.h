#ifdef SLANG_IN_SPIRV_EMIT_CONTEXT

// https://github.com/KhronosGroup/SPIRV-Registry/blob/main/nonsemantic/NonSemantic.Shader.DebugInfo.100.asciidoc#DebugCompilationUnit
template<typename T>
SpvInst* emitOpDebugCompilationUnit(SpvInstParent* parent, IRInst* inst, const T& idResultType, SpvInst* set, SpvInst* version, SpvInst* dwarfVersion, SpvInst* source, SpvInst* language)
{
    static_assert(isSingular<T>);
    return emitInst(parent, inst, SpvOpExtInst, idResultType, kResultID, set, SpvWord(1), version, dwarfVersion, source, language);
}

// https://github.com/KhronosGroup/SPIRV-Registry/blob/main/nonsemantic/NonSemantic.Shader.DebugInfo.100.asciidoc#DebugSource
template<typename T>
SpvInst* emitOpDebugSource(SpvInstParent* parent, IRInst* inst, const T& idResultType, SpvInst* set, IRInst* file, IRInst* text)
{
    static_assert(isSingular<T>);
    return emitInst(parent, inst, SpvOpExtInst, idResultType, kResultID, set, SpvWord(35), file, text);
}

// https://github.com/KhronosGroup/SPIRV-Registry/blob/main/nonsemantic/NonSemantic.Shader.DebugInfo.100.asciidoc#DebugLine
template<typename T>
SpvInst* emitOpDebugLine(SpvInstParent* parent, IRInst* inst, const T& idResultType, SpvInst* set, IRInst* source, IRInst* lineStart, IRInst* lineEnd, IRInst* colStart, IRInst* colEnd)
{
    static_assert(isSingular<T>);
    return emitInst(parent, inst, SpvOpExtInst, idResultType, kResultID, set, SpvWord(103), source, lineStart, lineEnd, colStart, colEnd);
}


#endif        // SLANG_IN_SPIRV_EMIT_CONTEXT
