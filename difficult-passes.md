# Difficult Passes for SLANG_PASS Wrapper

This file tracks passes that don't fit the simple `passFunc(irModule)` pattern and may need special handling or wrapper function enhancements.

## Passes with Complex Signatures

### Multiple Parameters Beyond IRModule (RESOLVED with variadic template)
- ✅ `lowerGLSLShaderStorageBufferObjectsToStructuredBuffers(irModule, sink)` - WRAPPED
- `translateGlobalVaryingVar(codeGenContext, irModule)` - takes CodeGenContext first (parameter order different)
- ✅ `bindExistentialSlots(irModule, sink)` - WRAPPED
- ✅ `collectGlobalUniformParameters(irModule, outLinkedIR.globalScopeVarLayout, target)` - WRAPPED
- ✅ `checkEntryPointDecorations(irModule, target, sink)` - WRAPPED
- ✅ `addDenormalModeDecorations(irModule, codeGenContext)` - WRAPPED
- `collectEntryPointUniformParams(irModule, outLinkedIR.entryPointLayouts[0]->parametersLayout)` - takes layout

### Conditional Execution
Many passes are wrapped in conditional statements based on `requiredLoweringPassSet` flags or target checks.

### Functions That Return Values
Some functions may return values that need to be handled.

### Functions with Optional Parameters (RESOLVED by passing explicit parameter)
- ✅ `performMandatoryEarlyInlining(irModule, HashSet<IRInst*>* modifiedFuncs = nullptr)` - WRAPPED by passing nullptr

### Function Overloads
- `resolveVaryingInputRef` - has overloads for both IRModule* and IRFunc*, compiler can't deduce template parameter
- `performForceInlining` - has overloads for IRModule* and IRGlobalValueWithCode*, compiler can't deduce template parameter

## Enhancement Ideas
- Template wrapper function could be enhanced to accept additional parameters
- Could create specialized wrapper macros for common patterns (sink, codeGenContext, target)
- May need lambda wrappers for complex cases