# Instruction Template Exceptions

This file documents types that have operand mismatches or special implementation requirements that prevent them from being converted to the standard Fiddle template system.

## Types with Implementation Mismatches

### OutParamType and BorrowInOutParamType
**Issue**: These types use `getPtrType()` instead of `getType()` in their original implementations.

**Original implementations**:
```cpp
IROutParamType* IRBuilder::getOutParamType(IRType* valueType)
{
    return (IROutParamType*)getPtrType(kIROp_OutParamType, valueType);
}

IRBorrowInOutParamType* IRBuilder::getBorrowInOutParamType(IRType* valueType)
{
    return (IRBorrowInOutParamType*)getPtrType(kIROp_BorrowInOutParamType, valueType);
}
```

**Template generates**:
```cpp
IROutParamType* IRBuilder::getOutParamType(IRType* valueType)
{
    return (IROutParamType*)getType(kIROp_OutParamType, valueType);
}
```

**Action**: Need to investigate if `getType()` vs `getPtrType()` makes a functional difference, or if template needs enhancement to handle this case.

### WitnessTableType and WitnessTableIDType
**Issue**: These types use `createIntrinsicInst()` instead of `getType()` in their original implementations.

**Original implementations**:
```cpp
IRWitnessTableType* IRBuilder::getWitnessTableType(IRType* baseType)
{
    return (IRWitnessTableType*)
        createIntrinsicInst(nullptr, kIROp_WitnessTableType, 1, (IRInst* const*)&baseType);
}

IRWitnessTableIDType* IRBuilder::getWitnessTableIDType(IRType* baseType)
{
    return (IRWitnessTableIDType*)
        createIntrinsicInst(nullptr, kIROp_WitnessTableIDType, 1, (IRInst* const*)&baseType);
}
```

**Action**: These need special handling and cannot be converted to standard template.

### SPIRVLiteralType
**Issue**: Uses array-style operand passing instead of direct parameter.

**Original implementation**:
```cpp
IRSPIRVLiteralType* IRBuilder::getSPIRVLiteralType(IRType* type)
{
    IRInst* operands[] = {type};
    return (IRSPIRVLiteralType*)getType(kIROp_SPIRVLiteralType, 1, operands);
}
```

**Action**: Could potentially be converted but uses different parameter style.

### Types with Array-Style Operand Passing
**Issue**: These types use `IRInst* operands[]` array pattern instead of direct parameters.

**Examples**:
```cpp
IRResultType* IRBuilder::getResultType(IRType* valueType, IRType* errorType)
{
    IRInst* operands[] = {valueType, errorType};
    return (IRResultType*)getType(kIROp_ResultType, 2, operands);
}

IRConstantBufferType* IRBuilder::getConstantBufferType(IRType* elementType, IRType* layoutType) 
{
    IRInst* operands[] = {elementType, layoutType};
    return (IRConstantBufferType*)getType(kIROp_ConstantBufferType, 2, operands);
}
```

**Affected types**: ResultType, ConstantBufferType, GLSLOutputParameterGroupType, RateQualifiedType, PseudoPtrType

**Action**: Template system would need enhancement to handle array-style operand passing.

### Types with Operand Definition Mismatches
**Issue**: The Lua instruction definition doesn't match the expected method signature.

**Example**:
```cpp
// Expected signature from usage:
IRThisType* getThisType(IRType* interfaceType) 

// But Lua definition has no operands, so template generates:
IRThisType* getThisType() // No parameters
```

**Affected types**: ThisType

**Action**: Verify operand definitions in slang-ir-insts.lua match expected method signatures.

## Notes
- All other types in batches 1-7 successfully converted without issues  
- Template system correctly handles single operand types that use standard `getType(opcode, operand)` pattern
- Types requiring `getPtrType()`, `createIntrinsicInst()`, or special parameter handling need manual implementation