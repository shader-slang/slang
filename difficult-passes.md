# Difficult Passes for SLANG_PASS Wrapper

This file tracks passes that don't fit the simple `passFunc(irModule)` pattern and may need special handling or wrapper function enhancements.

## Passes with Complex Signatures

### Multiple Parameters Beyond IRModule
- `lowerGLSLShaderStorageBufferObjectsToStructuredBuffers(irModule, sink)` - takes DiagnosticSink
- `translateGlobalVaryingVar(codeGenContext, irModule)` - takes CodeGenContext first
- `bindExistentialSlots(irModule, sink)` - takes DiagnosticSink
- `collectGlobalUniformParameters(irModule, outLinkedIR.globalScopeVarLayout, target)` - takes layout and target
- `checkEntryPointDecorations(irModule, target, sink)` - takes target and sink
- `addDenormalModeDecorations(irModule, codeGenContext)` - takes CodeGenContext
- `collectEntryPointUniformParams(irModule, outLinkedIR.entryPointLayouts[0]->parametersLayout)` - takes layout

### Conditional Execution
Many passes are wrapped in conditional statements based on `requiredLoweringPassSet` flags or target checks.

### Functions That Return Values
Some functions may return values that need to be handled.

## Enhancement Ideas
- Template wrapper function could be enhanced to accept additional parameters
- Could create specialized wrapper macros for common patterns (sink, codeGenContext, target)
- May need lambda wrappers for complex cases