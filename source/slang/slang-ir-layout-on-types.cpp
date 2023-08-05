#include "slang-ir-layout-on-types.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

struct TypeTypeLayout
{
    IRType* type;
    IRTypeLayout* layout;
};

template<typename Struct, typename StructuredBuffer, typename Array, typename Base>
static void zipPreorderTypeAndTypeLayout(
    Struct struct_,
    StructuredBuffer structuredBuffer,
    Array array,
    Base base,
    IRType* type,
    IRTypeLayout* layout)
{
    auto go = [&](auto& go, IRType* type, IRLayout* layout) -> void{
        if(const auto structTypeLayout = as<IRStructTypeLayout>(layout))
        {
            const auto structType = as<IRStructType>(type);
            SLANG_ASSERT(structType);
            struct_(structType, structTypeLayout);

            Index i = 0;
            for(const auto field : structType->getFields())
            {
                const auto fieldVarLayout = structTypeLayout->getFieldLayout(i);
                const auto fieldTypeLayout = fieldVarLayout->getTypeLayout();
                const auto fieldType = field->getFieldType();
                go(go, fieldType, fieldTypeLayout);
                ++i;
            }
        }
        else if(const auto structuredBufferTypeLayout = as<IRStructuredBufferTypeLayout>(layout))
        {
            const auto structuredBufferType = as<IRHLSLStructuredBufferTypeBase>(type);
            SLANG_ASSERT(structuredBufferType);
            structuredBuffer(structuredBufferType, structuredBufferTypeLayout);

            go(go, structuredBufferType->getElementType(), structuredBufferTypeLayout->getElementTypeLayout());
        }
        else if(const auto arrayTypeLayout = as<IRArrayTypeLayout>(layout))
        {
            const auto arrayType = as<IRArrayTypeBase>(type);
            SLANG_ASSERT(arrayType);
            array(arrayType , arrayTypeLayout);

            go(go, arrayType->getElementType(), arrayTypeLayout->getElementTypeLayout());
        }
        else
        {
            base((IRType*)type, (IRTypeLayout*)layout);
        }
    };
    go(go, type, layout);
}

static Dictionary<IRType*, IRTypeLayout*> associateTypesWithLayouts(IRModule* module)
{
    List<TypeTypeLayout> worklist;
    for(auto globalInst : module->getGlobalInsts())
    {
       if(const auto varLayout = as<IRVarLayout>(globalInst))
       {
           auto typeLayout = varLayout->getTypeLayout();
           List<IRInst*> vars;
           traverseUsers(varLayout, [&](IRInst* varLayoutUser){
                if(const auto dec = as<IRLayoutDecoration>(varLayoutUser))
                {
                    if(const auto globalParam = as<IRGlobalParam>(dec->getParent()))
                        vars.add(globalParam);
                    else
                        ; // todo
                }
                else if(const auto dec = as<IREntryPointLayout>(varLayoutUser))
                    ; // todo
                else if(const auto dec = as<IRStructFieldLayoutAttr>(varLayoutUser))
                    ; // todo
                else
                    SLANG_UNEXPECTED("Var layout was used somewhere unexpected");
           });
           if(vars.getCount() == 1)
           {
               auto type = vars[0]->getDataType();
               worklist.add({type, typeLayout});
           }
           else if(vars.getCount() > 2)
           {
               SLANG_UNIMPLEMENTED_X("vars with different layouts");
           }
       }
    }

    Dictionary<IRType*, IRTypeLayout*> ret;
    while(!worklist.getCount() == 0)
    {
        const auto ttl = worklist.getLast();
        worklist.removeLast();
        const auto add = [&](IRType* type, IRTypeLayout* typeLayout){
            const auto* otherTypeLayout = ret.tryGetValueOrAdd(type, typeLayout);
            if(otherTypeLayout)
                SLANG_ASSERT(*otherTypeLayout == typeLayout);
        };
        zipPreorderTypeAndTypeLayout(add, add, add, add, ttl.type, ttl.layout);
    }

    return ret;
}

static void decorateTypesWithLayouts(IRModule* module, const Dictionary<IRType*, IRTypeLayout*>& assocs)
{
    IRBuilder builder(module);
    for(const auto& [type, layout] : assocs)
    {
       builder.setInsertBefore(type);
       // TODO: types with more than one decoration (needs deduplicating)
       builder.addLayoutDecoration(type, layout);
    }
}

void placeTypeLayoutsOnTypes(IRModule* module, CodeGenContext* codeGenContext)
{
    SLANG_ASSERT(module);
    SLANG_ASSERT(codeGenContext);
    const auto assocs = associateTypesWithLayouts(module);
    decorateTypesWithLayouts(module, assocs);
}
}
