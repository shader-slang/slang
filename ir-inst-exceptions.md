# IR Instruction get*Type() Functions Analysis

This document tracks the analysis of `get.*Type()` functions in slang-ir-insts.h to determine which ones can be converted to use auto-generation.

## Functions Found with Variadic Parameters

### Can Convert (Suitable for Auto-Generation)
These functions have variadic arguments and can potentially use the auto-generation system:

1. **IRTargetTupleType* getTargetTupleType(UInt count, IRType* const* types)** - Line 3184
   - Status: CONVERT - Has simple variadic pattern with types array
   - IR Definition: `{ TargetTuple = { struct_name = "TargetTupleType", hoistable = true } }` - MISSING OPERANDS!
   - Action: SKIP - No operands defined in .lua file, can't auto-generate

2. **IRFuncType* getFuncType(UInt paramCount, IRType* const* paramTypes, IRType* resultType)** - Line 3307
   - Status: CONVERT - Standard variadic pattern
   - IR Definition: `{ Func = { struct_name = "FuncType", hoistable = true } }` - MISSING OPERANDS!
   - Action: SKIP - No operands defined in .lua file, can't auto-generate

3. **IRType* getConjunctionType(UInt typeCount, IRType* const* types)** - Line 3333 
   - Status: CONVERT - Simple variadic types array
   - IR Definition: `{ Conjunction = { struct_name = "ConjunctionType", hoistable = true } }` - MISSING OPERANDS!
   - Action: SKIP - No operands defined in .lua file, can't auto-generate

4. **IRAssociatedType* getAssociatedType(ArrayView<IRInterfaceType*> constraintTypes)** - Line 3180
   - Status: CONVERT - ArrayView can be converted to variadic operands
   - IR Definition: `{ associated_type = { hoistable = true } }` - MISSING OPERANDS!
   - Action: SKIP - No operands defined in .lua file, can't auto-generate

### Need Investigation
These functions may or may not be suitable:

5. **IRType* getBindExistentialsType(IRInst* baseType, UInt slotArgCount, IRInst* const* slotArgs)** - Line 3323/3325
   - Status: INVESTIGATE - Mixed operand types, need to check IR definition
   - IR Definition: `{ BindExistentials = { struct_name = "BindExistentialsType", operands = { { "baseType", "IRType" } } } }` - MISSING VARIADIC!
   - Action: SKIP - Would need to add variadic operands to IR definition
   - Has overload with IRUse const* slotArgs

6. **IRType* getAttributedType(IRType* baseType, UInt attributeCount, IRAttr* const* attributes)** - Line 3341/3343
   - Status: INVESTIGATE - Uses IRAttr* instead of IRInst*, may not fit pattern
   - IR Definition: `{ Attributed = { struct_name = "AttributedType", operands = { { "baseType", "IRType" }, { "attr" } }, hoistable = true } }` - MISSING VARIADIC!
   - Action: SKIP - Uses IRAttr* not IRInst*, and would need variadic operand definitions
   - Has List<IRAttr*> overload

### Cannot Convert (Skip)
These functions should not be converted:

7. **IRType* getType(IROp op, UInt operandCount, IRInst* const* operands)** - Line 3777
   - Status: SKIP - Generic function used by auto-generation system itself

8. **IRFuncType* getFuncType(List<IRType*> const& paramTypes, IRType* resultType)** - Line 3315
   - Status: SKIP - Convenience overload, likely calls the count/array version

9. **IRType* getAttributedType(IRType* baseType, List<IRAttr*> attributes)** - Line 3343
   - Status: SKIP - Convenience overload

## Functions with Multiple Fixed Parameters
These might be candidates for variadic conversion:

10. **IRTupleType* getTupleType(IRType* type0, IRType* type1)** - Line 3187
11. **IRTupleType* getTupleType(IRType* type0, IRType* type1, IRType* type2)** - Line 3188  
12. **IRTupleType* getTupleType(IRType* type0, IRType* type1, IRType* type2, IRType* type3)** - Line 3189
    - Status: ALREADY CONVERTED - TupleType was already added to basic_types list

## Analysis Results

**Summary: NO FUNCTIONS CAN BE CONVERTED TO AUTO-GENERATION**

All the functions with variadic parameters that I found either:
1. Have no operands defined in their IR instruction definition (.lua file) 
2. Have incomplete operand definitions (missing variadic operands)
3. Use non-IRInst* types (like IRAttr*)

**Key Findings:**
- The existing auto-generation system works well for types that already have proper variadic operand definitions in slang-ir-insts.lua (like TupleType which was already converted)
- Functions like `getTargetTupleType`, `getFuncType`, `getConjunctionType`, etc. would need their IR instruction definitions updated in slang-ir-insts.lua to include variadic operand definitions before they could use auto-generation
- This would be a more significant change that affects the IR instruction schema

**Recommendation:**
Since all the candidate functions would require changes to the canonical IR instruction definitions in slang-ir-insts.lua, and this affects the IR instruction schema which may have downstream compatibility concerns, I recommend leaving these functions as manually implemented for now.

The auto-generation system is working correctly for types that already have proper variadic operand definitions (like TupleType). The templates we improved can handle any future types that are added with proper variadic operand definitions.