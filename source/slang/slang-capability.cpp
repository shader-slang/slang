// slang-capability.cpp
#include "slang-capability.h"

#include "../core/slang-dictionary.h"

// This file implements the core of the "capability" system.

namespace Slang
{

//
// CapabilityAtom
//

// We are going to divide capability atoms into a few categories.
//
enum class CapabilityNameFlavor : int32_t
{
    // A concrete capability atom is something that a target
    // can directly support, where the presence of the feature
    // directly provides functionality. A specific OpenGL
    // or Vulkan extension would be an example of a concrete
    // capability.
    //
    Concrete,

    // An abstract capability represents a class of feature
    // where multiple different implementations might be possible.
    // For example, "ray tracing" might be an abstract feature
    // that a function can require, but a specific target will
    // only be able to provide that abstract feature via some
    // specific concrete feature (e.g., `GL_EXT_ray_tracing`).
    Abstract,

    // An alias capability atom is one that is exactly equivalent
    // to the things it inherits from.
    //
    // For example, a `ps_5_1` capability would just be an
    // alias for the combination of the `fragment` capability
    // and the `sm_5_1` capability.
    //
    Alias,
};

// The macros in the `slang-capability-defs.h` file will be used
// to fill out a `static const` array of information about each
// capability atom.
//
struct CapabilityAtomInfo
{
    /// The API-/language-exposed name of the capability.
    char const* name;

    /// Flavor of atom: concrete, abstract, or alias
    CapabilityNameFlavor        flavor;

    /// If the atom is a direct descendent of an abstract base, keep that for reference here.
    CapabilityName abstractBase;

    /// Ranking to use when deciding if this atom is a "better" one to select.
    uint32_t                    rank;

    /// Canonical representation in the form of disjunction-of-conjunction of atoms.
    ArrayView<ArrayView<CapabilityName>> canonicalRepresentation;
};

#include "slang-generated-capability-defs-impl.h"

static UInt asAtomUInt(CapabilityName name)
{
    SLANG_ASSERT((CapabilityAtom)name < CapabilityAtom::Count);
    return (UInt)((CapabilityAtom)name);
}

static CapabilityAtom asAtom(CapabilityName name)
{
    SLANG_ASSERT((CapabilityAtom)name < CapabilityAtom::Count);
    return (CapabilityAtom)name;
}

/// Get the extended information structure for the given capability `atom`
static CapabilityAtomInfo const& _getInfo(CapabilityName atom)
{
    SLANG_ASSERT(Int(atom) < Int(CapabilityName::Count));
    return kCapabilityNameInfos[Int(atom)];
}
static CapabilityAtomInfo const& _getInfo(CapabilityAtom atom)
{
    SLANG_ASSERT(Int(atom) < Int(CapabilityAtom::Count));
    return kCapabilityNameInfos[Int(atom)];
}

void getCapabilityNames(List<UnownedStringSlice>& ioNames)
{
    ioNames.reserve(Count(CapabilityName::Count));
    for (Index i = 0; i < Count(CapabilityName::Count); ++i)
    {
        if (_getInfo(CapabilityName(i)).flavor != CapabilityNameFlavor::Abstract)
        {
            ioNames.add(UnownedStringSlice(_getInfo(CapabilityName(i)).name));
        }
    }
}

UnownedStringSlice capabilityNameToString(CapabilityName name)
{
    return UnownedStringSlice(_getInfo(name).name);
}

bool isDirectChildOfAbstractAtom(CapabilityAtom name)
{
    return _getInfo(name).abstractBase != CapabilityName::Invalid;
}

bool lookupCapabilityName(const UnownedStringSlice& str, CapabilityName& value);

CapabilityName findCapabilityName(UnownedStringSlice const& name)
{
    CapabilityName result{};
    if (!lookupCapabilityName(name, result))
        return CapabilityName::Invalid;
    return result;
}

bool isCapabilityDerivedFrom(CapabilityAtom atom, CapabilityAtom base)
{
    if (atom == base)
    {
        return true;
    }

    const auto& info = kCapabilityNameInfos[Index(atom)];
    for (auto cur : info.canonicalRepresentation)
    {
        for (auto cbase : cur)
            if (asAtom(cbase) == base)
                return true;
    }

    return false;
}

//// CapabiltySet

void CapabilitySet::addToTargetCapabilityWithValidUIntSetAndTargetAndStage(CapabilityName target, CapabilityName stage, CapabilityAtomSet setToAdd)
{
    SLANG_ASSERT(target != CapabilityName::Invalid && stage != CapabilityName::Invalid);
    auto stageAtom = asAtom(stage);
    auto targetAtom = asAtom(target);
    CapabilityTargetSet& targetSet = m_targetSets[targetAtom];
    targetSet.target = targetAtom;
    targetSet.shaderStageSets.reserve(kCapabilityStageCount);

    auto& localStageSets = targetSet.shaderStageSets[stageAtom];
    localStageSets.stage = stageAtom;

    localStageSets.addNewSet(std::move(setToAdd));
}

void CapabilitySet::addToTargetCapabilityWithTargetAndStageAtom(CapabilityName target, CapabilityName stage, const ArrayView<CapabilityName>& canonicalRepresentation)
{
    // If no provided 'stage', set the capability as a target of all stages
    if (stage == CapabilityName::Invalid)
    {
        auto info = _getInfo(CapabilityName::any_stage);
        List<CapabilityName> newArr;
        auto count = canonicalRepresentation.getCount();
        newArr.setCount(count + 1);
        memcpy(newArr.getBuffer(), canonicalRepresentation.getBuffer(), count * sizeof(CapabilityName));
        m_targetSets[asAtom(target)].shaderStageSets.reserve(info.canonicalRepresentation.getCount());
        for (auto i : info.canonicalRepresentation)
        {
            newArr[count] = i[0];
            addToTargetCapabilityWithTargetAndStageAtom(target, i[0], newArr.getArrayView());
        }
        return;
    }
    
    CapabilityAtomSet setToAdd = CapabilityAtomSet((UInt)CapabilityAtom::Count);
    for(auto i : canonicalRepresentation)
        setToAdd.add(asAtomUInt(i));

    addToTargetCapabilityWithValidUIntSetAndTargetAndStage(target, stage, setToAdd);
}

// No targets atoms have been defined on yet, set stage to target any_target capability 
void CapabilitySet::addToTargetCapabilityWithStageAtom(CapabilityName stage, const ArrayView<CapabilityName>& canonicalRepresentation)
{
    
    if (m_targetSets.getCount() == 0)
    {
        const auto anyTargetInfo = _getInfo(CapabilityName::any_target);
        CapabilityAtomSet setToAdd;
        setToAdd.resize((UInt)CapabilityAtom::Count);
        for (int i = 0; i < canonicalRepresentation.getCount(); i++)
            setToAdd.add((UInt)canonicalRepresentation[i]);
        CapabilityName targetAtom{};
        for (const auto& targetAtomCanonicalRep : anyTargetInfo.canonicalRepresentation)
        {
            for (auto anyTargetAtom : targetAtomCanonicalRep)
            {
                setToAdd.add((UInt)anyTargetAtom);
                if (_getInfo(anyTargetAtom).abstractBase == CapabilityName::target)
                    targetAtom = anyTargetAtom;
            }
            addToTargetCapabilityWithValidUIntSetAndTargetAndStage(targetAtom, stage, setToAdd);
            for (auto anyTargetAtom : targetAtomCanonicalRep)
                setToAdd.remove((UInt)anyTargetAtom);
        }
    }
}

void CapabilitySet::addToTargetCapabilityWithTargetAndOrStageAtom(CapabilityName target, CapabilityName stage, const ArrayView<CapabilityName>& canonicalRepresentation)
{
    if(target != CapabilityName::Invalid)
        addToTargetCapabilityWithTargetAndStageAtom(target, stage, canonicalRepresentation);
    else if(stage != CapabilityName::Invalid)
        addToTargetCapabilityWithStageAtom(stage, canonicalRepresentation);
}

void CapabilitySet::addToTargetCapabilitesWithCanonicalRepresentation(const ArrayView<CapabilityName>& canonicalRepresentation)
{
    // only need to search i == 0/1 to find a relevant node
    // target node should ALWAYS be first, so if we find a node, we stop searching. This is the most important node. We assume only stage+target with this logic.
    // canonicalRepresentation of node has optionally 0-1 abstract node of each type, with a minimum of 1 abstract node total.
    CapabilityName target = CapabilityName::Invalid;
    CapabilityName stage = CapabilityName::Invalid;
    for (const auto& i : canonicalRepresentation)
    {
        const auto info = _getInfo(i);
        if (info.abstractBase == CapabilityName::Invalid)
            continue;
        else if (info.abstractBase == CapabilityName::target)
            target = i;
        else if (info.abstractBase == CapabilityName::stage)
            stage = i;

        if (target != CapabilityName::Invalid && stage != CapabilityName::Invalid)
            break;
    }

    addToTargetCapabilityWithTargetAndOrStageAtom(target, stage, canonicalRepresentation);
}

void CapabilitySet::addUnexpandedCapabilites(CapabilityName atom)
{
    auto info = _getInfo(atom);
    for (const auto& cr : info.canonicalRepresentation)
        addToTargetCapabilitesWithCanonicalRepresentation(cr);
}

CapabilitySet::CapabilitySet()
{}

CapabilitySet::CapabilitySet(Int atomCount, CapabilityName const* atoms)
{
    for (Int i = 0; i < atomCount; i++)
        addCapability(atoms[i]);
}

CapabilitySet::CapabilitySet(CapabilityName atom)
{
    this->m_targetSets.reserve(kCapabilityTargetCount);
    addUnexpandedCapabilites(atom);
}

CapabilitySet::CapabilitySet(List<CapabilityName> const& atoms)
{
    for (auto atom : atoms)
        addCapability(atom);
}

CapabilitySet CapabilitySet::makeEmpty()
{
    return CapabilitySet();
}

CapabilitySet CapabilitySet::makeInvalid()
{
    CapabilitySet result;
    result.m_targetSets[CapabilityAtom::Invalid].target = CapabilityAtom::Invalid;

    return result;
}

void CapabilitySet::addCapability(CapabilityName name)
{
    join(CapabilitySet(name));
}

bool CapabilitySet::isEmpty() const
{
    return m_targetSets.getCount() == 0;
}

bool CapabilitySet::isInvalid() const
{
    return m_targetSets.containsKey(CapabilityAtom::Invalid);
}

bool CapabilitySet::isIncompatibleWith(CapabilityAtom other) const
{
    // should be a target or derivative, otherwise this makes no sense.

    if (isEmpty())
        return false;
    
    CapabilitySet otherSet((CapabilityName)other);
    return isIncompatibleWith(otherSet);
}

bool CapabilitySet::isIncompatibleWith(CapabilityName other) const
{
    if (isEmpty())
        return false;
    auto otherSet = CapabilitySet(other);
    return isIncompatibleWith(otherSet);
}

bool CapabilitySet::isIncompatibleWith(CapabilitySet const& other) const
{
    if (isEmpty())
        return false;
    if (other.isEmpty())
        return false;

    // Incompatible means there are 0 intersecting abstract nodes from sets in `other` with sets in `this`
    for (auto& otherSet : other.m_targetSets)
    {
        auto targetSet = this->m_targetSets.tryGetValue(otherSet.first);
        if (!targetSet)
            continue;

        for (auto& otherStageSet : otherSet.second.shaderStageSets)
        {
            auto stageSet = targetSet->shaderStageSets.tryGetValue(otherStageSet.first);
            if (!stageSet)
                continue;

            return false;
        }
    }
    return true;
}

const CapabilityAtomSet& getAtomSetOfTargets()
{
    return kAnyTargetUIntSetBuffer;
}
const CapabilityAtomSet& getAtomSetOfStages()
{
    return kAnyStageUIntSetBuffer;
}

bool hasTargetAtom(const CapabilityAtomSet& setIn, CapabilityAtom& targetAtom)
{
    CapabilityAtomSet intersection;
    setIn.calcIntersection(intersection, getAtomSetOfTargets(), setIn);

    if (intersection.isEmpty())
        return false;

    targetAtom = intersection.getElements<CapabilityAtom>().getLast();
    return true;
}

bool CapabilitySet::implies(CapabilityAtom atom) const
{
    if (isEmpty() || atom == CapabilityAtom::Invalid)
        return false;

    CapabilitySet tmpSet = CapabilitySet(CapabilityName(atom));

    return this->implies(tmpSet);
}

bool CapabilitySet::implies(CapabilitySet const& other, const bool onlyRequireSingleImply) const
{
    // x implies (c | d) only if (x implies c) and (x implies d).

    for (const auto& otherTarget : other.m_targetSets)
    {
        auto thisTarget = this->m_targetSets.tryGetValue(otherTarget.first);
        if (!thisTarget)
        {
            // 'this' lacks a target 'other' has.
            return false;
        }

        for (const auto& otherStage : otherTarget.second.shaderStageSets)
        {
            auto thisStage = thisTarget->shaderStageSets.tryGetValue(otherStage.first);
            if (!thisStage)
            {
                // 'this' lacks a stage 'other' has.
                return false;
            }

            // all stage sets that are in 'other' must be contained by 'this'
            if(thisStage->atomSet)
            {
                auto& thisStageSet = thisStage->atomSet.value();
                if(otherStage.second.atomSet)
                {   
                    if (!onlyRequireSingleImply)
                    {
                        if (!thisStageSet.contains(otherStage.second.atomSet.value()))
                            return false;
                    }
                    else
                    {
                        if (thisStageSet.contains(otherStage.second.atomSet.value()))
                            return true;
                    }
                }
            }
        }
    }
    return !onlyRequireSingleImply;
}

void CapabilityTargetSet::unionWith(const CapabilityTargetSet& other)
{
    for (auto otherStageSet : other.shaderStageSets)
    {
        auto& thisStageSet = this->shaderStageSets[otherStageSet.first];
        thisStageSet.stage = otherStageSet.first;

        if (!thisStageSet.atomSet)
            thisStageSet.atomSet = otherStageSet.second.atomSet;
        else
            if(otherStageSet.second.atomSet)
                thisStageSet.atomSet->unionWith(*otherStageSet.second.atomSet);
    }
}

void CapabilitySet::unionWith(const CapabilitySet& other)
{
    if (this->isInvalid() || other.isInvalid())
        return;

    this->m_targetSets.reserve(other.m_targetSets.getCount());
    for (auto otherTargetSet : other.m_targetSets)
    {
        CapabilityTargetSet& thisTargetSet = this->m_targetSets[otherTargetSet.first];
        thisTargetSet.target = otherTargetSet.first;
        thisTargetSet.shaderStageSets.reserve(otherTargetSet.second.shaderStageSets.getCount());
        thisTargetSet.unionWith(otherTargetSet.second);
    }
}

/// Join sets, but: 
/// 1. do not destroy target set's which are incompatible with `other` (destroying shaderStageSets is fine)
/// 2. do not create an `CapabilityAtom::Invalid` target set.
void CapabilitySet::nonDestructiveJoin(const CapabilitySet& other)
{
    if (this->isInvalid() || other.isInvalid())
        return;

    if (this->isEmpty())
    {
        this->m_targetSets = other.m_targetSets;
        return;
    }
    for (auto& thisTargetSet : this->m_targetSets)
    {
        thisTargetSet.second.tryJoin(other.m_targetSets);
    }
}

void CapabilitySet::addCapability(List<List<CapabilityAtom>>& atomLists)
{
    for (const auto& cr : atomLists)
        addToTargetCapabilitesWithCanonicalRepresentation( (*(List<CapabilityName>*)(&cr)).getArrayView());
}

CapabilitySet CapabilitySet::getTargetsThisHasButOtherDoesNot(const CapabilitySet& other)
{
    CapabilitySet newSet{};
    for (auto& i : this->m_targetSets)
    {
        if (other.m_targetSets.tryGetValue(i.first))
            continue;

        newSet.m_targetSets[i.first].target = i.first;
        auto info = _getInfo(i.first);
        if(info.canonicalRepresentation.getCount() > 0)
            newSet.addToTargetCapabilityWithTargetAndStageAtom((CapabilityName)i.first, CapabilityName::Invalid, info.canonicalRepresentation[0]);
    }
    return newSet;
}

/// Join `this` with a compatble stage set of `CapabilityTargetSet other`.
/// Return false when `other` is fully incompatible.
/// incompatability is when `this->stage` is not a supported stage by `other.shaderStageSets`.
bool CapabilityStageSet::tryJoin(const CapabilityTargetSet& other)
{
    const CapabilityStageSet* otherStageSet = other.shaderStageSets.tryGetValue(this->stage);
    if (!otherStageSet)
        return false;

    // should not exceed far beyond 2*2 or 1*1 elements
    if(otherStageSet->atomSet && this->atomSet)
        this->atomSet->add(*otherStageSet->atomSet);

    return true;
}

/// Join a compatable target set from `this` with `CapabilityTargetSet other`.
/// Return false when `other` is fully incompatible.
/// incompatability is when one of 2 senarios are true:
/// 1. `this->target` is not a supported target by `other.shaderStageSets`
/// 2. `this` has completly disjoint shader stages from other.
bool CapabilityTargetSet::tryJoin(const CapabilityTargetSets& other)
{
    const CapabilityTargetSet* otherTargetSet = other.tryGetValue(this->target);
    if (otherTargetSet == nullptr)
        return false;

    List<CapabilityAtom> destroySet;
    destroySet.reserve(this->shaderStageSets.getCount());
    for (auto& shaderStageSet : this->shaderStageSets)
    {
        if (!shaderStageSet.second.tryJoin(*otherTargetSet))
            destroySet.add(shaderStageSet.first);
    }
    if (destroySet.getCount() == Slang::Index(this->shaderStageSets.getCount()))
        return false;

    for (const auto& i : destroySet)
        this->shaderStageSets.remove(i);

    return true;
}

void CapabilitySet::join(const CapabilitySet& other)
{
    if (this->isEmpty() || other.isInvalid())
    {
        *this = other;
        return;
    }
    if (this->isInvalid())
        return;
    if (other.isEmpty())
        return;

    List<CapabilityAtom> destroySet;
    destroySet.reserve(this->m_targetSets.getCount());
    for (auto& thisTargetSet : this->m_targetSets)
    {
        if (!thisTargetSet.second.tryJoin(other.m_targetSets))
        {
            destroySet.add(thisTargetSet.first);
        }
    }
    for (const auto& i : destroySet)
    {
        this->m_targetSets.remove(i);
    }
    // join made a invalid CapabilitySet
    if (this->m_targetSets.getCount() == 0)
        this->m_targetSets[CapabilityAtom::Invalid].target = CapabilityAtom::Invalid;
}

static uint32_t _calcAtomListDifferenceScore(List<CapabilityAtom> const& thisList, List<CapabilityAtom> const& thatList)
{
    uint32_t score = 0;

    // Our approach here will be to scan through `this` and `that`
    // to identify atoms that are in `this` but not `that` (that is,
    // the atoms that would be present in the set difference `this - that`)
    // and then compute the maximum rank/score of those atoms.

    Index thisCount = thisList.getCount();
    Index thatCount = thatList.getCount();
    Index thisIndex = 0;
    Index thatIndex = 0;
    for (;;)
    {
        if (thisIndex == thisCount) break;
        if (thatIndex == thatCount) break;

        auto thisAtom = thisList[thisIndex];
        auto thatAtom = thatList[thatIndex];

        if (thisAtom == thatAtom)
        {
            thisIndex++;
            thatIndex++;
            continue;
        }

        if (thisAtom < thatAtom)
        {
            // `thisAtom` is not present in `that`, so it
            // should contribute to our ranking of the difference.
            //
            auto thisAtomInfo = _getInfo(thisAtom);
            auto thisAtomRank = thisAtomInfo.rank;

            if (thisAtomRank > score)
            {
                score = thisAtomRank;
            }

            thisIndex++;
        }
        else
        {
            SLANG_ASSERT(thisAtom > thatAtom);
            thatIndex++;
        }
    }
    return score;
}

bool CapabilitySet::hasSameTargets(const CapabilitySet& other) const
{
    for (const auto& i : this->m_targetSets)
    {
        if (!other.m_targetSets.tryGetValue(i.first))
            return false;
    }
    return this->m_targetSets.getCount() == other.m_targetSets.getCount();
}


// MSVC incorrectly throws warning
#pragma warning(push)
#pragma warning(disable:4702)
/// returns true if 'this' is a better target for 'targetCaps' than 'that'
/// isEqual: is `this` and `that` equal
/// isIncompatible: is `this` and `that` incompatible
bool CapabilitySet::isBetterForTarget(CapabilitySet const& that, CapabilitySet const& targetCaps, bool& isEqual) const
{
    if (this->isEmpty() && (that.isEmpty() || that.isInvalid()))
    {
        if(this->isEmpty() && that.isEmpty())
            isEqual = true;
        return true;
    }

    // required to have target.
    for (auto& targetWeNeed : targetCaps.m_targetSets)
    {
        auto thisTarget = this->m_targetSets.tryGetValue(targetWeNeed.first);
        if (!thisTarget)
        {
            isEqual = hasSameTargets(that);
            return false;
        }
        auto thatTarget = that.m_targetSets.tryGetValue(targetWeNeed.first);
        if (!thatTarget)
        {
            isEqual = hasSameTargets(that);
            return true;
        }

        // required to have shader stage
        for (auto& shaderStageSetsWeNeed : targetWeNeed.second.shaderStageSets)
        {
            auto thisStageSets = thisTarget->shaderStageSets.tryGetValue(shaderStageSetsWeNeed.first);
            if (!thisStageSets)
                return false;
            auto thatStageSets = thatTarget->shaderStageSets.tryGetValue(shaderStageSetsWeNeed.first);
            if (!thatStageSets)
                return true;

            // We want the smallest (most specialized) set which is still contained by this/that. This means:
            // 1. target.contains(this/that)
            // 2. choose smallest super set
            // 3. rank each super set and their atoms, choose the smallest rank'd set (most specialized)
            if(shaderStageSetsWeNeed.second.atomSet)
            {
                auto& shaderStageSetWeNeed = shaderStageSetsWeNeed.second.atomSet.value();

                CapabilityAtomSet tmp_set{};
                Index tmpCount = 0;
                CapabilityAtomSet thisSet{};
                Index thisSetCount = 0;
                CapabilityAtomSet thatSet{};
                Index thatSetCount = 0;

                // subtraction of the set we want gets us the "elements which 'targetSet' has but `this/that` is less specialized for"
                if(thisStageSets->atomSet)
                {
                    auto& thisStageSet = thisStageSets->atomSet.value();
                    // if `thisStageSet` is more specialized than the target, `thisStageSet` should not be a candidate
                    if (thisStageSet == shaderStageSetWeNeed)
                        return true; 
                    if (shaderStageSetWeNeed.contains(thisStageSet))
                    {
                        CapabilityAtomSet::calcSubtract(tmp_set, shaderStageSetWeNeed, thisStageSet);
                        tmpCount = tmp_set.countElements();
                        if (thisSetCount < tmpCount)
                        {
                            thisSet = tmp_set;
                            thisSetCount = tmpCount;
                        }
                    }
                }
                if (thatStageSets->atomSet)
                {
                    auto& thatStageSet = thatStageSets->atomSet.value();
                    if (thatStageSet == shaderStageSetWeNeed)
                        return false;
                    if (shaderStageSetWeNeed.contains(thatStageSet))
                    {
                        CapabilityAtomSet::calcSubtract(tmp_set, shaderStageSetWeNeed, thatStageSet);
                        tmpCount = tmp_set.countElements();
                        if (thatSetCount < tmpCount)
                        {
                            thatSet = tmp_set;
                            thatSetCount = tmpCount;
                        }
                    }
                }

                if (thisSet == thatSet)
                    isEqual = true;
                
                //empty means no candidate
                if (thisSet.areAllZero())
                    return false;
                if (thatSet.areAllZero())
                    return true;
                if (thisSetCount < thatSetCount)
                    return true;
                else if (thisSetCount > thatSetCount)
                    return false;
                
                auto thisSetElements = thisSet.getElements<CapabilityAtom>();
                auto thatSetElements = thisSet.getElements<CapabilityAtom>();
                auto shaderStageSetWeNeedElements = shaderStageSetWeNeed.getElements<CapabilityAtom>();

                auto thisDiffScore = _calcAtomListDifferenceScore(thisSetElements, shaderStageSetWeNeedElements);
                auto thatDiffScore = _calcAtomListDifferenceScore(thisSetElements, shaderStageSetWeNeedElements);

                return thisDiffScore < thatDiffScore;
            }
        }
    }
    return true;
}
#pragma warning(pop)

CapabilitySet::AtomSets::Iterator CapabilitySet::getAtomSets() const
{
    return CapabilitySet::AtomSets::Iterator(&this->getCapabilityTargetSets()).begin();
}

bool CapabilitySet::checkCapabilityRequirement(CapabilitySet const& available, CapabilitySet const& required, CapabilityAtomSet& outFailedAvailableSet)
{
    // Requirements x are met by available disjoint capabilities (a | b) iff
    // both 'a' satisfies x and 'b' satisfies x.
    // If we have a caller function F() decorated with:
    //     [require(hlsl, _sm_6_3)] [require(spirv, _spv_ray_tracing)] void F() { g(); }
    // We'd better make sure that `g()` can be compiled with both (hlsl+_sm_6_3) and (spirv+_spv_ray_tracing) capability sets.
    // In this method, F()'s capability declaration is represented by `available`,
    // and g()'s capability is represented by `required`.
    // We will check that for every capability conjunction X of F(), there is one capability conjunction Y in g() such that X implies Y.
    // 

    // if empty there is no body, all capabilities are supported.
    if (required.isEmpty())
        return true;

    if (required.isInvalid())
    {
        outFailedAvailableSet.add((UInt)CapabilityAtom::Invalid);
        return false;
    }

    // If F's capability is empty, we can satisfy any non-empty requirements.
    //
    if (available.isEmpty() && !required.isEmpty())
        return false;
    
    
    // if all sets in `available` are not a super-set to at least 1 `required` set, then we have an err
    for (auto& availableTarget : available.m_targetSets)
    {
        auto reqTarget = required.m_targetSets.tryGetValue(availableTarget.first);
        if (!reqTarget)
        {
            outFailedAvailableSet.add((UInt)availableTarget.first);
            return false;
        }

        for (auto& availableStage : availableTarget.second.shaderStageSets)
        {
            auto reqStage = reqTarget->shaderStageSets.tryGetValue(availableStage.first);
            if (!reqStage)
            {
                outFailedAvailableSet.add((UInt)availableStage.first);
                return false;
            }

            const CapabilityAtomSet* lastBadStage = nullptr;
            if(availableStage.second.atomSet)
            {
                const auto& availableStageSet = availableStage.second.atomSet.value();
                lastBadStage = nullptr;
                if(reqStage->atomSet)
                {
                    const auto& reqStageSet = reqStage->atomSet.value();
                    if (availableStageSet.contains(reqStageSet))
                        break;
                    else 
                        lastBadStage = &reqStageSet;
                }
                if (lastBadStage)
                {
                    // get missing atoms
                    CapabilityAtomSet::calcSubtract(outFailedAvailableSet, *lastBadStage, availableStageSet);
                    return false;
                }
            }
        }               
    }

    return true;
}

void printDiagnosticArg(StringBuilder& sb, const CapabilitySet& capSet)
{
    bool isFirstSet = true;
    for (auto& set : capSet.getAtomSets())
    {
        if (!isFirstSet)
        {
            sb<< " | ";
        }
        bool isFirst = true;
        for (auto atom : set)
        {
            CapabilityName formattedAtom = (CapabilityName)atom;
            if (!isFirst)
            {
                sb << " + ";
            }
            auto name = capabilityNameToString((CapabilityName)formattedAtom);
            if (name.startsWith("_"))
                name = name.tail(1);
            sb << name;
            isFirst = false;
        }
        isFirstSet = false;
    }
}

void printDiagnosticArg(StringBuilder& sb, CapabilityAtom atom)
{
    printDiagnosticArg(sb, (CapabilityName)atom);
}

void printDiagnosticArg(StringBuilder& sb, CapabilityName name)
{
    sb << _getInfo(name).name;
}

#ifdef UNIT_TEST_CAPABILITIES

#define CHECK_CAPS(inData) SLANG_ASSERT(inData>0)

int TEST_findTargetCapSet(CapabilitySet& capSet, CapabilityAtom target)
{
    return true
        && capSet.getCapabilityTargetSets().containsKey(target);
}

int TEST_findTargetStage(
    CapabilitySet& capSet,
    CapabilityAtom target,
    CapabilityAtom stage)
{
    return capSet.getCapabilityTargetSets()[target].shaderStageSets.containsKey(stage);
}

int TEST_targetCapSetWithSpecificSetInStage(
    CapabilitySet& capSet,
    CapabilityAtom target,
    CapabilityAtom stage,
    List<CapabilityAtom> setToFind)
{

    bool containsStageKey = capSet.getCapabilityTargetSets()[target].shaderStageSets.containsKey(stage);
    if (!containsStageKey) 
        return 0;

    auto& stageSet = capSet.getCapabilityTargetSets()[target].shaderStageSets[stage];
    if (stage != stageSet.stage)
        return -1;

    CapabilityAtomSet set;
    for (auto i : setToFind)
        set.add(UInt(i));

    if (stageSet.atomSet)
    {
        auto& i = stageSet.atomSet.value();
        if (i == set)
            return true;
    }

    return -2;
}

void TEST_CapabilitySet_addAtom()
{
    CapabilitySet testCapSet{};

    // ------------------------------------------------------------

    testCapSet = CapabilitySet(CapabilityName::TEST_ADD_1);

    CHECK_CAPS(TEST_findTargetCapSet(testCapSet, CapabilityAtom::hlsl));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(testCapSet, CapabilityAtom::hlsl, CapabilityAtom::vertex,
        { CapabilityAtom::textualTarget, CapabilityAtom::hlsl, CapabilityAtom::vertex,
        CapabilityAtom::_sm_4_0, CapabilityAtom::_sm_4_1 }));

    CHECK_CAPS(TEST_findTargetCapSet(testCapSet, CapabilityAtom::glsl));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(testCapSet, CapabilityAtom::glsl, CapabilityAtom::vertex,
        { CapabilityAtom::textualTarget, CapabilityAtom::glsl, CapabilityAtom::vertex,
        CapabilityAtom::_GLSL_130 }));

    CHECK_CAPS(TEST_findTargetCapSet(testCapSet, CapabilityAtom::spirv_1_0));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(testCapSet, CapabilityAtom::spirv_1_0, CapabilityAtom::vertex,
        { CapabilityAtom::spirv_1_0, CapabilityAtom::vertex,
        CapabilityAtom::spirv_1_1 }));

    CHECK_CAPS(TEST_findTargetCapSet(testCapSet, CapabilityAtom::metal));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(testCapSet, CapabilityAtom::metal, CapabilityAtom::vertex,
        { CapabilityAtom::textualTarget, CapabilityAtom::metal, CapabilityAtom::vertex }));

    // ------------------------------------------------------------

    testCapSet = CapabilitySet(CapabilityName::TEST_ADD_2);

    CHECK_CAPS(TEST_findTargetCapSet(testCapSet, CapabilityAtom::hlsl));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(testCapSet, CapabilityAtom::hlsl, CapabilityAtom::vertex,
        { CapabilityAtom::textualTarget, CapabilityAtom::hlsl, CapabilityAtom::vertex,
        CapabilityAtom::_sm_4_0, CapabilityAtom::_sm_4_1 }));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(testCapSet, CapabilityAtom::hlsl, CapabilityAtom::fragment,
        { CapabilityAtom::textualTarget, CapabilityAtom::hlsl, CapabilityAtom::fragment,
        CapabilityAtom::_sm_4_0, CapabilityAtom::_sm_4_1 }));

    // ------------------------------------------------------------

    testCapSet = CapabilitySet(CapabilityName::TEST_ADD_3);

    CHECK_CAPS((int)!TEST_findTargetCapSet(testCapSet, CapabilityAtom::spirv_1_0));
    CHECK_CAPS(TEST_findTargetCapSet(testCapSet, CapabilityAtom::glsl));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(testCapSet, CapabilityAtom::glsl, CapabilityAtom::fragment,
        { CapabilityAtom::textualTarget, CapabilityAtom::glsl, CapabilityAtom::fragment,
        CapabilityAtom::_GLSL_130 }));
    // ------------------------------------------------------------
}

void TEST_CapabilitySet_join()
{
    CapabilitySet testCapSetA{};
    CapabilitySet testCapSetB{};

    // ------------------------------------------------------------

    testCapSetA = CapabilitySet(CapabilityName::TEST_JOIN_1A);
    testCapSetB = CapabilitySet(CapabilityName::TEST_JOIN_1B);
    testCapSetA.join(testCapSetB);

    CHECK_CAPS((int)!TEST_findTargetCapSet(testCapSetA, CapabilityAtom::hlsl));
    CHECK_CAPS((int)!TEST_findTargetCapSet(testCapSetA, CapabilityAtom::glsl));

    // ------------------------------------------------------------

    testCapSetA = CapabilitySet(CapabilityName::TEST_JOIN_2A);
    testCapSetB = CapabilitySet(CapabilityName::TEST_JOIN_2B);
    testCapSetA.join(testCapSetB);

    CHECK_CAPS(TEST_findTargetCapSet(testCapSetA, CapabilityAtom::hlsl));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(testCapSetA, CapabilityAtom::hlsl, CapabilityAtom::vertex,
        { CapabilityAtom::textualTarget, CapabilityAtom::hlsl, CapabilityAtom::vertex,
        CapabilityAtom::_sm_4_0, CapabilityAtom::_sm_4_1 }));
    
    // ------------------------------------------------------------

    testCapSetA = CapabilitySet(CapabilityName::TEST_JOIN_3A);
    testCapSetB = CapabilitySet(CapabilityName::TEST_JOIN_3B);
    testCapSetA.join(testCapSetB);

    CHECK_CAPS((int)!TEST_findTargetCapSet(testCapSetA, CapabilityAtom::spirv_1_0));
    CHECK_CAPS(TEST_findTargetCapSet(testCapSetA, CapabilityAtom::glsl));
    CHECK_CAPS((int)!TEST_findTargetStage(testCapSetA, CapabilityAtom::glsl, CapabilityAtom::raygen));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(testCapSetA, CapabilityAtom::glsl, CapabilityAtom::fragment,
        { CapabilityAtom::textualTarget, CapabilityAtom::glsl, CapabilityAtom::fragment,
        CapabilityAtom::_GLSL_130, CapabilityAtom::_GLSL_140 }));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(testCapSetA, CapabilityAtom::glsl, CapabilityAtom::vertex,
        { CapabilityAtom::textualTarget, CapabilityAtom::glsl, CapabilityAtom::vertex,
        CapabilityAtom::_GLSL_130, CapabilityAtom::_GLSL_140 }));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(testCapSetA, CapabilityAtom::hlsl, CapabilityAtom::fragment,
        { CapabilityAtom::textualTarget, CapabilityAtom::hlsl, CapabilityAtom::fragment,
        CapabilityAtom::_sm_4_0, CapabilityAtom::_sm_4_1 }));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(testCapSetA, CapabilityAtom::hlsl, CapabilityAtom::vertex,
        { CapabilityAtom::textualTarget, CapabilityAtom::hlsl, CapabilityAtom::vertex,
        CapabilityAtom::_sm_4_0 }));

    // ------------------------------------------------------------

    testCapSetA = CapabilitySet(CapabilityName::TEST_JOIN_4A);
    testCapSetB = CapabilitySet(CapabilityName::TEST_JOIN_4B);
    testCapSetA.join(testCapSetB);

    CHECK_CAPS(TEST_findTargetCapSet(testCapSetA, CapabilityAtom::glsl));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(testCapSetA, CapabilityAtom::glsl, CapabilityAtom::fragment,
        { CapabilityAtom::textualTarget, CapabilityAtom::glsl, CapabilityAtom::fragment,
        CapabilityAtom::_GLSL_130, CapabilityAtom::_GLSL_140, CapabilityAtom::_GLSL_150, CapabilityAtom::_GL_EXT_texture_query_lod, CapabilityAtom::_GL_EXT_texture_shadow_lod }));

    // ------------------------------------------------------------


}

void TEST_CapabilitySet()
{
    TEST_CapabilitySet_addAtom();
    TEST_CapabilitySet_join();
}

/*
/// Test Capabilities

alias TEST_ADD_1 = _sm_4_1 | _GLSL_130 | spirv_1_1 | metal
                    ;

alias TEST_ADD_2 = _sm_4_1 | _sm_4_0 + shader_stages_compute_fragment
                    ;

alias TEST_ADD_3 = _GLSL_130 + shader_stages_compute_fragment_geometry_vertex;

//

alias TEST_JOIN_1A = hlsl;
alias TEST_JOIN_1B = glsl;

alias TEST_JOIN_2A = hlsl;
alias TEST_JOIN_2B = _sm_4_1 | glsl;

alias TEST_JOIN_3A = glsl + fragment | _sm_4_0 + fragment 
                    | glsl + vertex | hlsl + vertex
                    ;
alias TEST_JOIN_3B = _sm_4_1 + fragment 
                    | _sm_4_0 + vertex
                    | _sm_4_0 + compute
                    | _GLSL_140 + vertex
                    | _GLSL_140 + fragment
                    | spirv_1_0 + fragment
                    | glsl + raygen 
                    | hlsl + raygen
                    ;

alias TEST_JOIN_4A = _GLSL_140 + _GL_EXT_texture_query_lod;
alias TEST_JOIN_4B = _GLSL_150 + _GL_EXT_texture_shadow_lod;
///
*/
#undef CHECK_CAPS

#endif

}
