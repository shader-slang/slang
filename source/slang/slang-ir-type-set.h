// slang-ir-type-set.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"

#include "../core/slang-string-slice-pool.h"

namespace Slang
{

/*
NOTE! This type set is only designed to work for emitting code to determine unique types. It is envisaged in the
future that it will not be needed because types will be made unique within a module, and thus the pointer to a type
will uniquely identify the type.

The other reason this type exists, is to allow an IRModule for emit to be immutable. That is not currently possible
within emit code because it may be necessary in order to emit to be able to create other types that needed (for example
vector types required for a matrix type implementation).

This is used so as to try and use slangs type system to uniquely identify types and specializations on intrinsic.
That we want to have a pointer to a type be unique, and slang supports this through the m_sharedIRBuilder. BUT for this to
work all work on the module must use the same sharedIRBuilder, and that appears to not be the case in terms
of other passes.
Even if it was the case when we may want to add types as part of emitting, we can't use the previously used
shared builder, so again we end up with pointers to the same things not being the same thing.

To work around this we clone types we want to use as keys into the 'unique module'.
This is not necessary for all types though - as we assume nominal types *must* have unique pointers (that is the
definition of nominal).

This could be handled in other ways (for example not testing equality on pointer equality). Anyway for now this
works, but probably needs to be handled in a better way. The better way may involve having guarantees about equality
enabled in other code generation and making de-duping possible in emit code.

Note that one pro for this approach is that it does not alter the source module. That as it stands it's not necessary
for the source module to be immutable, because it is created for emitting and then discarded.

NOTE! That Vector<X, 1> or Matrix<X, 1, 1> will be turned into the type X.

 */
class IRTypeSet
{
public:
    enum class Kind
    {
        Scalar,
        Vector,
        Matrix,
        CountOf,
    };

    IRType* add(IRType* type);    
    IRType* addVectorType(IRType* elementType, int colsCount);

    void addAllBuiltinTypes(IRModule* module);

    void addVectorForMatrixTypes();

    void getTypes(List<IRType*>& outTypes) const;
    void getTypes(Kind kind, List<IRType*>& outTypes) const;

    IRType* getType(IRType* type) { return cloneType(type); }

    IRType* cloneType(IRType* type) { return (IRType*)cloneInst((IRInst*)type); }
    IRInst* cloneInst(IRInst* inst);

        /// Returns true if the type belongs and is created on the module owned by the set
    bool isOwned(IRType* type) { return type->getModule() == m_module; }

    IRBuilder& getBuilder() { return m_builder; }
    IRModule* getModule() const { return m_module; }

    void clear();

    IRTypeSet(Session* session);
    ~IRTypeSet();

protected:
    void _addAllBuiltinTypesRec(IRInst* inst);
    void _clearTypes();

        // Maps insts from source modules into m_module.
        // NOTE! That nominal types are not cloned, as they are identified by pointer. They are just 
    Dictionary<IRInst*, IRInst*> m_cloneMap;

        // Can find all types by traversing the types in the m_module
    SharedIRBuilder m_sharedBuilder;    
    IRBuilder m_builder;
    RefPtr<IRModule> m_module;
};

} // namespace Slang
