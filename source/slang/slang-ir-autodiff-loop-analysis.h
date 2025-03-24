// slang-ir-autodiff-loop-analysis.h
#pragma once

#include "slang-ir-autodiff-region.h"
#include "slang-ir-autodiff.h"
#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{
struct SimpleRelation
{
    enum Type
    {
        Any,             // Target can be anything (all values are possible)
        IntegerRelation, // Target satisfies a simple integer equality/inequality
        BoolRelation,    // Target satisfies boolean equality
        Impossible       // Target is impossible (has no possible values)
    } type;

    enum Comparator
    {
        LessThan,
        GreaterThan,
        LessThanEqual,
        GreaterThanEqual,
        Equal,
        NotEqual
    } comparator;
    IRIntegerValue integerValue;
    bool boolValue;

    static SimpleRelation integerRelation(Comparator comparator, IRIntegerValue integerValue)
    {
        return SimpleRelation{IntegerRelation, comparator, integerValue, false};
    }

    static SimpleRelation boolRelation(bool boolValue)
    {
        return SimpleRelation{BoolRelation, Equal, 0, boolValue};
    }

    static SimpleRelation impossibleRelation()
    {
        return SimpleRelation{Impossible, Equal, 0, false};
    }

    static SimpleRelation anyRelation() { return SimpleRelation{Any, Equal, 0, false}; }

    bool operator==(const SimpleRelation& other) const
    {
        switch (type)
        {
        case Any:
            return other.type == Any;
        case IntegerRelation:
            return other.type == IntegerRelation && comparator == other.comparator &&
                   integerValue == other.integerValue;
        case BoolRelation:
            return other.type == BoolRelation && boolValue == other.boolValue;
        case Impossible:
            return other.type == Impossible;
        default:
            SLANG_UNREACHABLE("Unhandled relation type");
        }
    }

    bool operator!=(const SimpleRelation& other) const { return !(*this == other); }

    SimpleRelation negated() const
    {
        switch (type)
        {
        case Any:
            return SimpleRelation{Impossible, Equal, 0, false};
        case Impossible:
            return SimpleRelation{Any, Equal, 0, false};
        case BoolRelation:
            return SimpleRelation{BoolRelation, Equal, 0, !boolValue};
        case IntegerRelation:
            switch (comparator)
            {
            case LessThan:
                return SimpleRelation{IntegerRelation, GreaterThanEqual, integerValue, false};
            case GreaterThan:
                return SimpleRelation{IntegerRelation, LessThanEqual, integerValue, false};
            case LessThanEqual:
                return SimpleRelation{IntegerRelation, GreaterThan, integerValue, false};
            case GreaterThanEqual:
                return SimpleRelation{IntegerRelation, LessThan, integerValue, false};
            case Equal:
                return SimpleRelation{IntegerRelation, NotEqual, integerValue, false};
            case NotEqual:
                return SimpleRelation{IntegerRelation, Equal, integerValue, false};
            default:
                SLANG_UNREACHABLE("Unhandled comparator");
            }
        default:
            SLANG_UNREACHABLE("Unhandled relation type");
        }
    }

    HashCode64 getHashCode() const
    {
        HashCode64 code = Slang::getHashCode(int(type));
        switch (type)
        {
        case IntegerRelation:
            code = combineHash(code, Slang::getHashCode(comparator));
            code = combineHash(code, Slang::getHashCode(integerValue));
            break;
        case BoolRelation:
            code = combineHash(code, Slang::getHashCode(boolValue));
            break;
        case Impossible:
        case Any:
            break;
        default:
            SLANG_UNREACHABLE("Unhandled relation type");
        }
        return code;
    }
};

// A statement that a relation holds on an inst.
struct Statement
{
    enum Type
    {
        Concrete, // A concrete relation about an inst.
        Variable, // An unknown statement about an inst in a block. (A placeholder)
        Empty     // Says nothing about anything.
    } type;

    IRInst* inst;
    SimpleRelation relation;

    // Only needed if Type == Variable.
    IRBlock* block;

    static Statement variable(IRInst* inst, IRBlock* block)
    {
        return Statement(Variable, inst, SimpleRelation::anyRelation(), block);
    }

    static Statement empty()
    {
        return Statement(Empty, nullptr, SimpleRelation::anyRelation(), nullptr);
    }

    static Statement concrete(IRInst* inst, SimpleRelation relation)
    {
        return Statement(Concrete, inst, relation, nullptr);
    }

    // A simple statement that's always true for everything.
    // Use as predicate for collecting unconditional statements
    //
    static Statement trivial()
    {
        return Statement::concrete(nullptr, SimpleRelation::anyRelation());
    }

    // Transfer a statement to a new instruction, because this statement's
    // inst and '_inst' are referring to the same thing.
    //
    Statement toInst(IRInst* _inst)
    {
        switch (type)
        {
        case Concrete:
            return Statement(Concrete, _inst, relation, nullptr);
        case Empty:
            return Statement::empty();
        case Variable:
            // We won't translate a statement variable to the new instruction.
            return *this;
        default:
            SLANG_UNREACHABLE("Unhandled statement type");
        }
    }

    Statement()
        : type(Empty), inst(nullptr), relation(SimpleRelation::anyRelation()), block(nullptr)
    {
    }

    bool operator==(const Statement& other) const
    {
        switch (type)
        {
        case Concrete:
            return other.type == Concrete && inst == other.inst && relation == other.relation;
        case Variable:
            return other.type == Variable && inst == other.inst && block == other.block;
        case Empty:
            return other.type == Empty;
        default:
            SLANG_UNREACHABLE("Unhandled statement type");
        }
    }

    HashCode64 getHashCode() const
    {
        HashCode64 code = Slang::getHashCode(inst);
        code = combineHash(code, Slang::getHashCode(int(type)));

        switch (type)
        {
        case Concrete:
            code = combineHash(code, Slang::getHashCode(relation));
            break;
        case Variable:
            code = combineHash(code, Slang::getHashCode(block));
            break;
        case Empty:
            break;
        default:
            SLANG_UNREACHABLE("Unhandled statement type");
        }

        return code;
    }

private:
    Statement(Type type, IRInst* inst, SimpleRelation relation, IRBlock* block)
        : type(type), inst(inst), relation(relation), block(block)
    {
    }
};

struct StatementCacheKey
{
    IRBlock* block;
    IRInst* inst;
    Statement predicate;

    StatementCacheKey(IRBlock* block, IRInst* inst, Statement predicate)
        : block(block), inst(inst), predicate(predicate)
    {
    }

    StatementCacheKey()
        : block(nullptr), inst(nullptr), predicate(Statement::trivial())
    {
    }

    bool operator==(const StatementCacheKey& other) const
    {
        return block == other.block && inst == other.inst && predicate == other.predicate;
    }

    HashCode64 getHashCode() const
    {
        HashCode64 code = Slang::getHashCode(block);
        code = combineHash(code, Slang::getHashCode(inst));
        code = combineHash(code, predicate.getHashCode());
        return code;
    }
};

// Utility functions.
bool isIntegerConstantValue(IRInst* inst);
bool isBoolConstantValue(IRInst* inst);
IRIntegerValue getConstantIntegerValue(IRInst* inst);
bool getConstantBoolValue(IRInst* inst);

bool doesRelationImply(SimpleRelation relationA, SimpleRelation relationB);

// Try to collect a statement such that predicate => statement, statement.block == block and
// statement.inst == param.
//
// This is a "best effort" process, so we want to return as tight a statement as possible,
// but shouldn't return anything incorrect.
//
Statement tryCollectPredicatedStatement(
    Dictionary<StatementCacheKey, Statement>& cache,
    IRBlock* block,
    IRInst* inst,
    Statement predicate);

} // namespace Slang
