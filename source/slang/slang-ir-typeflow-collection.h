// slang-ir-typeflow-collection.h
#pragma once
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

IRCollectionTagType* makeTagType(IRCollectionBase* collection);

UCount getCollectionCount(IRCollectionBase* collection);
UCount getCollectionCount(IRCollectionTaggedUnionType* taggedUnion);
UCount getCollectionCount(IRCollectionTagType* tagType);

IRInst* getCollectionElement(IRCollectionBase* collection, UInt index);
IRInst* getCollectionElement(IRCollectionTagType* collectionTagType, UInt index);

// Helper to iterate over collection elements

template<typename F>
void forEachInCollection(IRCollectionBase* info, F func)
{
    for (UInt i = 0; i < info->getOperandCount(); ++i)
        func(info->getOperand(i));
}

template<typename F>
void forEachInCollection(IRCollectionTagType* tagType, F func)
{
    forEachInCollection(as<IRCollectionBase>(tagType->getOperand(0)), func);
}

struct CollectionBuilder
{
    CollectionBuilder(IRModule* module);

    UInt getUniqueID(IRInst* inst);

    // Helper methods for creating canonical collections
    IRCollectionBase* createCollection(IROp op, const HashSet<IRInst*>& elements);
    IROp getCollectionTypeForInst(IRInst* inst);
    IRCollectionBase* makeSingletonSet(IRInst* value);
    IRCollectionBase* makeSet(const HashSet<IRInst*>& values);

private:
    // Reference to parent module
    IRModule* module;

    // Unique ID assignment for functions and witness tables
    Dictionary<IRInst*, UInt>* uniqueIds;
};

} // namespace Slang
