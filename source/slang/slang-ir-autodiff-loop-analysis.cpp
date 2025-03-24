// slang-ir-autodiff-loop-analysis.cpp

#include "slang-ir-autodiff-loop-analysis.h"

namespace Slang
{

static bool isCompareCmpInst(IRInst* inst)
{
    // Switch on the opcode of the instruction
    switch (inst->getOp())
    {
    case kIROp_Less:
    case kIROp_Greater:
    case kIROp_Leq:
    case kIROp_Geq:
    case kIROp_Eql:
    case kIROp_Neq:
        return true;
    default:
        return false;
    }
}

SimpleRelation mergeEqualityWithIntegerRelation(SimpleRelation equality, SimpleRelation relation)
{
    SLANG_ASSERT(
        equality.type == SimpleRelation::IntegerRelation &&
        relation.type == SimpleRelation::IntegerRelation);
    SLANG_ASSERT(equality.comparator == SimpleRelation::Equal);

    switch (relation.comparator)
    {
    case SimpleRelation::Equal:
        if (relation.integerValue == equality.integerValue)
            return SimpleRelation::integerRelation(SimpleRelation::Equal, equality.integerValue);
        else
            return SimpleRelation::anyRelation(); // Technically this is a "set"
                                                  // {equality.integerValue, relation.integerValue};
                                                  // but we don't have a representation for this.
    case SimpleRelation::LessThan:
    case SimpleRelation::LessThanEqual:
        if (relation.integerValue <= equality.integerValue)
            return SimpleRelation::integerRelation(
                SimpleRelation::LessThanEqual,
                equality.integerValue);
        else
            return SimpleRelation::anyRelation();
    case SimpleRelation::GreaterThan:
    case SimpleRelation::GreaterThanEqual:
        if (relation.integerValue >= equality.integerValue)
            return SimpleRelation::integerRelation(
                SimpleRelation::GreaterThanEqual,
                equality.integerValue);
        else
            return SimpleRelation::anyRelation();
    default:
        break;
    }

    return SimpleRelation::anyRelation();
}

SimpleRelation mergeIntervals(SimpleRelation a, SimpleRelation b)
{
    SLANG_ASSERT(
        a.type == SimpleRelation::IntegerRelation && b.type == SimpleRelation::IntegerRelation);

    if (a.comparator == SimpleRelation::Equal)
    {
        return mergeEqualityWithIntegerRelation(a, b);
    }
    else if (b.comparator == SimpleRelation::Equal)
    {
        return mergeEqualityWithIntegerRelation(b, a);
    }

    // TODO: Handle other cases...
    return SimpleRelation::anyRelation();
}

// Returns the tighest "simple" relation we can prove to be true given an input that may be
// "a" OR "b" (disjunction)
//
// Note: "simple" means that the relation is not a disjunction or conjunction of other relations.
//
SimpleRelation relationUnion(SimpleRelation a, SimpleRelation b)
{
    // Base case. The disjunction operator is idempotent.
    if (a == b)
        return a;

    // If either side is trivially true, the result is trivially true.
    if (a.type == SimpleRelation::Any || b.type == SimpleRelation::Any)
        return SimpleRelation::anyRelation();

    // If either side is trivially false, then the result is the other relation.
    if (a.type == SimpleRelation::Impossible)
        return b;

    if (b.type == SimpleRelation::Impossible)
        return a;

    // If one is the negated form of the other, there's really nothing we can prove, since
    // A OR ~A is always true.
    //
    if (a.negated() == b)
        return SimpleRelation::anyRelation();

    // Handle the case of where one is an inequality and the other is an equality.
    if (a.type == SimpleRelation::IntegerRelation && b.type == SimpleRelation::IntegerRelation)
        return mergeIntervals(a, b);

    // TODO: Here's where we can handle subset cases like (a < 10) and (a < 20) => (a < 20), etc..
    // But we don't _have_ to. The more we can prove, the more cases we can handle, but the result
    // is still correct without it.
    //

    // Default to not being able to say anything.
    return SimpleRelation::anyRelation();
}

// Returns the tighest "simple" relation we can prove to be true given an input that is
// "a" AND "b" (conjunction)
//
SimpleRelation relationIntersection(SimpleRelation a, SimpleRelation b)
{
    // Base case. The conjunction operator is idempotent.
    if (a == b)
        return a;

    // If one is the negated form of the other, then we can prove that the result is impossible.
    // Doesn't necessarily mean that we have an error on our hands, but it does mean that whatever
    // case we're considering can't happen, so can be ignored (unreachable)
    //
    if (a.negated() == b)
        return SimpleRelation::impossibleRelation();

    // If any of the relations is impossible, then the result is impossible.
    if (a.type == SimpleRelation::Impossible || b.type == SimpleRelation::Impossible)
        return SimpleRelation::impossibleRelation();

    // If any one of the relations is trivially true, then the result is the other relation.
    if (a.type == SimpleRelation::Any)
        return b;

    if (b.type == SimpleRelation::Any)
        return a;

    //
    // We'll handle the case where one is an equality and the other is an inequality.
    //
    // i.e. if we have (a == 10) and (a < 20), then (a < 20) is still the tighest relation we can
    // prove.
    //
    if (a.type == SimpleRelation::IntegerRelation && b.type == SimpleRelation::IntegerRelation)
    {
        if (a.comparator == SimpleRelation::Equal)
        {
            if (b.comparator == SimpleRelation::LessThan && a.integerValue < b.integerValue)
            {
                return b;
            }
            else if (b.comparator == SimpleRelation::GreaterThan && a.integerValue > b.integerValue)
            {
                return b;
            }
            else if (
                b.comparator == SimpleRelation::LessThanEqual && a.integerValue <= b.integerValue)
            {
                return b;
            }
            else if (
                b.comparator == SimpleRelation::GreaterThanEqual &&
                a.integerValue >= b.integerValue)
            {
                return b;
            }
        }
        else if (b.comparator == SimpleRelation::Equal)
        {
            if (a.comparator == SimpleRelation::LessThan && b.integerValue < a.integerValue)
            {
                return a;
            }
            else if (a.comparator == SimpleRelation::GreaterThan && b.integerValue > a.integerValue)
            {
                return a;
            }
            else if (
                a.comparator == SimpleRelation::LessThanEqual && b.integerValue <= a.integerValue)
            {
                return a;
            }
            else if (
                a.comparator == SimpleRelation::GreaterThanEqual &&
                b.integerValue >= a.integerValue)
            {
                return a;
            }
        }
    }

    // TODO: Here's where we can handle more subset cases like (a < 10) and (a < 20) => (a < 10),
    // etc.. But we don't _have_ to. The more we can prove, the more cases we can handle, but the
    // result is still correct without it.
    //

    return SimpleRelation::anyRelation();
}

// This function answers the question: "Can we prove that relationB is true if relationA is true?"
//
// Note that this is not the same as "Does relationA imply relationB", since there can be cases
// where this is indeed true, but we just don't have the logic to prove it.
//
bool doesRelationImply(SimpleRelation relationA, SimpleRelation relationB)
{
    // Equal relations imply each other
    if (relationA == relationB)
        return true;

    // If B is trivially true, then A implies B
    if (relationB.type == SimpleRelation::Any)
        return true;

    // If A is trivially true, then A implies B only if B is also trivially true
    if (relationA.type == SimpleRelation::Any)
        return (relationB.type == SimpleRelation::Any);

    // If A is impossible, then technically what we return doesn't matter...
    if (relationA.type == SimpleRelation::Impossible ||
        relationB.type == SimpleRelation::Impossible)
        return false;

    // If A is a boolean relation, then A implies B if B is also a boolean relation and the values
    // are the same.
    //
    if (relationA.type == SimpleRelation::BoolRelation)
        return (relationB.type == SimpleRelation::BoolRelation) &&
               (relationA.boolValue == relationB.boolValue);

    if (relationA.type == SimpleRelation::IntegerRelation)
    {
        if (relationB.type != SimpleRelation::IntegerRelation)
            return false;

        // Technically, the equality case is already handled above, so we'll only consider
        // cases where A and B are not the same relation, but where A -> B

        // If A is an equality, and B is an inequality, we can test
        if (relationA.comparator == SimpleRelation::Equal)
        {
            if (relationB.comparator == SimpleRelation::LessThan)
                return relationA.integerValue <= relationB.integerValue;
            else if (relationB.comparator == SimpleRelation::GreaterThan)
                return relationA.integerValue >= relationB.integerValue;
            else if (relationB.comparator == SimpleRelation::LessThanEqual)
                return relationA.integerValue <= relationB.integerValue;
            else if (relationB.comparator == SimpleRelation::GreaterThanEqual)
                return relationA.integerValue >= relationB.integerValue;
        }

        // If A is an equality, and B is an inequality with different values, then
        // A -> B
        //
        if (relationA.comparator == SimpleRelation::Equal &&
            relationB.comparator == SimpleRelation::NotEqual)
        {
            return relationA.integerValue != relationB.integerValue;
        }

        // TODO: Handle other cases.. these come up rarely, so we can
    }

    return false;
}

bool isIntegerConstantValue(IRInst* inst)
{
    return inst->getOp() == kIROp_IntLit;
}

bool isBoolConstantValue(IRInst* inst)
{
    return inst->getOp() == kIROp_BoolLit;
}

IRIntegerValue getConstantIntegerValue(IRInst* inst)
{
    SLANG_ASSERT(isIntegerConstantValue(inst));
    return as<IRIntLit>(inst)->getValue();
}

bool getConstantBoolValue(IRInst* inst)
{
    SLANG_ASSERT(isBoolConstantValue(inst));
    return as<IRBoolLit>(inst)->getValue();
}

static List<Statement> tryExtractStatements(IRTerminatorInst* inst, IRBlock* block)
{
    List<Statement> statements;

    // Lambds to add statements to the list. If there' already something about an inst,
    // we'll need to AND it with the new statement.
    //
    auto addStatement = [&](Statement statement)
    {
        for (auto& existingStatement : statements)
        {
            if (existingStatement.inst == statement.inst)
            {
                existingStatement.relation =
                    relationIntersection(existingStatement.relation, statement.relation);
                return;
            }
        }
        statements.add(statement);
    };

    // From condInst, extract a statement about any inst such that we have an equality
    // statement (integer or boolean) on the inst.
    //
    if (auto ifElse = as<IRIfElse>(inst))
    {
        // Check that the block is the true or false block of the if-else
        bool isTrueBlock = ifElse->getTrueBlock() == block;
        bool isFalseBlock = ifElse->getFalseBlock() == block;
        if (!isTrueBlock && !isFalseBlock)
            goto done;

        auto condInst = inst->getOperand(0);

        if (condInst->getOp() == kIROp_Eql)
        {
            auto leftOperand = condInst->getOperand(0);
            auto rightOperand = condInst->getOperand(1);

            if (isIntegerConstantValue(leftOperand))
            {
                addStatement(Statement::concrete(
                    rightOperand,
                    SimpleRelation::integerRelation(
                        (isTrueBlock ? SimpleRelation::Equal : SimpleRelation::NotEqual),
                        getConstantIntegerValue(leftOperand))));
            }
            else if (isIntegerConstantValue(rightOperand))
            {
                addStatement(Statement::concrete(
                    leftOperand,
                    SimpleRelation::integerRelation(
                        (isTrueBlock ? SimpleRelation::Equal : SimpleRelation::NotEqual),
                        getConstantIntegerValue(rightOperand))));
            }
        }
        else if (isCompareCmpInst(condInst))
        {
            auto leftOperand = condInst->getOperand(0);
            auto rightOperand = condInst->getOperand(1);

            bool isParamLeft = !isIntegerConstantValue(leftOperand);
            bool isParamRight = !isIntegerConstantValue(rightOperand);

            // If neither operand is an inst, we can't say anything.
            if (!isParamLeft && !isParamRight)
                goto done;

            auto paramOperand = isParamLeft ? leftOperand : rightOperand;
            auto otherOperand = isParamLeft ? rightOperand : leftOperand;

            // Check if the "other" operand is a constant
            if (!isIntegerConstantValue(otherOperand))
                goto done;

            auto constantVal = getConstantIntegerValue(otherOperand);

            SimpleRelation::Comparator comparator;
            switch (condInst->getOp())
            {
            case kIROp_Less:
                comparator = SimpleRelation::LessThan;
                break;
            case kIROp_Greater:
                comparator = SimpleRelation::GreaterThan;
                break;
            case kIROp_Leq:
                comparator = SimpleRelation::LessThanEqual;
                break;
            case kIROp_Geq:
                comparator = SimpleRelation::GreaterThanEqual;
                break;
            case kIROp_Eql:
                comparator = SimpleRelation::Equal;
                break;
            case kIROp_Neq:
                comparator = SimpleRelation::NotEqual;
                break;
            default:
                SLANG_UNREACHABLE("unexpected op code");
            }
            auto relation = SimpleRelation::integerRelation(comparator, constantVal);
            addStatement(Statement::concrete(
                paramOperand,
                ((isParamLeft ^ !isTrueBlock) ? relation : relation.negated())));
        }
        else if (auto condParam = as<IRParam>(condInst))
        {
            // We can add a statement about the parameter.
            addStatement(Statement::concrete(condParam, SimpleRelation::boolRelation(isTrueBlock)));
        }
    }
    else if (auto switchInst = as<IRSwitch>(inst))
    {
        // Check that the block is the default case of the switch
        if (switchInst->getDefaultLabel() == block)
            goto done;

        // Check each case block
        UInt caseCount = switchInst->getCaseCount();
        for (UInt i = 0; i < caseCount; i++)
        {
            auto caseValue = switchInst->getCaseValue(i);
            auto caseBlock = switchInst->getCaseLabel(i);

            if (caseBlock == block && isIntegerConstantValue(caseValue))
            {
                auto constantVal = getConstantIntegerValue(caseValue);
                addStatement(Statement::concrete(
                    switchInst->getCondition(),
                    SimpleRelation::integerRelation(SimpleRelation::Equal, constantVal)));
            }
        }
    }

done:
    return statements;
}

Statement statementConjunction(IRBlock* block, Statement a, Statement b)
{
    if (a == b)
        return a;

    if (a.type == Statement::Empty)
        return b;
    if (b.type == Statement::Empty)
        return a;

    SLANG_ASSERT(a.inst == b.inst);

    if (a.type == Statement::Concrete && b.type == Statement::Concrete)
    {
        return Statement::concrete(a.inst, relationIntersection(a.relation, b.relation));
    }

    if (a.type == Statement::Variable && b.type == Statement::Concrete)
    {
        // If we're talking about the same instruction, then we should return the other statement.
        if (a.inst == b.inst)
            return b;
        else
        {
            // Otherwise, we can be a bit more specfic depending on 'b's relation.
            if (b.relation.type == SimpleRelation::Any)
                return a; // The variable might be more specific, so we'll keep the variable.
            else if (b.relation.type == SimpleRelation::Impossible)
                return Statement::concrete(b.inst, SimpleRelation::impossibleRelation());
            else
            {
                // If B says something about the inst, then we'll use that.
                return b;
            }
        }
    }

    // Same as above, but for the other way around.
    if (b.type == Statement::Variable && a.type == Statement::Concrete)
    {
        // If we're talking about the same instruction, then we should return the other statement.
        if (b.inst == a.inst)
            return a;
        else
        {
            // Otherwise, we can be a bit more specfic depending on 'a's relation.
            if (a.relation.type == SimpleRelation::Any)
                return b; // The variable might be more specific, so we'll keep the variable.
            else if (a.relation.type == SimpleRelation::Impossible)
                return Statement::concrete(a.inst, SimpleRelation::impossibleRelation());
            else
            {
                // If A says something about the inst, then we'll use that.
                return a;
            }
        }
    }

    SLANG_UNREACHABLE("Unhandled statement conjunction case");
}

Statement statementDisjunction(IRBlock* block, Statement a, Statement b)
{
    if (a.type == Statement::Empty)
        return b;
    if (b.type == Statement::Empty)
        return a;

    // Here we're taking advantage of the fact that variables can only appear once.
    // x ^ a = x, gives us x = a as a solution.
    // x v a = x, gives us x = a as a solution.
    //
    // What about (x ^ a) v b = x ?
    // Substitute x = a v b, and we get ((a v b) ^ a) v b = ((a ^ a) v (b ^ a)) v b = a v (b ^ a) v
    // b = a v b
    //
    // What about (x v a) ^ b = x ?
    // Substitute x = a ^ b, and we get ((a v b) v a) ^ b = ((a v a) v (b v a)) ^ b = a v (b ^ a) ^
    // b = a v b
    //
    if (a.type == Statement::Variable && b.type == Statement::Concrete)
    {
        // If we're trying to disjunct an unknown relation with a concrete relation
        // _on the same inst_, then the result is the concrete relation.
        //
        if (a.inst == b.inst && a.block == block)
            return b;

        // If we're trying to disjunct an unknown relation with a concrete relation
        // _on different insts_, then the result is simply unknown since we don't
        // have enough information to say anything.
        //
        if (a.inst != b.inst)
            return Statement::concrete(b.inst, SimpleRelation::anyRelation());
    }

    if (b.type == Statement::Variable && a.type == Statement::Concrete)
    {
        if (a.inst == b.inst && b.block == block)
            return a;

        // If we're trying to disjunct an unknown relation with a concrete relation
        // _on different insts_, then the result is simply unknown since we don't
        // have enough information to say anything.
        //
        if (a.inst != b.inst)
            return Statement::concrete(a.inst, SimpleRelation::anyRelation());
    }

    SLANG_ASSERT(a.inst == b.inst);

    if (a.type == Statement::Concrete && b.type == Statement::Concrete)
    {
        return Statement::concrete(a.inst, relationUnion(a.relation, b.relation));
    }

    SLANG_UNREACHABLE("Unhandled statement disjunction case");
}

// NEW APPROACH:

// For each block,
// First, we need to get a statement from our predecessors.
//
// - For each predecessor,
//      - Transfer inst, and all predicate insts based on phi args.
//      - Extract statement from the split block, if there is one.
//      - Add to the predicate set.
//      - Resolve predicate set.
//      - If set resolves to False, ignore branch
//      - Otherwise, recursively query the predecessor with the new predicate set.
//  - Take disjunction of available statements.
//  - Base case: we come back to the loop starting block.
//
// Note: inst is always the same here. Other insts are just being added/removed from the predicate
// set.
//

struct StatementSet
{
    // A conjunction of independent predicates (a1 ^ a2 ^ a3 ...)
    Dictionary<IRInst*, Statement> predicates;

    // Disjunction of a predicate with the current set (pred v (a1 ^ a2 ^ a3 ...))
    void disjunct(Statement predicate)
    {
        if (predicate.type == Statement::Empty)
            return;

        // Since we hold only one statement per inst, we can perform disjunction
        // on a per-inst basis.
        // If an inst does not exist in the current set, then it's an empty statement.
        //
        if (predicates.containsKey(predicate.inst))
        {
            // If the predicate is already in the set, then we can just update it.
            predicates[predicate.inst] =
                statementDisjunction(predicate.block, predicate, predicates[predicate.inst]);
        }
    }

    // Disjunction of a set of statements (a1 v a2 v a3 ...) with the current set.
    void disjunct(StatementSet other)
    {
        for (auto& predicate : other.predicates)
            disjunct(predicate.second);
    }

    // Conjunction of a predicate with the current set (pred ^ (a1 ^ a2 ^ a3 ...))
    void conjunct(Statement predicate)
    {
        if (predicate.type == Statement::Empty)
            return;

        if (predicates.containsKey(predicate.inst))
        {
            // Conjunct the predicate with the appropriate existing predicate
            auto newStatement =
                statementConjunction(predicate.block, predicate, predicates[predicate.inst]);

            if (newStatement.type == Statement::Concrete &&
                newStatement.relation.type == SimpleRelation::Any)
            {
                // If the new statement is trivially true, then we can remove the predicate.
                predicates.remove(predicate.inst);
            }
            else
            {
                // Otherwise, we update the predicate.
                predicates[predicate.inst] = newStatement;
            }
        }
        else
        {
            // Otherwise, we add the predicate to the set.
            predicates[predicate.inst] = predicate;
        }
    }

    // Predicate
    bool isTriviallyFalse()
    {
        for (auto& predicate : predicates)
        {
            if (predicate.second.type == Statement::Concrete &&
                predicate.second.relation.type == SimpleRelation::Impossible)
                return true;
        }
        return false;
    }
};

struct StatementCacheKey2
{
    IRBlock* block;
    IRInst* inst;

    StatementCacheKey2(IRBlock* block, IRInst* inst)
        : block(block), inst(inst)
    {
    }
};

Statement _tryCollectPredicatedStatement(
    Dictionary<StatementCacheKey2, Statement>& cache,
    IRBlock* block,
    IRInst* inst,
    PredicateSet predicateSet)
{
    auto cacheKey = StatementCacheKey2(block, inst);
    if (auto cached = cache.tryGetValue(cacheKey))
        return *cached;

    // Memoization lambda
    auto checkAndMemoize = [&](Statement result)
    {
        // Check that we aren't caching something about a different block or inst.
        SLANG_ASSERT(result.inst == cacheKey.inst);

        for (auto& predicate : predicateSet.predicates)
        {
            if (result.type == Statement::Concrete &&
                doesRelationImply(result.relation, predicate.second.relation.negated()))
            {
                cache[cacheKey] = Statement::empty();
                return Statement::empty();
            }
        }

        cache[cacheKey] = result;
        return result;
    };

    // We'll store a variable for the inst in the block, that we are currently solving for.
    // If we see this variable again, then we have a recursive relation, and we can use that
    // information to find a concrete relation.
    //
    checkAndMemoize(Statement::variable(inst, block));

    // Base cases
    if (isIntegerConstantValue(inst))
    {
        return checkAndMemoize(Statement::concrete(
            inst,
            SimpleRelation::integerRelation(SimpleRelation::Equal, getConstantIntegerValue(inst))));
    }

    if (isBoolConstantValue(inst))
    {
        return checkAndMemoize(
            Statement::concrete(inst, SimpleRelation::boolRelation(getConstantBoolValue(inst))));
    }

    if (inst->getParent() == block && !as<IRParam>(inst))
    {
        // Arithemetic instructions.
        if (inst->getOp() == kIROp_Add || inst->getOp() == kIROp_Sub)
        {
            auto left = inst->getOperand(0);
            auto right = inst->getOperand(1);
            auto isLeftConstant = isIntegerConstantValue(left);
            auto isRightConstant = isIntegerConstantValue(right);

            if (((isLeftConstant || isRightConstant) && (inst->getOp() == kIROp_Add)) ||
                ((isRightConstant) && (inst->getOp() == kIROp_Sub)))
            {
                auto constantVal =
                    isLeftConstant ? getConstantIntegerValue(left) : getConstantIntegerValue(right);
                auto paramOperand = isLeftConstant ? right : left;

                if (inst->getOp() == kIROp_Sub)
                    constantVal = -constantVal;

                auto statementOnOperand =
                    _tryCollectPredicatedStatement(cache, block, paramOperand, predicateSet);

                if (statementOnOperand.type == Statement::Concrete &&
                    statementOnOperand.relation.type == SimpleRelation::IntegerRelation)
                {
                    switch (statementOnOperand.relation.comparator)
                    {
                    case SimpleRelation::Equal:
                        return checkAndMemoize(Statement::concrete(
                            inst,
                            SimpleRelation::integerRelation(
                                SimpleRelation::Equal,
                                constantVal + statementOnOperand.relation.integerValue)));
                    case SimpleRelation::LessThan:
                    case SimpleRelation::LessThanEqual:
                    case SimpleRelation::GreaterThan:
                    case SimpleRelation::GreaterThanEqual:
                        return checkAndMemoize(Statement::concrete(
                            inst,
                            SimpleRelation::integerRelation(
                                statementOnOperand.relation.comparator,
                                constantVal + statementOnOperand.relation.integerValue)));
                    default:
                        break;
                    }
                }
            }
        }

        // If none of the above returned a statement, then the resulting value can take on any
        // value. (can't provide a bound)
        //
        return checkAndMemoize(Statement::concrete(inst, SimpleRelation::anyRelation()));
    }

    // Otherwise, we need to look at the predecessors to see if we can propagate a
    // statement about the inst from the predecessor blocks.
    //
    Statement result = Statement::empty();

    for (auto predecessor : block->getPredecessors())
    {
        auto translatedInst = inst;
        PredicateSet translatedPredicateSet = predicateSet;

        if (as<IRParam>(inst))
        {
            auto paramIndex = getParamIndexInBlock(cast<IRParam>(inst));
            translatedInst =
                as<IRUnconditionalBranch>(predecessor->getTerminator())->getArg(paramIndex);
        }

        for (auto& predicate : predicateSet.predicates)
        {
            if (as<IRParam>(predicate.first))
            {
                auto paramIndex = getParamIndexInBlock(cast<IRParam>(predicate.first));
                auto translatedPredInst =
                    as<IRUnconditionalBranch>(predecessor->getTerminator())->getArg(paramIndex);
                translatedPredicateSet.conjunct(predicate.second.toInst(translatedPredInst));
            }
        }


        auto branchStatements = tryExtractStatements(predecessor->getTerminator(), block);

        for (auto& branchStatement : branchStatements)
            translatedPredicateSet.conjunct(branchStatement);

        if (!translatedPredicateSet.isTriviallyFalse())
        {
            auto statementFromPredecessor = _tryCollectPredicatedStatement(
                cache,
                predecessor,
                translatedInst,
                translatedPredicateSet);
            result = statementDisjunction(block, result, statementFromPredecessor.toInst(inst));
        }
    }

    return checkAndMemoize(result);
}

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
    Statement predicate)
{
    auto cacheKey = StatementCacheKey(block, inst, predicate);
    if (auto cached = cache.tryGetValue(cacheKey))
        return *cached;

    // Memoization lambda
    auto memoize = [&](Statement result)
    {
        // Check that we aren't caching something about a different block or inst.
        SLANG_ASSERT(result.inst == cacheKey.inst);

        cache[cacheKey] = result;
        return result;
    };

    // We'll store a variable for the inst in the block, that we are currently solving for.
    // If we see this variable again, then we have a recursive relation, and we can use that
    // information to find a concrete relation.
    //
    memoize(Statement::variable(inst, block));

    // Base cases
    if (isIntegerConstantValue(inst))
    {
        return memoize(Statement::concrete(
            inst,
            SimpleRelation::integerRelation(SimpleRelation::Equal, getConstantIntegerValue(inst))));
    }

    if (isBoolConstantValue(inst))
    {
        return memoize(
            Statement::concrete(inst, SimpleRelation::boolRelation(getConstantBoolValue(inst))));
    }

    // Arithemetic instructions.
    if (inst->getOp() == kIROp_Add || inst->getOp() == kIROp_Sub)
    {
        auto left = inst->getOperand(0);
        auto right = inst->getOperand(1);
        auto isLeftConstant = isIntegerConstantValue(left);
        auto isRightConstant = isIntegerConstantValue(right);

        if (((isLeftConstant || isRightConstant) && (inst->getOp() == kIROp_Add)) ||
            ((isRightConstant) && (inst->getOp() == kIROp_Sub)))
        {
            auto constantVal =
                isLeftConstant ? getConstantIntegerValue(left) : getConstantIntegerValue(right);
            auto paramOperand = isLeftConstant ? right : left;

            if (inst->getOp() == kIROp_Sub)
                constantVal = -constantVal;

            auto statementOnOperand =
                tryCollectPredicatedStatement(cache, block, paramOperand, predicate);

            if (statementOnOperand.type == Statement::Concrete &&
                statementOnOperand.relation.type == SimpleRelation::IntegerRelation)
            {
                switch (statementOnOperand.relation.comparator)
                {
                case SimpleRelation::Equal:
                    return memoize(Statement::concrete(
                        inst,
                        SimpleRelation::integerRelation(
                            SimpleRelation::Equal,
                            constantVal + statementOnOperand.relation.integerValue)));
                case SimpleRelation::LessThan:
                case SimpleRelation::LessThanEqual:
                case SimpleRelation::GreaterThan:
                case SimpleRelation::GreaterThanEqual:
                    return memoize(Statement::concrete(
                        inst,
                        SimpleRelation::integerRelation(
                            statementOnOperand.relation.comparator,
                            constantVal + statementOnOperand.relation.integerValue)));
                default:
                    break;
                }
            }
        }
    }

    // Is the inst defined in this block, and isn't a parameter?
    // Then, right now, there's not a whole lot we can say.
    //
    if (inst->getParent() == block && !as<IRParam>(inst))
    {
        return memoize(Statement::concrete(inst, SimpleRelation::anyRelation()));
    }

    // Otherwise, we need to look at the predecessors to see if we can propagate a
    // statement about the inst from the predecessor blocks.
    //
    Statement result = Statement::empty();
    for (auto predecessor : block->getPredecessors())
    {
        auto translatedInst = inst;
        auto translatedPredInst = predicate.inst;

        // If the parameter is defined in the current block, we need to translate it to the
        // inst in the predecessor block.
        //
        if (as<IRParam>(inst) && (inst->getParent() == block))
        {
            auto paramIndex = getParamIndexInBlock(cast<IRParam>(inst));
            translatedInst =
                as<IRUnconditionalBranch>(predecessor->getTerminator())->getArg(paramIndex);
        }

        // If the predicate parameter is defined in the current block, we need to translate it
        // to the inst in the predecessor block.
        //
        if (as<IRParam>(predicate.inst) && (predicate.inst->getParent() == block))
        {
            auto predParamIndex = getParamIndexInBlock(cast<IRParam>(predicate.inst));
            translatedPredInst =
                as<IRUnconditionalBranch>(predecessor->getTerminator())->getArg(predParamIndex);
        }

        Statement translatedPredicate = Statement::trivial();

        // If we have a predicate, check that the predicate is true in the predecessor block, by
        // (i) finding the best statement that we can prove to be always true in the predecessor
        // block, and (ii) checking if that implies the predicate to be false.
        //
        if (translatedPredInst)
        {
            auto predecessorPredicateInstStatement = tryCollectPredicatedStatement(
                cache,
                predecessor,
                translatedPredInst,
                Statement::trivial());

            bool isPredStatementConcrete =
                predecessorPredicateInstStatement.type == Statement::Concrete;

            // If A -> ~Pred, then this branch is irrelevant
            if (isPredStatementConcrete && doesRelationImply(
                                               predecessorPredicateInstStatement.relation,
                                               predicate.relation.negated()))
                continue;

            // If A -> Pred, then we'll use an always true predicate (mostly for performance, since
            // we don't want re-calculate the same thing for multiple predicates that are all true)
            //
            if (isPredStatementConcrete &&
                doesRelationImply(predecessorPredicateInstStatement.relation, predicate.relation))
                translatedPredicate = Statement::trivial();
            else
                translatedPredicate = Statement::concrete(translatedPredInst, predicate.relation);
        }
        else
        {
            translatedPredicate = Statement::trivial();
        }

        // Otherwise, collect the whatever statement we can prove given the predicate is true
        // for the predecessor block.
        //
        auto statementFromPredecessor =
            tryCollectPredicatedStatement(cache, predecessor, translatedInst, translatedPredicate)
                .toInst(inst);

        // We can narrow the relation if we know that we got here conditionally.
        // We have a bottleneck function that returns all statements (for all instructions)
        // that we can prove for a given target block of a conditional branch.
        //
        auto branchStatements = tryExtractStatements(predecessor->getTerminator(), block);

        // Do we have anything about 'translatedInst' in the branch statements?
        //
        for (auto& branchStatement : branchStatements)
        {
            SLANG_ASSERT(branchStatement.type == Statement::Concrete);

            if (branchStatement.inst == translatedInst)
            {
                // Refine our statement
                statementFromPredecessor = statementConjunction(
                    block,
                    statementFromPredecessor,
                    branchStatement.toInst(inst));
                continue;
            }

            // There's one more thing we can do. Even if we don't have anything about
            // 'translatedInst', we may be able to find a statement about `translatedInst` that is
            // implied by one of the branch statements.
            //
            // Effectively, we're trying to construct a transitive proof by finding a statement "B",
            // such that A -> B and B -> C. We already have the first part, we just need the second.
            //

            // We'll stick to integer & boolean "==" and "!=" for now, since those cover most
            // scenarios.
            //
            if (!(branchStatement.relation.type == SimpleRelation::IntegerRelation ||
                  branchStatement.relation.type == SimpleRelation::BoolRelation))
                continue;

            if (!(branchStatement.relation.comparator == SimpleRelation::Equal ||
                  branchStatement.relation.comparator == SimpleRelation::NotEqual))
                continue;

            // We'll try to find a statement, by using the branch statement as a predicate.
            //
            auto statementOnInst =
                tryCollectPredicatedStatement(cache, predecessor, translatedInst, branchStatement);

            // Refine our relation..
            statementFromPredecessor =
                statementConjunction(block, statementFromPredecessor, statementOnInst.toInst(inst));
        }

        // The final result is the relation disjunction of all the implications we found for
        // predecessor blocks.
        //
        result = statementDisjunction(block, result, statementFromPredecessor.toInst(inst));
    }

    switch (result.type)
    {
    case Statement::Empty:
        return memoize(Statement::concrete(inst, SimpleRelation::anyRelation()));
    case Statement::Concrete:
        return memoize(result);
    case Statement::Variable:
        if (result.inst == inst && result.block == block)
        {
            // If we got here, then we had a recursive relation that we couldn't resolve.
            // This shouldn't happen..
            //
            SLANG_ASSERT(!"Unable to solve for variable");
        }
        else
        {
            return memoize(Statement::concrete(inst, SimpleRelation::anyRelation()));
        }
    default:
        SLANG_UNREACHABLE("Unhandled statement type");
    }
}

// Different approach...

// Use the equality of the parameter & argument to translate a statement to a predecessor block.
Statement translateToPredecessor(IRBlock* block, IRBlock* predecessor, Statement statement)
{
    if (as<IRParam>(statement.inst) && statement.type == Statement::Concrete)
    {
        auto paramIndex = getParamIndexInBlock(cast<IRParam>(statement.inst));
        auto translatedInst =
            as<IRUnconditionalBranch>(predecessor->getTerminator())->getArg(paramIndex);
        return Statement::concrete(translatedInst, statement.relation);
    }

    // Not a parameter (or we have a non-concrete statement), so no need to translate it.
    return statement;
}

StatementSet translateToPredecessor(IRBlock* block, IRBlock* predecessor, StatementSet statementSet)
{
    StatementSet newStatementSet;
    // Translate each statement in the set.
    for (auto& statement : statementSet.predicates)
        newStatementSet.conjunct(translateToPredecessor(block, predecessor, statement.second));
    return newStatementSet;
}

Statement translateToCurrent(IRBlock* block, IRBlock* predecessor, Statement statement)
{
    List<IRParam*> params;
    for (auto param : block->getParams())
        params.add(param);

    // If the statement is concrete, we need to check if it's an argument in the predecessor's
    // branch
    if (statement.type == Statement::Concrete)
    {
        auto terminator = predecessor->getTerminator();
        auto branch = as<IRUnconditionalBranch>(terminator);

        if (branch)
        {
            // Scan through all arguments to find if statement.inst is one of them
            auto argCount = branch->getArgCount();
            for (UInt argIndex = 0; argIndex < argCount; argIndex++)
            {
                auto arg = branch->getArg(argIndex);
                if (arg == statement.inst)
                {
                    // Found the argument - translate to the corresponding parameter in the current
                    // block
                    auto param = params[argIndex];
                    if (param)
                    {
                        return Statement::concrete(param, statement.relation);
                    }
                }
            }
        }
    }

    // If it's not an argument in the branch or not a concrete statement, no translation needed
    return statement;
}

// Same translation, but from predecessor to current block.
StatementSet translateToCurrent(IRBlock* block, IRBlock* predecessor, StatementSet statementSet)
{
    StatementSet newStatementSet;
    for (auto& statement : statementSet.predicates)
        newStatementSet.conjunct(translateToCurrent(block, predecessor, statement.second));
    return newStatementSet;
}


// Obtain all implications that can be inferred from the predicate in a
// certain block.
//
// This is something we could do on a per-block basis.
//
StatementSet tryGetImplications(
    Dictionary<StatementCacheKey, StatementSet>& cache,
    IRBlock* block,
    Statement predicate)
{
    auto cacheKey = StatementCacheKey(block, predicate.inst, predicate);
    if (auto cached = cache.tryGetValue(cacheKey))
        return *cached;

    auto memoize = [&](StatementSet result)
    {
        cache[cacheKey] = result;
        return result;
    };

    // Memoize a variable for each parameter in this block.
    // This way if we see these variables again, we know that
    // we have a recursive relation.
    //
    // This memoization simultaneously avoids infinite recursion,
    // and allows an elegant representation of recursive relationships.
    //
    StatementSet parameterImplications;
    for (auto param : block->getParams())
        parameterImplications.conjunct(Statement::variable(param, block));
    memoize(parameterImplications);

    // Empty set of statements.
    ShortList<StatementSet, 2> predecessorStatementSets;

    // Merge statements from predecessors.
    for (auto predecessor : block->getPredecessors())
    {
        // Translate predicate and parameters to the predecessor block.


        auto predecessorImplications = tryGetImplications(cache, predecessor, predicate);

        // If we have any extra statements that we can infer from this branch,
        // we'll add them to the list. (This could be the true or false branch of a
        // conditional).
        //
        auto branchStatements = tryExtractStatements(predecessor->getTerminator(), block);

        for (auto& branchStatement : branchStatements)
            predecessorImplications.conjunct(branchStatement);

        if (predecessorImplications.isTriviallyFalse())
            continue;

        predecessorStatementSets.add(predecessorImplications);
    }

    // If we get to a situation where there are no valid
    // predecessors into this block, then something is wrong.
    //
    SLANG_ASSERT(predecessorStatementSets.getCount() > 0);

    if (predecessorStatementSets.getCount() == 0)
        return StatementSet();

    // Disjunction of all the implications.
    StatementSet result = predecessorStatementSets[0];
    for (int i = 1; i < predecessorStatementSets.getCount(); i++)
        result.disjunct(predecessorStatementSets[i]);

    return memoize(result);
}

} // namespace Slang
