// slang-ir-typeflow-collection.h
#pragma once
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

IRCollectionTagType* makeTagType(IRCollectionBase* collection);

//
// Count and indexing helpers
//

UCount getCollectionCount(IRCollectionBase* collection);
UCount getCollectionCount(IRCollectionTaggedUnionType* taggedUnion);
UCount getCollectionCount(IRCollectionTagType* tagType);

IRInst* getCollectionElement(IRCollectionBase* collection, UInt index);
IRInst* getCollectionElement(IRCollectionTagType* collectionTagType, UInt index);

//
// Helpers to iterate over elements of a collection.
//

template<typename F>
void forEachInCollection(IRCollectionBase* info, F func)
{
    for (UInt i = 0; i < info->getOperandCount(); ++i)
        func(info->getOperand(i));
}

template<typename F>
void forEachInCollection(IRCollectionTagType* tagType, F func)
{
    forEachInCollection(as<IRCollectionBase>(tagType->getCollection()), func);
}

// Upcast the value in 'arg' to match the destInfo type. This method inserts
// any necessary reinterprets or tag translation instructions.
//
IRInst* upcastCollection(IRBuilder* builder, IRInst* arg, IRType* destInfo);

// Builder class that helps greatly with constructing `CollectionBase` instructions,
// which conceptually represent sets, and maintain the property that the equal sets
// should always be represented by the same instruction.
//
// Uses a unique ID assignment to keep stable ordering throughout the lifetime of the
// module.
//
struct CollectionBuilder
{
    // Get a collection builder for 'module'.
    CollectionBuilder(IRModule* module);

    // Create an inst to represent the elements in the set.
    //
    // All insts in `elements` must be global and concrete. They must not
    // be collections themselves.
    //
    // Op must be one of the ops in `CollectionBase`
    //
    // For a given set, the returned inst is always the same within a single
    // module.
    //
    IRCollectionBase* createCollection(IROp op, const HashSet<IRInst*>& elements);

    // Get a suitable collection op-code to use for an set containing 'inst'.
    IROp getCollectionTypeForInst(IRInst* inst);

    // Create a collection with a single element
    IRCollectionBase* makeSingletonSet(IRInst* value);

    // Create a collection with the given elements (the collection op will be
    // automatically deduced using` getCollectionTypeForInst`)
    //
    IRCollectionBase* makeSet(const HashSet<IRInst*>& values);

    // Return a unique ID for the inst. Assuming the module pointer
    // is consistent, this should always be the same for a given inst.
    //
    UInt getUniqueID(IRInst* inst);

private:
    // Reference to parent module
    IRModule* module;

    // Unique ID assignment for functions and witness tables.
    //
    // This is a pointer to a shared dictionary (typically
    // a part of the module inst) so that all CollectionBuilder
    // objects for the same module will always produce the same
    // ordering.
    //
    Dictionary<IRInst*, UInt>* uniqueIds;
};

} // namespace Slang
