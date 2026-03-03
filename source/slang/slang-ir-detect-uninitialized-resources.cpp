// slang-ir-detect-uninitialized-resources.cpp
//
// This pass detects attempts to default-initialize resource types or structs containing
// resource types using empty initializers ({}) or undefined values. This pattern is not
// supported by downstream compilers like DXC.
//
// The pass detects empty makeStruct or undefined values being passed to constructors
// where the parameter type contains resources.

#include "slang-ir-detect-uninitialized-resources.h"

#include "slang-compiler.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

struct UninitializedResourceDetectionContext
{
    IRModule* module;
    DiagnosticSink* sink;

    // Cache of types we've already checked
    InstHashSet structsWithResources;
    Dictionary<IRType*, bool> typeContainsResourceCache;

    UninitializedResourceDetectionContext(IRModule* module, DiagnosticSink* sink)
        : module(module), sink(sink), structsWithResources(module)
    {
    }

    // Check if a type is or contains a resource type
    bool containsResourceType(IRType* type)
    {
        // Check cache first
        if (auto cached = typeContainsResourceCache.tryGetValue(type))
            return *cached;

        bool result = false;

        // Direct resource type
        if (as<IRResourceType>(type))
        {
            result = true;
        }
        // Check struct types for resource fields
        else if (auto structType = as<IRStructType>(type))
        {
            for (auto field : structType->getFields())
            {
                if (containsResourceType(field->getFieldType()))
                {
                    result = true;
                    break;
                }
            }
        }
        // Check array types
        else if (auto arrayType = as<IRArrayType>(type))
        {
            result = containsResourceType(arrayType->getElementType());
        }
        // Check vector types
        else if (auto vectorType = as<IRVectorType>(type))
        {
            result = containsResourceType(vectorType->getElementType());
        }
        // Check matrix types
        else if (auto matrixType = as<IRMatrixType>(type))
        {
            result = containsResourceType(matrixType->getElementType());
        }

        // Cache the result
        typeContainsResourceCache[type] = result;
        return result;
    }

    // Get a human-readable name for a resource type
    String getResourceTypeName(IRType* type)
    {
        if (as<IRTextureType>(type))
            return "texture";
        if (as<IRSamplerStateType>(type))
            return "sampler";
        if (as<IRHLSLStructuredBufferTypeBase>(type))
            return "buffer";

        return "resource";
    }

    // Get struct name if available
    String getStructName(IRStructType* structType)
    {
        if (auto name = structType->findDecoration<IRNameHintDecoration>())
            return String(name->getName());
        return "<unnamed>";
    }

    // Check if an instruction is a problematic uninitialized value
    bool isUninitializedResourceValue(IRInst* inst)
    {
        // Check for empty makeStruct
        if (auto makeStruct = as<IRMakeStruct>(inst))
        {
            if (makeStruct->getOperandCount() == 0 &&
                containsResourceType(makeStruct->getDataType()))
            {
                return true;
            }
        }

        // Check for undefined values
        if (as<IRUndefined>(inst))
        {
            if (containsResourceType(inst->getDataType()))
            {
                return true;
            }
        }

        return false;
    }

    // Check if an instruction is a constructor call
    bool isConstructorCall(IRCall* call)
    {
        if (auto callee = call->getCallee())
        {
            return callee->findDecoration<IRConstructorDecoration>() != nullptr;
        }
        return false;
    }

    void processInst(IRInst* inst)
    {
        // We're only interested in constructor calls
        if (auto call = as<IRCall>(inst))
        {
            if (!isConstructorCall(call))
                return;

            // Check each argument
            UInt argCount = call->getArgCount();
            for (UInt i = 0; i < argCount; i++)
            {
                auto arg = call->getArg(i);
                if (isUninitializedResourceValue(arg))
                {
                    auto sourceLoc = call->sourceLoc;
                    auto uninitializedType = arg->getDataType();

                    // Get the type being constructed
                    IRType* constructedType = nullptr;
                    if (auto callee = call->getCallee())
                    {
                        if (auto funcType = as<IRFuncType>(callee->getDataType()))
                        {
                            constructedType = funcType->getResultType();
                        }
                    }

                    // If we're constructing a struct and passing an uninitialized resource
                    if (constructedType && as<IRStructType>(constructedType))
                    {
                        auto structType = as<IRStructType>(constructedType);
                        String structName = getStructName(structType);

                        if (as<IRResourceType>(uninitializedType))
                        {
                            String resourceName = getResourceTypeName(uninitializedType);

                            // Main error
                            sink->diagnose(
                                sourceLoc,
                                Diagnostics::cannotDefaultInitializeStructWithUninitializedResource,
                                structName,
                                resourceName);

                            // Note pointing to struct definition
                            if (structType->sourceLoc.isValid())
                            {
                                sink->diagnose(
                                    structType->sourceLoc,
                                    Diagnostics::seeDefinitionOfStruct,
                                    structName);
                            }
                        }
                        else
                        {
                            // Struct contains nested resources
                            sink->diagnose(
                                sourceLoc,
                                Diagnostics::cannotDefaultInitializeStructContainingResources,
                                structName);
                        }
                    }
                    else
                    {
                        // Direct resource initialization
                        String resourceName = getResourceTypeName(uninitializedType);
                        sink->diagnose(
                            sourceLoc,
                            Diagnostics::cannotDefaultInitializeResource,
                            resourceName);
                    }
                }
            }
        }
    }

    void processModule()
    {
        // Simple recursive traversal of the entire module
        processInstRec(module->getModuleInst());
    }

    void processInstRec(IRInst* inst)
    {
        processInst(inst);

        for (auto child : inst->getChildren())
        {
            processInstRec(child);
        }
    }
};

void detectUninitializedResources(IRModule* module, DiagnosticSink* sink)
{
    UninitializedResourceDetectionContext context(module, sink);
    context.processModule();
}

} // namespace Slang
