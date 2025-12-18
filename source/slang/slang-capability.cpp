// slang-capability.cpp
#include "slang-capability.h"

#include "core/slang-dictionary.h"
#include "core/slang-performance-profiler.h"
#include "slang-ast-builder.h"
#include "slang-capability-val.h"

#include <ranges>

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
    // where multiple distinct implementations might be possible.
    // 'raytracing' may be allowed with a 'raygen' "stage", but
    // not a 'vertex' "stage"
    // For more information (and a clearer description of the rules),
    // read `slang-capabilities.capdef`
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
    UnownedStringSlice name;

    /// Flavor of atom: concrete, abstract, or alias
    CapabilityNameFlavor flavor;

    /// If the atom is a direct descendent of an abstract base, keep that for reference here.
    CapabilityName abstractBase;

    /// Ranking to use when deciding if this atom is a "better" one to select.
    uint32_t rank;

    /// Canonical representation of atoms in the form of disjoint conjunctions of atoms.
    ArrayView<CapabilityAtomSet*> canonicalRepresentation;
};

#include "slang-generated-capability-defs-impl.h"

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
            ioNames.add(_getInfo(CapabilityName(i)).name);
        }
    }
}

UnownedStringSlice capabilityNameToString(CapabilityName name)
{
    return _getInfo(name).name;
}

bool isDirectChildOfAbstractAtom(CapabilityAtom name)
{
    return _getInfo(name).abstractBase != CapabilityName::Invalid;
}

bool isStageAtom(CapabilityName name, CapabilityName& outCanonicalStage)
{
    auto& info = _getInfo(name);
    if (info.abstractBase == CapabilityName::stage)
    {
        outCanonicalStage = name;
        return true;
    }
    switch (name)
    {
    case CapabilityName::anyhit:
        outCanonicalStage = CapabilityName::_anyhit;
        return true;
    case CapabilityName::closesthit:
        outCanonicalStage = CapabilityName::_closesthit;
        return true;
    case CapabilityName::miss:
        outCanonicalStage = CapabilityName::_miss;
        return true;
    case CapabilityName::intersection:
        outCanonicalStage = CapabilityName::_intersection;
        return true;
    case CapabilityName::raygen:
        outCanonicalStage = CapabilityName::_raygen;
        return true;
    case CapabilityName::callable:
        outCanonicalStage = CapabilityName::_callable;
        return true;
    case CapabilityName::mesh:
        outCanonicalStage = CapabilityName::_mesh;
        return true;
    case CapabilityName::amplification:
        outCanonicalStage = CapabilityName::_amplification;
        return true;
    default:
        return false;
    }
}

bool isTargetVersionAtom(CapabilityAtom name)
{
    if (name >= CapabilityAtom::_spirv_1_0 && name <= getLatestSpirvAtom())
        return true;
    if (name >= CapabilityAtom::metallib_2_3 && name <= getLatestMetalAtom())
        return true;
    return false;
}

bool isSpirvExtensionAtom(CapabilityAtom name)
{
    return _getInfo(name).name.startsWith("SPV_");
}

bool lookupCapabilityName(const UnownedStringSlice& str, CapabilityName& value);

CapabilityName findCapabilityName(UnownedStringSlice const& name)
{
    CapabilityName result{};
    if (!lookupCapabilityName(name, result))
        return CapabilityName::Invalid;
    return result;
}

bool isInternalCapabilityName(CapabilityName name)
{
    SLANG_ASSERT(_getInfo(name).name != nullptr);
    return _getInfo(name).name.startsWith("_");
}

CapabilityAtom getLatestSpirvAtom()
{
    static CapabilityAtom result = CapabilityAtom::Invalid;
    if (result == CapabilityAtom::Invalid)
    {
        CapabilitySet latestSpirvCapSet = CapabilitySet(CapabilityName::_spirv_latest);
        auto latestSpirvCapSetElements =
            latestSpirvCapSet.getAtomSets()->getElements<CapabilityAtom>();
        result = asAtom(
            latestSpirvCapSetElements[latestSpirvCapSetElements.getCount() - 2]); //-1 gets shader
                                                                                  // stage
    }
    return result;
}

CapabilityAtom getLatestMetalAtom()
{
    static CapabilityAtom result = CapabilityAtom::Invalid;
    if (result == CapabilityAtom::Invalid)
    {
        CapabilitySet latestMetalCapSet = CapabilitySet(CapabilityName::metallib_latest);
        auto latestMetalCapSetElements =
            latestMetalCapSet.getAtomSets()->getElements<CapabilityAtom>();
        result = asAtom(
            latestMetalCapSetElements[latestMetalCapSetElements.getCount() - 2]); //-1 gets shader
                                                                                  // stage
    }
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
        if (cur->contains((UInt)base))
            return true;
    }

    return false;
}

// CapabilityAtomSet

CapabilityAtomSet CapabilityAtomSet::newSetWithoutImpliedAtoms() const
{
    // plan is to add all atoms which is impled (=>) another atom.
    // Implying an atom appears in the form of atom1=>atom2 or atom2=>atom1.
    Dictionary<CapabilityAtom, bool> candidateForSimplifiedList;
    CapabilityAtomSet simplifiedSet;
    for (auto atom1UInt : *this)
    {
        CapabilityAtom atom1 = (CapabilityAtom)atom1UInt;
        if (!candidateForSimplifiedList.addIfNotExists(atom1, true) &&
            candidateForSimplifiedList[atom1] == false)
            continue;

        for (auto atom2UInt : *this)
        {
            if (atom1UInt == atom2UInt)
                continue;

            CapabilityAtom atom2 = (CapabilityAtom)atom2UInt;
            if (!candidateForSimplifiedList.addIfNotExists(atom2, true) &&
                candidateForSimplifiedList[atom2] == false)
                continue;

            auto atomInfo1 = _getInfo(atom1).canonicalRepresentation;
            auto atomInfo2 = _getInfo(atom2).canonicalRepresentation;
            for (auto atomSet1 : atomInfo1)
            {
                for (auto atomSet2 : atomInfo2)
                {
                    if (atomSet1->contains(*atomSet2))
                    {
                        candidateForSimplifiedList[atom2] = false;
                        continue;
                    }
                    else if (atomSet2->contains(*atomSet1))
                    {
                        candidateForSimplifiedList[atom1] = false;
                        continue;
                    }
                }
            }
        }
    }
    for (auto i : candidateForSimplifiedList)
        if (i.second)
            simplifiedSet.add((UInt)i.first);
    return simplifiedSet;
}

//// CapabiltySet

CapabilityAtomSet getTargetAtomsInSet(const CapabilitySet& set)
{
    CapabilityAtomSet out;
    for (auto i : set.getCapabilityTargetSets())
        out.add((UInt)i.first);
    return out;
}

CapabilityAtomSet getStageAtomsInSet(const CapabilityTargetSet& set)
{
    CapabilityAtomSet out;
    for (auto i : set.getShaderStageSets())
        out.add((UInt)i.first);
    return out;
}

CapabilityAtom getTargetAtomInSet(const CapabilityAtomSet& atomSet)
{
    auto targetSet = getAtomSetOfTargets();
    CapabilityAtomSet out;
    CapabilityAtomSet::calcIntersection(out, targetSet, atomSet);
    auto iter = out.begin();
    if (iter == out.end())
        return CapabilityAtom::Invalid;
    return asAtom(*iter);
}

CapabilityAtom getStageAtomInSet(const CapabilityAtomSet& atomSet)
{
    auto stageSet = getAtomSetOfStages();
    CapabilityAtomSet out;
    CapabilityAtomSet::calcIntersection(out, stageSet, atomSet);
    auto iter = out.begin();
    if (iter == out.end())
        return CapabilityAtom::Invalid;
    return asAtom(*iter);
}

template<CapabilityName keyholeAtomToPermuteWith>
void CapabilitySet::addPermutationsOfConjunctionForEachInContainer(
    CapabilityAtomSet& setToPermutate,
    const CapabilityAtomSet& elementsToPermutateWith,
    CapabilityAtom knownTargetAtom,
    CapabilityAtom knownStageAtom)
{
    SLANG_UNUSED(knownTargetAtom);
    SLANG_UNUSED(knownStageAtom);
    for (auto i : elementsToPermutateWith)
    {
        CapabilityName atom = (CapabilityName)i;
        CapabilityAtomSet conjunctionPermutation = setToPermutate;
        auto targetInfo = _getInfo(atom);
        conjunctionPermutation.add(*targetInfo.canonicalRepresentation[0]);

        if constexpr (keyholeAtomToPermuteWith == CapabilityName::target)
        {
            addConjunction(conjunctionPermutation, asAtom(atom), knownStageAtom);
        }
        else if constexpr (keyholeAtomToPermuteWith == CapabilityName::stage)
        {
            addConjunction(conjunctionPermutation, knownTargetAtom, asAtom(atom));
        }
        else
        {
            addConjunction(conjunctionPermutation, knownTargetAtom, knownStageAtom);
        }
    }
}

void CapabilitySet::addConjunction(
    CapabilityAtomSet conjunction,
    CapabilityAtom knownTargetAtom,
    CapabilityAtom knownStageAtom)
{
    if (knownTargetAtom == CapabilityAtom::Invalid)
    {
        knownTargetAtom = getTargetAtomInSet(conjunction);
        // if no target in conjunction, add a permutation of the conjunction with every target
        if (knownTargetAtom == CapabilityAtom::Invalid)
        {
            addPermutationsOfConjunctionForEachInContainer<CapabilityName::target>(
                conjunction,
                getAtomSetOfTargets(),
                CapabilityAtom::Invalid,
                getStageAtomInSet(conjunction));
            return;
        }
    }
    auto& capabilitySetToTargetSet = m_targetSets[knownTargetAtom];
    capabilitySetToTargetSet.target = knownTargetAtom;

    if (knownStageAtom == CapabilityAtom::Invalid)
    {
        knownStageAtom = getStageAtomInSet(conjunction);
        // if no target in conjunction, add a permutation of the conjunction with every stage
        if (knownStageAtom == CapabilityAtom::Invalid)
        {
            capabilitySetToTargetSet.shaderStageSets.reserve(kCapabilityStageCount);
            addPermutationsOfConjunctionForEachInContainer<CapabilityName::stage>(
                conjunction,
                getAtomSetOfStages(),
                knownTargetAtom,
                CapabilityAtom::Invalid);
            return;
        }
    }
    auto& targetSetToStageSet = capabilitySetToTargetSet.shaderStageSets[knownStageAtom];
    targetSetToStageSet.stage = knownStageAtom;
    targetSetToStageSet.addNewSet(std::move(conjunction));
}

void CapabilitySet::addUnexpandedCapabilites(CapabilityName atom)
{
    auto info = _getInfo(atom);
    for (const auto cr : info.canonicalRepresentation)
        addConjunction(*cr, CapabilityAtom::Invalid, CapabilityAtom::Invalid);
}

CapabilityAtom CapabilitySet::getUniquelyImpliedStageAtom() const
{
    CapabilityAtom result = CapabilityAtom::Invalid;
    for (auto& targetKV : m_targetSets)
    {
        if (targetKV.second.shaderStageSets.getCount() == 1)
        {
            auto thisStage = targetKV.second.shaderStageSets.begin()->first;
            if (result == CapabilityAtom::Invalid)
                result = thisStage;
            else if (result != thisStage)
                return CapabilityAtom::Invalid;
        }
    }
    return result;
}

CapabilitySet::CapabilitySet() {}

CapabilitySet::CapabilitySet(CapabilitySetVal const* other)
{
    if (other)
    {
        *this = other->thaw();
    }
}

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

    // Incompatible means there are 0 intersecting abstract nodes from sets in `other` with sets in
    // `this`
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

// Implication depends heavily on context as per the `ImpliesFlags`.
CapabilitySet::ImpliesReturnFlags CapabilitySet::_implies(
    CapabilitySet const& otherSet,
    ImpliesFlags flags) const
{
    // By default (`ImpliesFlags::None`): x implies (c | d) only if (x implies c) and (x implies d).

    bool onlyRequireSingleImply = ((int)flags & (int)ImpliesFlags::OnlyRequireASingleValidImply);
    bool cannotHaveMoreTargetAndStageSets =
        ((int)flags & (int)ImpliesFlags::CannotHaveMoreTargetAndStageSets);
    bool canHaveSubsetOfTargetAndStageSets =
        ((int)flags & (int)ImpliesFlags::CanHaveSubsetOfTargetAndStageSets);

    int flagsCollected = (int)CapabilitySet::ImpliesReturnFlags::NotImplied;

    if (otherSet.isEmpty())
        return CapabilitySet::ImpliesReturnFlags::Implied;

    // If empty, and the other is not empty, it does not matter what flags are used,
    // `this` is considered to "not imply" another set. This is important since
    // `T.join(U)` causes `T == U`.
    if (this->isEmpty())
        return CapabilitySet::ImpliesReturnFlags::NotImplied;

    if (cannotHaveMoreTargetAndStageSets &&
        this->getCapabilityTargetSets().getCount() > otherSet.getCapabilityTargetSets().getCount())
    {
        return CapabilitySet::ImpliesReturnFlags::NotImplied;
    }

    for (const auto& otherTargetPair : otherSet.m_targetSets)
    {
        auto thisTarget = this->m_targetSets.tryGetValue(otherTargetPair.first);
        const auto& otherTarget = otherTargetPair.second;
        if (!thisTarget)
        {
            if (onlyRequireSingleImply)
                continue;

            if (canHaveSubsetOfTargetAndStageSets)
                continue;
            // 'this' lacks a target 'other' has.
            return CapabilitySet::ImpliesReturnFlags::NotImplied;
        }

        if (cannotHaveMoreTargetAndStageSets && thisTarget->getShaderStageSets().getCount() >
                                                    otherTarget.getShaderStageSets().getCount())
        {
            return CapabilitySet::ImpliesReturnFlags::NotImplied;
        }

        for (const auto& otherStagePair : otherTarget.getShaderStageSets())
        {
            auto thisStage = thisTarget->shaderStageSets.tryGetValue(otherStagePair.first);
            const auto& otherStage = otherStagePair.second;
            if (!thisStage)
            {
                if (onlyRequireSingleImply)
                    continue;

                if (canHaveSubsetOfTargetAndStageSets)
                    continue;
                // 'this' lacks a stage 'other' has.
                return CapabilitySet::ImpliesReturnFlags::NotImplied;
            }

            // all stage sets that are in 'other' must be contained by 'this'
            if (thisStage->atomSet)
            {
                auto& thisStageSet = thisStage->atomSet.value();
                if (otherStage.atomSet)
                {
                    auto contained = thisStageSet.contains(otherStage.atomSet.value());
                    if (!onlyRequireSingleImply && !contained)
                    {
                        return CapabilitySet::ImpliesReturnFlags::NotImplied;
                    }
                    else if (onlyRequireSingleImply && contained)
                    {
                        return CapabilitySet::ImpliesReturnFlags::Implied;
                    }
                }
            }
        }
    }
    if (!onlyRequireSingleImply)
        flagsCollected |= (int)CapabilitySet::ImpliesReturnFlags::Implied;

    return (CapabilitySet::ImpliesReturnFlags)flagsCollected;
}

bool CapabilitySet::implies(CapabilitySet const& other) const
{
    return (int)_implies(other, ImpliesFlags::None) &
           (int)CapabilitySet::ImpliesReturnFlags::Implied;
}

CapabilitySet::ImpliesReturnFlags CapabilitySet::atLeastOneSetImpliedInOther(
    CapabilitySet const& other) const
{
    return _implies(other, ImpliesFlags::OnlyRequireASingleValidImply);
}

bool CapabilitySet::joinWithOtherWillChangeThis(CapabilitySet const& other) const
{
    return !(
        (int)_implies(other, ImpliesFlags::CannotHaveMoreTargetAndStageSets) &
        (int)CapabilitySet::ImpliesReturnFlags::Implied);
}

void CapabilityTargetSet::unionWith(const CapabilityTargetSet& other)
{
    for (auto otherStageSet : other.shaderStageSets)
    {
        auto& thisStageSet = this->shaderStageSets[otherStageSet.first];
        thisStageSet.stage = otherStageSet.first;

        if (!thisStageSet.atomSet)
            thisStageSet.atomSet = otherStageSet.second.atomSet;
        else if (otherStageSet.second.atomSet)
            thisStageSet.atomSet->unionWith(*otherStageSet.second.atomSet);
    }
}

void CapabilityTargetSet::unionWith(const CapabilityTargetSetVal& other)
{
    // Iterate through all stages in the CapabilityTargetSetVal
    for (Index i = 0; i < other.getStageSetCount(); i++)
    {
        auto otherStageSetVal = other.getStageSet(i);
        auto stageAtom = otherStageSetVal->getStage();

        auto& thisStageSet = this->shaderStageSets[stageAtom];
        thisStageSet.stage = stageAtom;

        // Union the atom sets if they exist
        auto otherAtomSetVal = otherStageSetVal->getAtomSet();
        if (otherAtomSetVal)
        {
            if (!thisStageSet.atomSet)
            {
                // Convert UIntSetVal to UIntSet
                thisStageSet.atomSet = otherAtomSetVal->toUIntSet();
            }
            else
            {
                // Union with the existing UIntSet
                thisStageSet.atomSet->unionWith(*otherAtomSetVal);
            }
        }
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

void CapabilitySet::unionWith(const CapabilitySetVal* other)
{
    if (!other)
        return;

    if (this->isInvalid() || other->isInvalid())
        return;

    // Iterate through all targets in the CapabilitySetVal
    for (Index i = 0; i < other->getTargetSetCount(); i++)
    {
        auto otherTargetSetVal = other->getTargetSet(i);
        auto targetAtom = otherTargetSetVal->getTarget();

        CapabilityTargetSet& thisTargetSet = this->m_targetSets[targetAtom];
        thisTargetSet.target = targetAtom;

        // Estimate stage count for reservation
        thisTargetSet.shaderStageSets.reserve(otherTargetSetVal->getStageSetCount());
        thisTargetSet.unionWith(*otherTargetSetVal);
    }
}

/// Join sets, but:
/// 1. do not destroy target set's which are incompatible with `other` (destroying shaderStageSets
/// is fine)
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

void CapabilitySet::nonDestructiveJoin(const CapabilitySetVal* other)
{
    // Treat null pointer as empty set
    if (!other)
        return;

    if (this->isInvalid() || other->isInvalid())
        return;

    if (this->isEmpty())
    {
        *this = CapabilitySet(other);
        return;
    }

    for (auto& thisTargetSet : this->m_targetSets)
    {
        thisTargetSet.second.tryJoin(*other);
    }
}

bool CapabilitySet::operator==(CapabilitySet const& that) const
{
    // Check if both sets have the same number of target sets
    if (this->m_targetSets.getCount() != that.m_targetSets.getCount())
        return false;

    for (auto set : this->m_targetSets)
    {
        auto thatSet = that.m_targetSets.tryGetValue(set.first);
        if (!thatSet)
            return false;

        // Check if both target sets have the same number of stage sets
        if (set.second.shaderStageSets.getCount() != thatSet->shaderStageSets.getCount())
            return false;

        for (auto stageSet : set.second.shaderStageSets)
        {
            auto thatStageSet = thatSet->shaderStageSets.tryGetValue(stageSet.first);
            if (!thatStageSet)
                return false;
            if (stageSet.second.atomSet != thatStageSet->atomSet)
                return false;
        }
    }
    return true;
}

HashCode64 CapabilityStageSet::getHashCode() const
{
    HashCode64 stageHash = ::Slang::getHashCode(static_cast<uint64_t>(stage));
    HashCode64 atomSetHash = atomSet ? atomSet->getHashCode() : 0;
    return combineHash(stageHash, atomSetHash);
}

HashCode64 CapabilityTargetSet::getHashCode() const
{
    HashCode64 targetHash = ::Slang::getHashCode(static_cast<uint64_t>(target));
    // The key and the "target" member of the target sets are always the same,
    // so we can just hash the values
    auto stageSetsView = shaderStageSets | std::views::values;
    HashCode64 stageSetsHash = symmetricHash(stageSetsView.begin(), stageSetsView.end());
    return combineHash(targetHash, stageSetsHash);
}

HashCode64 CapabilitySet::getHashCode() const
{
    // The key and the "target" member of the target sets are always the same,
    // so we can just hash the values
    auto targetSetsView = m_targetSets | std::views::values;
    return symmetricHash(targetSetsView.begin(), targetSetsView.end());
}

CapabilitySet CapabilitySet::getTargetsThisHasButOtherDoesNot(const CapabilitySet& other)
{
    CapabilitySet newSet{};
    for (auto& i : this->m_targetSets)
    {
        if (other.m_targetSets.tryGetValue(i.first))
            continue;

        newSet.m_targetSets[i.first] = i.second;
    }
    return newSet;
}

CapabilitySet CapabilitySet::getStagesThisHasButOtherDoesNot(const CapabilitySet& other)
{
    CapabilitySet newSet{};
    for (auto& i : this->m_targetSets)
    {
        if (auto otherTarget = other.m_targetSets.tryGetValue(i.first))
        {
            auto& thisTarget = m_targetSets[i.first];
            for (auto& stage : thisTarget.shaderStageSets)
            {
                if (otherTarget->shaderStageSets.containsKey(stage.first))
                    continue;
                newSet.m_targetSets[i.first].shaderStageSets[stage.first] = stage.second;
            }
        }
    }
    return newSet;
}

bool CapabilityStageSet::tryJoin(const CapabilityTargetSet& other)
{
    const CapabilityStageSet* otherStageSet = other.shaderStageSets.tryGetValue(this->stage);
    if (!otherStageSet)
        return false;

    // should not exceed far beyond 2*2 or 1*1 elements
    if (otherStageSet->atomSet && this->atomSet)
        this->atomSet->add(*otherStageSet->atomSet);

    return true;
}

bool CapabilityStageSet::tryJoin(const CapabilityTargetSetVal& other)
{
    // Find the corresponding stage set in the immutable target set
    CapabilityStageSetVal* otherStageSet = other.findStageSet(this->stage);
    if (!otherStageSet)
        return false;

    // Join the atom sets if both exist
    if (otherStageSet->getAtomSet() && this->atomSet)
    {
        UIntSetVal* otherAtomSet = otherStageSet->getAtomSet();
        this->atomSet->unionWith(*otherAtomSet);
    }

    return true;
}

bool CapabilityStageSet::compatibleMerge(const CapabilityStageSet& stageSet)
{
    CapabilityStageSet tmp = *this;
    tmp.atomSet->unionWith(stageSet.atomSet.value());

    // if 'this' U stageSet == stageSet, then 'this' is a subset of stageSet
    // do need to do anything
    if (tmp.atomSet == stageSet.atomSet)
    {
        return true;
    }
    else if (tmp.atomSet == this->atomSet)
    {
        // 'stageSet' is a subset of 'this', so we can just copy over
        atomSet = stageSet.atomSet;
        return true;
    }
    else
    {
        // neither is a subset of the other, so we can not perform subset join
        return false;
    }
}

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

bool CapabilityTargetSet::tryJoin(const CapabilitySetVal& other)
{
    // Find the corresponding target set in the immutable capability set
    CapabilityTargetSetVal* otherTargetSet = nullptr;
    for (Index i = 0; i < other.getTargetSetCount(); i++)
    {
        CapabilityTargetSetVal* targetSet = other.getTargetSet(i);
        if (targetSet->getTarget() == this->target)
        {
            otherTargetSet = targetSet;
            break;
        }
    }

    if (!otherTargetSet)
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

/// Are the two CapabilityTargetSet equal?
bool CapabilityTargetSet::operator==(CapabilityTargetSet const& that) const
{
    for (auto stageSet : shaderStageSets)
    {
        auto thatStageSet = that.shaderStageSets.tryGetValue(stageSet.first);
        if (!thatStageSet)
            return false;
        if (stageSet.second.atomSet != thatStageSet->atomSet)
            return false;
    }
    return true;
}

/// Perform a compatibleMerge on the given `targetSet` with `this`.
/// This function treats the whole target set as a single bit mask, and perform the
/// operation on it.
/// Definition of compatibleMerge is:
/// # compatibleMerge(A, B) =
/// ⎧ B   if A = ∅
/// ⎪ A   if B = ∅
/// ⎪ A   if A ⊆ B
/// ⎪ B   if B ⊆ A
/// ⎩ ∅   otherwise, and function will return false.
/// For example:
/// if A = {spirv, ext_X}, and B = {spirv, ext_X, ext_Y}, compatibleMerge(A, B) = A
/// if A = {spirv, ext_X, ext_Y}, and B = {spirv, ext_X}, compatibleMerge(A, B) = B
/// if A = {spirv, ext_X}, and B = {spirv, ext_Y}, compatibleMerge(A, B) = ∅
/// If target A doesn't exist, then add target B directly.
bool CapabilityTargetSet::compatibleMerge(const CapabilityTargetSet& targetSet)
{
    CapabilityTargetSet tmp = *this;
    tmp.unionWith(targetSet);

    // if this U targetSet == targetSet, then 'this' is a subset of targetSet
    // do need to do anything
    if (tmp == targetSet)
    {
        return true;
    }
    else if (tmp == *this)
    {
        // 'targetSet' is a subset of 'this', so we can just copy over
        shaderStageSets = targetSet.getShaderStageSets();
        return true;
    }
    else
    {
        // neither is a subset of the other, so we can not perform subset join
        return false;
    }
}

/// Similar to compatibleMerge for CapabilityTargetSet, but this overload perform the operation
/// on a finer granularity, we perform the operation on a specific stage of a target
bool CapabilityTargetSet::compatibleMerge(const CapabilityStageSet& stageSet)
{
    if (auto existStage = shaderStageSets.tryGetValue(stageSet.stage))
    {
        return existStage->compatibleMerge(stageSet);
    }
    else
    {
        shaderStageSets.add(stageSet.stage, stageSet);
        return true;
    }
}

CapabilitySet& CapabilitySet::join(const CapabilitySet& other)
{
    if (this->isEmpty() || other.isInvalid())
    {
        *this = other;
        return *this;
    }
    if (this->isInvalid())
        return *this;
    if (other.isEmpty())
        return *this;

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
    return *this;
}

CapabilitySet& CapabilitySet::join(const CapabilitySetVal* other)
{
    // Treat null pointer as empty set
    if (!other)
        return *this;

    if (this->isEmpty() || other->isInvalid())
    {
        *this = CapabilitySet(other);
        return *this;
    }
    if (this->isInvalid())
        return *this;
    if (other->isEmpty())
        return *this;

    List<CapabilityAtom> destroySet;
    destroySet.reserve(this->m_targetSets.getCount());
    for (auto& thisTargetSet : this->m_targetSets)
    {
        if (!thisTargetSet.second.tryJoin(*other))
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
    return *this;
}

static uint32_t _calcAtomListDifferenceScore(
    List<CapabilityAtom> const& thisList,
    List<CapabilityAtom> const& thatList)
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
        if (thisIndex == thisCount)
            break;
        if (thatIndex == thatCount)
            break;

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
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4702)
#endif
bool CapabilitySet::isBetterForTarget(
    CapabilitySet const& that,
    CapabilitySet const& targetCaps,
    bool& isEqual) const
{
    if (this->isEmpty() && (that.isEmpty() || that.isInvalid()))
    {
        if (this->isEmpty() && that.isEmpty())
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
            auto thisStageSets =
                thisTarget->shaderStageSets.tryGetValue(shaderStageSetsWeNeed.first);
            if (!thisStageSets)
                return false;
            auto thatStageSets =
                thatTarget->shaderStageSets.tryGetValue(shaderStageSetsWeNeed.first);
            if (!thatStageSets)
                return true;

            // We want the smallest (most specialized) set which is still contained by this/that.
            // This means:
            // 1. target.contains(this/that)
            // 2. choose smallest super set
            // 3. rank each super set and their atoms, choose the smallest rank'd set (most
            // specialized)
            if (shaderStageSetsWeNeed.second.atomSet)
            {
                auto& shaderStageSetWeNeed = shaderStageSetsWeNeed.second.atomSet.value();

                CapabilityAtomSet tmp_set{};
                Index tmpCount = 0;
                CapabilityAtomSet thisSet{};
                Index thisSetCount = 0;
                CapabilityAtomSet thatSet{};
                Index thatSetCount = 0;

                // subtraction of the set we want gets us the "elements which 'targetSet' has but
                // `this/that` is less specialized for"
                if (thisStageSets->atomSet)
                {
                    auto& thisStageSet = thisStageSets->atomSet.value();
                    // if `thisStageSet` is more specialized than the target, `thisStageSet` should
                    // not be a candidate
                    if (thisStageSet == shaderStageSetWeNeed)
                        return true;
                    if (shaderStageSetWeNeed.contains(thisStageSet))
                    {
                        CapabilityAtomSet::calcSubtract(
                            tmp_set,
                            shaderStageSetWeNeed,
                            thisStageSet);
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
                        CapabilityAtomSet::calcSubtract(
                            tmp_set,
                            shaderStageSetWeNeed,
                            thatStageSet);
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

                // empty means no candidate
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
                auto shaderStageSetWeNeedElements =
                    shaderStageSetWeNeed.getElements<CapabilityAtom>();

                auto thisDiffScore =
                    _calcAtomListDifferenceScore(thisSetElements, shaderStageSetWeNeedElements);
                auto thatDiffScore =
                    _calcAtomListDifferenceScore(thisSetElements, shaderStageSetWeNeedElements);

                return thisDiffScore < thatDiffScore;
            }
        }
    }
    return true;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

CapabilitySet::AtomSets::Iterator CapabilitySet::getAtomSets() const
{
    return CapabilitySet::AtomSets::Iterator(&this->getCapabilityTargetSets()).begin();
}

void CapabilitySet::checkCapabilityRequirement(
    CheckCapabilityRequirementOptions options,
    CapabilitySet const& available,
    CapabilitySet const& required,
    CapabilityAtomSet& outFailedAvailableSet,
    CheckCapabilityRequirementResult& result)
{
    // 'required' capabilities x are met by 'available' disjoint capabilities (a | b) iff
    // both 'a' satisfies x and 'b' satisfies x.
    // If we have a caller function F() decorated with:
    //     [require(hlsl, _sm_6_3)] [require(spirv, _spv_ray_tracing)] void F() { g(); }
    // We'd better make sure that `g()` can be compiled with both (hlsl+_sm_6_3) and
    // (spirv+_spv_ray_tracing) capability sets. In this method, F()'s capability declaration is
    // represented by `available`, and g()'s capability is represented by `required`. We will check
    // that for every capability conjunction X of F(), there is a capability conjunction Y in g()
    // such that X implies Y.
    //

    // If empty, all capabilities are supported.
    // Either, we require no capabilities (return true)
    // or we have no capability requirements (return true)
    if (required.isEmpty() || available.isEmpty())
    {
        result = CheckCapabilityRequirementResult::AvailableIsASuperSetToRequired;
        return;
    }

    // invalid isn't a fail because the capabilities already threw an error.
    if (required.isInvalid())
    {
        outFailedAvailableSet.add((UInt)CapabilityAtom::Invalid);
        result = CheckCapabilityRequirementResult::AvailableIsASuperSetToRequired;
        return;
    }

    auto availableTargetSets = available.getCapabilityTargetSets();
    auto requiredTargetSets = required.getCapabilityTargetSets();
    if (options == CheckCapabilityRequirementOptions::MustHaveEqualAbstractAtoms)
    {
        // If we have a mismatch in capability-target count we clearly have a
        // mismatch and will fail
        auto availableTargetSetsCount = availableTargetSets.getCount();
        auto requiredTargetSetsCount = requiredTargetSets.getCount();
        if (availableTargetSetsCount != requiredTargetSetsCount)
        {
            auto availableTargets = getTargetAtomsInSet(available);
            auto requiredTargets = getTargetAtomsInSet(required);

            if (requiredTargetSetsCount > availableTargetSetsCount)
            {
                result = CheckCapabilityRequirementResult::AvailableIsNotASuperSetToRequired;
                requiredTargets.subtractWith((UIntSet)availableTargets);
                outFailedAvailableSet.add((UIntSet)requiredTargets);
            }
            else
            {
                result = CheckCapabilityRequirementResult::RequiredIsMissingAbstractAtoms;
                availableTargets.subtractWith((UIntSet)requiredTargets);
                outFailedAvailableSet.add((UIntSet)availableTargets);
            }
            return;
        }
    }

    // if all sets in `available` are not a superset to `required` then we have an
    // error.
    for (auto& availableTarget : availableTargetSets)
    {
        auto reqTarget = requiredTargetSets.tryGetValue(availableTarget.first);
        if (!reqTarget)
        {
            outFailedAvailableSet.add((UInt)availableTarget.first);
            result = CheckCapabilityRequirementResult::RequiredIsMissingAbstractAtoms;
            return;
        }

        if (options == CheckCapabilityRequirementOptions::MustHaveEqualAbstractAtoms)
        {
            // If we have a mismatch in capability-stage count we clearly have a
            // mismatch and will fail
            auto availableStageSetsCount = availableTarget.second.getShaderStageSets().getCount();
            auto requiredStageSetsCount = reqTarget->getShaderStageSets().getCount();
            if (availableStageSetsCount != requiredStageSetsCount)
            {
                auto availableStages = getStageAtomsInSet(availableTarget.second);
                auto requiredStages = getStageAtomsInSet(*reqTarget);

                if (requiredStageSetsCount > availableStageSetsCount)
                {
                    result = CheckCapabilityRequirementResult::AvailableIsNotASuperSetToRequired;
                    requiredStages.subtractWith((UIntSet)availableStages);
                    outFailedAvailableSet.add((UIntSet)requiredStages);
                }
                else
                {
                    result = CheckCapabilityRequirementResult::RequiredIsMissingAbstractAtoms;
                    availableStages.subtractWith((UIntSet)requiredStages);
                    outFailedAvailableSet.add((UIntSet)availableStages);
                }
                return;
            }
        }

        for (auto& availableStage : availableTarget.second.getShaderStageSets())
        {
            auto reqStage = reqTarget->getShaderStageSets().tryGetValue(availableStage.first);
            if (!reqStage)
            {
                outFailedAvailableSet.add((UInt)availableStage.first);
                result = CheckCapabilityRequirementResult::RequiredIsMissingAbstractAtoms;
                return;
            }

            const CapabilityAtomSet* lastBadStage = nullptr;
            if (availableStage.second.atomSet)
            {
                const auto& availableStageSet = availableStage.second.atomSet.value();
                lastBadStage = nullptr;
                if (reqStage->atomSet)
                {
                    const auto& reqStageSet = reqStage->atomSet.value();
                    if (availableStageSet.contains(reqStageSet))
                        continue;
                    else
                        lastBadStage = &reqStageSet;
                }
                if (lastBadStage)
                {
                    // get missing atoms
                    CapabilityAtomSet::calcSubtract(
                        outFailedAvailableSet,
                        *lastBadStage,
                        availableStageSet);

                    // Not a failiure if nothing is missing
                    if (outFailedAvailableSet.isEmpty())
                        continue;
                    result = CheckCapabilityRequirementResult::AvailableIsNotASuperSetToRequired;
                    return;
                }
            }
        }
    }

    result = CheckCapabilityRequirementResult::AvailableIsASuperSetToRequired;
    return;
}

/// Converts spirv version atom to the glsl_spirv equivlent. If not possible, Invalid is returned
inline CapabilityName maybeConvertSpirvVersionToGlslSpirvVersion(CapabilityName& atom)
{
    if (atom >= CapabilityName::_spirv_1_0 && asAtom(atom) <= getLatestSpirvAtom())
    {
        return (CapabilityName)((Int)CapabilityName::glsl_spirv_1_0 +
                                ((Int)atom - (Int)CapabilityName::_spirv_1_0));
    }
    return CapabilityName::Invalid;
}

void CapabilitySet::addSpirvVersionFromOtherAsGlslSpirvVersion(CapabilitySet& other)
{
    if (auto* otherTargetSet = other.m_targetSets.tryGetValue(CapabilityAtom::spirv))
    {
        auto* thisTargetSet = m_targetSets.tryGetValue(CapabilityAtom::glsl);
        if (!thisTargetSet)
            return;

        for (auto& otherStageSet : otherTargetSet->shaderStageSets)
        {
            if (!otherStageSet.second.atomSet)
                continue;

            auto* thisStageSet = thisTargetSet->shaderStageSets.tryGetValue(otherStageSet.first);
            if (!thisStageSet || !thisStageSet->atomSet)
                continue;

            CapabilityAtomSet::Iterator otherAtom = otherStageSet.second.atomSet->begin();
            while (otherAtom != otherStageSet.second.atomSet->end())
            {
                otherAtom++;
                auto otherAtomName = (CapabilityName)*otherAtom;
                if (otherAtomName > (CapabilityName)getLatestSpirvAtom())
                {
                    otherAtom = otherStageSet.second.atomSet->end();
                    continue;
                }
                auto maybeConvertedSpirvVersionAtom =
                    maybeConvertSpirvVersionToGlslSpirvVersion(otherAtomName);
                if (maybeConvertedSpirvVersionAtom == CapabilityName::Invalid)
                    continue;

                thisStageSet->atomSet->add((UInt)maybeConvertedSpirvVersionAtom);
            }
        }
    }
}

UnownedStringSlice capabilityNameToStringWithoutPrefix(CapabilityName capabilityName)
{
    auto name = capabilityNameToString(capabilityName);
    if (name.startsWith("_"))
        return name.tail(1);
    return name;
}

void printDiagnosticArg(StringBuilder& sb, const CapabilityAtomSet& atomSet)
{
    bool isFirst = true;
    for (auto atom : atomSet.newSetWithoutImpliedAtoms())
    {
        CapabilityName formattedAtom = (CapabilityName)atom;
        if (!isFirst)
            sb << " + ";
        printDiagnosticArg(sb, formattedAtom);
        isFirst = false;
    }
}

// Collection of stages which have same atom sets to compress reprisentation of atom and stage per
// target
struct CompressedCapabilitySet
{
    /// Collection of stages which have same atom sets to compress reprisentation of atom and stage:
    /// {vertex/fragment, ... }
    struct StageAndAtomSet
    {
        CapabilityAtomSet stages;
        CapabilityAtomSet atomsWithoutStage;
    };

    auto begin() { return atomSetsOfTargets.begin(); }

    /// Compress 1 capabilitySet into a reprisentation which merges stages that share all of their
    /// atoms for printing.
    Dictionary<CapabilityAtom, List<StageAndAtomSet>> atomSetsOfTargets;
    CompressedCapabilitySet(const CapabilitySet& capabilitySet)
    {
        for (auto& atomSet : capabilitySet.getAtomSets())
        {
            auto target = getTargetAtomInSet(atomSet);

            auto stageInSetAtom = getStageAtomInSet(atomSet);
            CapabilityAtomSet stageInSet;
            stageInSet.add((UInt)stageInSetAtom);

            CapabilityAtomSet atomsWithoutStage;
            CapabilityAtomSet::calcSubtract(atomsWithoutStage, atomSet, stageInSet);
            if (!atomSetsOfTargets.containsKey(target))
            {
                atomSetsOfTargets[target].add({stageInSet, atomsWithoutStage});
                continue;
            }

            // try to find an equivlent atom set by iterating all of the same
            // `atomSetsOfTarget[target]` and merge these 2 together.
            auto& atomSetsOfTarget = atomSetsOfTargets[target];
            for (auto& i : atomSetsOfTarget)
            {
                if (i.atomsWithoutStage.contains(atomsWithoutStage) &&
                    atomsWithoutStage.contains(i.atomsWithoutStage))
                {
                    i.stages.add((UInt)stageInSetAtom);
                }
            }
        }
        for (auto& targetSets : atomSetsOfTargets)
            for (auto& targetSet : targetSets.second)
                targetSet.atomsWithoutStage =
                    targetSet.atomsWithoutStage.newSetWithoutImpliedAtoms();
    }
};

void printDiagnosticArg(StringBuilder& sb, const CompressedCapabilitySet& capabilitySet)
{
    ////Secondly we will print our new list of atomSet's.
    sb << "{";
    bool firstSet = true;
    for (auto targetSets : capabilitySet.atomSetsOfTargets)
    {
        if (!firstSet)
            sb << " || ";
        for (auto targetSet : targetSets.second)
        {
            bool firstStage = true;
            for (auto stageAtom : targetSet.stages)
            {
                if (!firstStage)
                    sb << "/";
                printDiagnosticArg(sb, (CapabilityName)stageAtom);
                firstStage = false;
            }
            for (auto atom : targetSet.atomsWithoutStage)
            {
                sb << " + ";
                printDiagnosticArg(sb, (CapabilityName)atom);
            }
        }
        firstSet = false;
    }
    sb << "}";
}

void printDiagnosticArg(StringBuilder& sb, const CapabilitySet& capabilitySet)
{
    // Firstly we will compress the printing of capabilities such that any atomSet
    // with different abstract atoms but equal non-abstract atoms will be bundled together.
    if (capabilitySet.isInvalid() || capabilitySet.isEmpty())
    {
        sb << "{}";
        return;
    }
    printDiagnosticArg(sb, CompressedCapabilitySet(capabilitySet));
}

void printDiagnosticArg(StringBuilder& sb, CapabilityAtom atom)
{
    printDiagnosticArg(sb, (CapabilityName)atom);
}

void printDiagnosticArg(StringBuilder& sb, CapabilityName name)
{
    sb << capabilityNameToStringWithoutPrefix(name);
}

void printDiagnosticArg(StringBuilder& sb, List<CapabilityAtom>& list)
{
    CapabilityAtomSet set;
    for (auto i : list)
        set.add((UInt)i);
    printDiagnosticArg(sb, set.newSetWithoutImpliedAtoms());
}

void printDiagnosticArg(StringBuilder& sb, const CapabilitySetVal* capabilitySetVal)
{
    printDiagnosticArg(sb, CapabilitySet{capabilitySetVal});
}

void CapabilityStageSetVal::_toTextOverride(StringBuilder& out)
{
    out << "CapabilityStageSetVal{";
    out << "stage=";
    printDiagnosticArg(out, getStage());
    out << ", atoms=";
    auto atomSet = getAtomSet();
    atomSet->toText(out);
    out << "}";
}

void CapabilityTargetSetVal::_toTextOverride(StringBuilder& out)
{
    out << "CapabilityTargetSetVal{";
    out << "target=";
    printDiagnosticArg(out, getTarget());
    out << ", stageSets=[";
    for (Index i = 0; i < getStageSetCount(); i++)
    {
        if (i > 0)
            out << ", ";
        auto stageSet = getStageSet(i);
        stageSet->toText(out);
    }
    out << "]}";
}

void CapabilitySetVal::_toTextOverride(StringBuilder& out)
{
    out << "CapabilitySetVal{targetSets=[";
    for (Index i = 0; i < getTargetSetCount(); i++)
    {
        if (i > 0)
            out << ", ";
        auto targetSet = getTargetSet(i);
        targetSet->toText(out);
    }
    out << "]}";
}

//
// CapabilitySetVal
//

CapabilityStageSetVal* CapabilityTargetSetVal::findStageSet(CapabilityAtom stage) const
{
    SLANG_PROFILE_CAPABILITY_SETS;
    // Linear search through sorted list - these lists are typically very short (1-4 items)
    for (Index i = 0; i < getStageSetCount(); i++)
    {
        auto stageSet = getStageSet(i);
        auto stageAtom = stageSet->getStage();
        if (stageAtom == stage)
            return stageSet;
        if (stageAtom > stage)
            break; // List is sorted, so we can stop early
    }
    return nullptr;
}

CapabilityTargetSetVal* CapabilitySetVal::findTargetSet(CapabilityAtom target) const
{
    SLANG_PROFILE_CAPABILITY_SETS;
    // Linear search through sorted list - these lists are typically very short (1-4 items)
    for (Index i = 0; i < getTargetSetCount(); i++)
    {
        auto targetSet = getTargetSet(i);
        auto targetAtom = targetSet->getTarget();
        if (targetAtom == target)
            return targetSet;
        if (targetAtom > target)
            break; // List is sorted, so we can stop early
    }
    return nullptr;
}

bool CapabilitySetVal::isInvalid() const
{
    SLANG_PROFILE_CAPABILITY_SETS;
    // Check if we have the invalid target atom
    return findTargetSet(CapabilityAtom::Invalid) != nullptr;
}

CapabilitySet CapabilitySetVal::thaw() const
{
    if (cachedThawedCapabilitySet.has_value())
        return cachedThawedCapabilitySet.value();

    SLANG_PROFILE_CAPABILITY_SETS;

    CapabilitySet result;
    if (isEmpty())
    {
        result = CapabilitySet::makeEmpty();
    }
    else if (isInvalid())
    {
        result = CapabilitySet::makeInvalid();
    }
    else
    {
        auto& targetSets = result.getCapabilityTargetSets();

        // Convert each target set
        for (Index targetIndex = 0; targetIndex < getTargetSetCount(); targetIndex++)
        {
            auto targetSetVal = getTargetSet(targetIndex);
            auto targetAtom = targetSetVal->getTarget();

            auto& targetSet = targetSets[targetAtom];
            targetSet.target = targetAtom;

            // Convert each stage set
            for (Index stageIndex = 0; stageIndex < targetSetVal->getStageSetCount(); stageIndex++)
            {
                auto stageSetVal = targetSetVal->getStageSet(stageIndex);
                auto stageAtom = stageSetVal->getStage();

                auto& stageSet = targetSet.shaderStageSets[stageAtom];
                stageSet.stage = stageAtom;

                // Convert UIntSetVal back to CapabilityAtomSet
                auto atomSetVal = stageSetVal->getAtomSet();
                if (atomSetVal)
                {
                    CapabilityAtomSet atomSet{atomSetVal->toUIntSet()};
                    stageSet.atomSet = atomSet;
                }
            }
        }
    }

    cachedThawedCapabilitySet = result;
    return result;
}

[[nodiscard]] CapabilitySetVal* CapabilitySet::freeze(ASTBuilder* astBuilder) const
{
    if (auto cached = astBuilder->m_capabilitySetCache.tryGetValue(*this))
    {
        return *cached;
    }
    SLANG_PROFILE_CAPABILITY_SETS;

    if (isEmpty())
    {
        return astBuilder->getOrCreate<CapabilitySetVal>();
    }

    if (isInvalid())
    {
        // Create invalid capability set with invalid target
        auto invalidAtomSet = astBuilder->getUIntSetVal(CapabilityAtomSet{});
        auto invalidStageSet =
            astBuilder->getOrCreate<CapabilityStageSetVal>(CapabilityAtom::Invalid, invalidAtomSet);
        auto invalidTargetSet = astBuilder->getOrCreate<CapabilityTargetSetVal>(
            CapabilityAtom::Invalid,
            invalidStageSet);
        return astBuilder->getOrCreate<CapabilitySetVal>(invalidTargetSet);
    }

    List<CapabilityTargetSetVal*> targetSetVals;

    // Convert each target set, maintaining sorted order
    List<CapabilityAtom> sortedTargets;
    sortedTargets.reserve(m_targetSets.getCount());
    for (auto& targetPair : m_targetSets)
    {
        sortedTargets.add(targetPair.first);
    }
    sortedTargets.sort([](CapabilityAtom a, CapabilityAtom b) { return (UInt)a < (UInt)b; });

    for (auto targetAtom : sortedTargets)
    {
        auto& targetSet = m_targetSets.getValue(targetAtom);
        List<CapabilityStageSetVal*> stageSetVals;

        // Convert each stage set, maintaining sorted order
        List<CapabilityAtom> sortedStages;
        sortedStages.reserve(targetSet.shaderStageSets.getCount());
        for (auto& stagePair : targetSet.shaderStageSets)
        {
            sortedStages.add(stagePair.first);
        }
        sortedStages.sort([](CapabilityAtom a, CapabilityAtom b) { return (UInt)a < (UInt)b; });

        for (auto stageAtom : sortedStages)
        {
            auto& stageSet = targetSet.shaderStageSets.getValue(stageAtom);

            // Convert CapabilityAtomSet to UIntSetVal
            UIntSetVal* atomSetVal;
            atomSetVal = astBuilder->getUIntSetVal(stageSet.atomSet.value_or(CapabilityAtomSet{}));

            auto stageSetVal =
                astBuilder->getOrCreate<CapabilityStageSetVal>(stageAtom, atomSetVal);
            stageSetVals.add(stageSetVal);
        }

        auto targetSetVal =
            astBuilder->getOrCreate<CapabilityTargetSetVal>(targetAtom, stageSetVals);
        targetSetVals.add(targetSetVal);
    }

    auto result = astBuilder->getOrCreate<CapabilitySetVal>(targetSetVals);
    result->cachedThawedCapabilitySet = *this;

    // Cache the result for future lookups
    astBuilder->m_capabilitySetCache.add(*this, result);

    return result;
}

//
// CapabilitySetVal native implementations
//

/// Generic template function for concurrent iteration over two sorted collections of the same type
/// Calls callback for all elements, passing nullptr for gaps
template<typename Collection, typename ElementType, typename KeyType, typename MatchCallback>
void concurrentIterate(
    const Collection& left,
    const Collection& right,
    Index (Collection::*countGetter)() const,
    ElementType (Collection::*elementGetter)(Index) const,
    KeyType (std::remove_pointer_t<ElementType>::*keyGetter)() const,
    MatchCallback callback)
{
    Index leftIndex = 0;
    Index rightIndex = 0;
    Index leftSize = (left.*countGetter)();
    Index rightSize = (right.*countGetter)();

    // Process elements while both collections have remaining items
    while (leftIndex < leftSize && rightIndex < rightSize)
    {
        auto leftElement = (left.*elementGetter)(leftIndex);
        auto rightElement = (right.*elementGetter)(rightIndex);
        auto leftKey = (leftElement->*keyGetter)();
        auto rightKey = (rightElement->*keyGetter)();

        if (leftKey < rightKey)
        {
            // Left has element that right doesn't
            if (callback(leftElement, nullptr))
                return;
            leftIndex++;
        }
        else if (leftKey > rightKey)
        {
            // Right has element that left doesn't
            if (callback(nullptr, rightElement))
                return;
            rightIndex++;
        }
        else
        {
            // Found matching elements
            if (callback(leftElement, rightElement))
                return;
            leftIndex++;
            rightIndex++;
        }
    }

    // Process remaining elements from left collection (if any)
    while (leftIndex < leftSize)
    {
        auto leftElement = (left.*elementGetter)(leftIndex);
        if (callback(leftElement, nullptr))
            return;
        leftIndex++;
    }

    // Process remaining elements from right collection (if any)
    while (rightIndex < rightSize)
    {
        auto rightElement = (right.*elementGetter)(rightIndex);
        if (callback(nullptr, rightElement))
            return;
        rightIndex++;
    }
}

/// Overload for CapabilitySetVal target iteration
template<typename MatchCallback>
void concurrentIterate(
    const CapabilitySetVal& left,
    const CapabilitySetVal& right,
    MatchCallback callback)
{
    concurrentIterate(
        left,
        right,
        &CapabilitySetVal::getTargetSetCount,
        &CapabilitySetVal::getTargetSet,
        &CapabilityTargetSetVal::getTarget,
        callback);
}

/// Overload for CapabilityTargetSetVal stage iteration
template<typename MatchCallback>
void concurrentIterate(
    const CapabilityTargetSetVal& left,
    const CapabilityTargetSetVal& right,
    MatchCallback callback)
{
    concurrentIterate(
        left,
        right,
        &CapabilityTargetSetVal::getStageSetCount,
        &CapabilityTargetSetVal::getStageSet,
        &CapabilityStageSetVal::getStage,
        callback);
}


bool CapabilitySetVal::isIncompatibleWith(CapabilityAtom other) const
{
    SLANG_PROFILE_CAPABILITY_SETS;
    if (isEmpty())
        return false;

    // Convert single atom to a capability set and check compatibility
    auto otherSet = CapabilitySet(CapabilityName(other));
    return isIncompatibleWith(otherSet);
}

bool CapabilitySetVal::isIncompatibleWith(CapabilityName other) const
{
    SLANG_PROFILE_CAPABILITY_SETS;
    if (isEmpty())
        return false;

    // Convert single name to a capability set and check compatibility
    auto otherSet = CapabilitySet(other);
    return isIncompatibleWith(otherSet);
}

bool CapabilitySetVal::isIncompatibleWith(CapabilitySet const& other) const
{
    SLANG_PROFILE_CAPABILITY_SETS;
    if (isEmpty())
        return false;
    if (other.isEmpty())
        return false;

    // Two capability sets are incompatible if there are no intersecting target/stage combinations
    // Since Dictionary doesn't guarantee order, we iterate through the CapabilitySet's targets
    // But we can still optimize by avoiding repeated linear searches in this CapabilitySetVal
    for (auto& otherTargetPair : other.getCapabilityTargetSets())
    {
        auto otherTarget = otherTargetPair.first;

        // Use optimized linear search through sorted thisTargetSets
        auto thisTargetSet = findTargetSet(otherTarget);
        if (!thisTargetSet)
            continue;

        // Convert Dictionary to sorted iteration for stages
        for (auto& otherStagePair : otherTargetPair.second.getShaderStageSets())
        {
            auto otherStage = otherStagePair.first;

            // Use optimized linear search through sorted stage sets
            auto thisStageSet = thisTargetSet->findStageSet(otherStage);
            if (thisStageSet)
            {
                // Found matching target/stage pair - sets are compatible
                return false;
            }
        }
    }

    // No matching target/stage pairs found - sets are incompatible
    return true;
}

bool CapabilitySetVal::isIncompatibleWith(CapabilitySetVal const* other) const
{
    SLANG_PROFILE_CAPABILITY_SETS;
    if (!other)
        return false;
    if (isEmpty())
        return false;
    if (other->isEmpty())
        return false;

    // Two capability sets are incompatible if there are no intersecting target/stage combinations
    // Use concurrent iteration to find any matching target/stage pairs
    bool foundMatch = false;
    concurrentIterate(
        *this,
        *other,
        [&](CapabilityTargetSetVal* thisTarget, CapabilityTargetSetVal* otherTarget) -> bool
        {
            // Only process matching targets (skip gaps)
            if (!thisTarget || !otherTarget)
                return false;

            // Found matching targets - now check if any stages match
            concurrentIterate(
                *thisTarget,
                *otherTarget,
                [&](CapabilityStageSetVal* thisStage, CapabilityStageSetVal* otherStage) -> bool
                {
                    // Only process matching stages (skip gaps)
                    if (!thisStage || !otherStage)
                        return false;

                    // Found matching target/stage pair - set flag and exit early
                    foundMatch = true;
                    return true;
                });
            return foundMatch;
        });

    // If we found any matching target/stage pairs, sets are compatible
    return !foundMatch;
}

CapabilitySet::ImpliesReturnFlags CapabilitySetVal::_implies(
    CapabilitySetVal const* otherSet,
    CapabilitySet::ImpliesFlags flags) const
{
    SLANG_PROFILE_CAPABILITY_SETS;
    // By default (`ImpliesFlags::None`): x implies (c | d) only if (x implies c) and (x implies d).

    bool onlyRequireSingleImply =
        ((int)flags & (int)CapabilitySet::ImpliesFlags::OnlyRequireASingleValidImply);
    bool cannotHaveMoreTargetAndStageSets =
        ((int)flags & (int)CapabilitySet::ImpliesFlags::CannotHaveMoreTargetAndStageSets);
    bool canHaveSubsetOfTargetAndStageSets =
        ((int)flags & (int)CapabilitySet::ImpliesFlags::CanHaveSubsetOfTargetAndStageSets);

    int flagsCollected = (int)CapabilitySet::ImpliesReturnFlags::NotImplied;

    if (!otherSet || otherSet->isEmpty())
        return CapabilitySet::ImpliesReturnFlags::Implied;

    // If empty, and the other is not empty, it does not matter what flags are used,
    // `this` is considered to "not imply" another set. This is important since
    // `T.join(U)` causes `T == U`.
    if (this->isEmpty())
        return CapabilitySet::ImpliesReturnFlags::NotImplied;

    if (cannotHaveMoreTargetAndStageSets &&
        this->getTargetSetCount() > otherSet->getTargetSetCount())
    {
        return CapabilitySet::ImpliesReturnFlags::NotImplied;
    }

    // Use concurrent iteration to process all target pairs
    bool impliesAll = true;
    concurrentIterate(
        *this,
        *otherSet,
        [&](CapabilityTargetSetVal* thisTarget, CapabilityTargetSetVal* otherTarget) -> bool
        {
            if (!otherTarget)
            {
                // Other doesn't have this target - that's fine for implies
                return false; // Continue
            }

            if (!thisTarget)
            {
                // 'this' lacks a target 'other' has
                if (onlyRequireSingleImply || canHaveSubsetOfTargetAndStageSets)
                {
                    return false; // Continue
                }
                impliesAll = false;
                return true; // Early exit
            }

            if (cannotHaveMoreTargetAndStageSets &&
                thisTarget->getStageSetCount() > otherTarget->getStageSetCount())
            {
                impliesAll = false;
                return true; // Early exit
            }

            // Process all stage pairs for this target
            concurrentIterate(
                *thisTarget,
                *otherTarget,
                [&](CapabilityStageSetVal* thisStage, CapabilityStageSetVal* otherStage) -> bool
                {
                    if (!otherStage)
                    {
                        // Other doesn't have this stage - that's fine for implies
                        return false; // Continue
                    }

                    if (!thisStage)
                    {
                        // 'this' lacks a stage 'other' has
                        if (onlyRequireSingleImply || canHaveSubsetOfTargetAndStageSets)
                        {
                            return false; // Continue
                        }
                        impliesAll = false;
                        return true; // Early exit
                    }

                    // Check if all stage sets that are in 'other' are contained by 'this'
                    auto thisAtomSet = thisStage->getAtomSet();
                    auto otherAtomSet = otherStage->getAtomSet();

                    if (thisAtomSet && otherAtomSet)
                    {
                        auto thisUIntSet = thisAtomSet->toUIntSet();
                        auto otherUIntSet = otherAtomSet->toUIntSet();
                        bool contained = thisUIntSet.contains(otherUIntSet);

                        if (!onlyRequireSingleImply && !contained)
                        {
                            impliesAll = false;
                            return true; // Early exit
                        }
                        else if (onlyRequireSingleImply && contained)
                        {
                            flagsCollected |= (int)CapabilitySet::ImpliesReturnFlags::Implied;
                            return true; // Early exit - found one match
                        }
                    }

                    return false; // Continue
                });

            return !impliesAll; // Early exit if we failed
        });

    if (!onlyRequireSingleImply && impliesAll)
        flagsCollected |= (int)CapabilitySet::ImpliesReturnFlags::Implied;

    return (CapabilitySet::ImpliesReturnFlags)flagsCollected;
}

bool CapabilitySetVal::implies(CapabilitySet const& other) const
{
    SLANG_PROFILE_CAPABILITY_SETS;
    // For mixed types, convert to CapabilitySet to maintain compatibility
    return CapabilitySet{this}.implies(other);
}

CapabilitySet::ImpliesReturnFlags CapabilitySetVal::atLeastOneSetImpliedInOther(
    CapabilitySet const& other) const
{
    SLANG_PROFILE_CAPABILITY_SETS;
    // For mixed types, convert to CapabilitySet to maintain compatibility
    return CapabilitySet{this}.atLeastOneSetImpliedInOther(other);
}


#ifdef UNIT_TEST_CAPABILITIES

#define CHECK_CAPS(inData) SLANG_ASSERT(inData > 0)

int TEST_findTargetCapSet(CapabilitySet& capSet, CapabilityAtom target)
{
    return true && capSet.getCapabilityTargetSets().containsKey(target);
}

int TEST_findTargetStage(CapabilitySet& capSet, CapabilityAtom target, CapabilityAtom stage)
{
    return capSet.getCapabilityTargetSets()[target].shaderStageSets.containsKey(stage);
}


int TEST_targetCapSetWithSpecificAtomInStage(
    CapabilitySet& capSet,
    CapabilityAtom target,
    CapabilityAtom stage,
    CapabilityAtom atom)
{
    return capSet.getCapabilityTargetSets()[target].shaderStageSets[stage].atomSet->contains(
        (UInt)atom);
}

int TEST_targetCapSetWithSpecificSetInStage(
    CapabilitySet& capSet,
    CapabilityAtom target,
    CapabilityAtom stage,
    List<CapabilityAtom> setToFind)
{

    bool containsStageKey =
        capSet.getCapabilityTargetSets()[target].shaderStageSets.containsKey(stage);
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
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(
        testCapSet,
        CapabilityAtom::hlsl,
        CapabilityAtom::vertex,
        {CapabilityAtom::textualTarget,
         CapabilityAtom::hlsl,
         CapabilityAtom::vertex,
         CapabilityAtom::_sm_4_0,
         CapabilityAtom::_sm_4_1}));

    CHECK_CAPS(TEST_findTargetCapSet(testCapSet, CapabilityAtom::glsl));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(
        testCapSet,
        CapabilityAtom::glsl,
        CapabilityAtom::vertex,
        {CapabilityAtom::textualTarget,
         CapabilityAtom::glsl,
         CapabilityAtom::vertex,
         CapabilityAtom::_GLSL_130}));

    CHECK_CAPS(TEST_findTargetCapSet(testCapSet, CapabilityAtom::spirv_1_0));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(
        testCapSet,
        CapabilityAtom::spirv_1_0,
        CapabilityAtom::vertex,
        {CapabilityAtom::spirv_1_0, CapabilityAtom::vertex, CapabilityAtom::spirv_1_1}));

    CHECK_CAPS(TEST_findTargetCapSet(testCapSet, CapabilityAtom::metal));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(
        testCapSet,
        CapabilityAtom::metal,
        CapabilityAtom::vertex,
        {CapabilityAtom::textualTarget, CapabilityAtom::metal, CapabilityAtom::vertex}));

    // ------------------------------------------------------------

    testCapSet = CapabilitySet(CapabilityName::TEST_ADD_2);

    CHECK_CAPS(TEST_findTargetCapSet(testCapSet, CapabilityAtom::hlsl));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(
        testCapSet,
        CapabilityAtom::hlsl,
        CapabilityAtom::compute,
        {CapabilityAtom::textualTarget,
         CapabilityAtom::hlsl,
         CapabilityAtom::compute,
         CapabilityAtom::_sm_4_0,
         CapabilityAtom::_sm_4_1}));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(
        testCapSet,
        CapabilityAtom::hlsl,
        CapabilityAtom::fragment,
        {CapabilityAtom::textualTarget,
         CapabilityAtom::hlsl,
         CapabilityAtom::fragment,
         CapabilityAtom::_sm_4_0,
         CapabilityAtom::_sm_4_1}));

    // ------------------------------------------------------------

    testCapSet = CapabilitySet(CapabilityName::TEST_ADD_3);

    CHECK_CAPS((int)!TEST_findTargetCapSet(testCapSet, CapabilityAtom::spirv_1_0));
    CHECK_CAPS(TEST_findTargetCapSet(testCapSet, CapabilityAtom::glsl));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(
        testCapSet,
        CapabilityAtom::glsl,
        CapabilityAtom::fragment,
        {CapabilityAtom::textualTarget,
         CapabilityAtom::glsl,
         CapabilityAtom::fragment,
         CapabilityAtom::_GLSL_130}));

    // ------------------------------------------------------------

    testCapSet = CapabilitySet(CapabilityName::TEST_GEN_1);

    CHECK_CAPS(TEST_findTargetCapSet(testCapSet, CapabilityAtom::hlsl));
    CHECK_CAPS((int)!TEST_findTargetCapSet(testCapSet, CapabilityAtom::glsl));
    CHECK_CAPS(TEST_findTargetStage(testCapSet, CapabilityAtom::hlsl, CapabilityAtom::vertex));
    CHECK_CAPS(TEST_findTargetStage(testCapSet, CapabilityAtom::hlsl, CapabilityAtom::fragment));
    CHECK_CAPS(TEST_targetCapSetWithSpecificAtomInStage(
        testCapSet,
        CapabilityAtom::hlsl,
        CapabilityAtom::fragment,
        CapabilityAtom::_sm_6_0));
    CHECK_CAPS(TEST_targetCapSetWithSpecificAtomInStage(
        testCapSet,
        CapabilityAtom::hlsl,
        CapabilityAtom::fragment,
        CapabilityAtom::_sm_5_0));

    // ------------------------------------------------------------

    testCapSet = CapabilitySet(CapabilityName::TEST_GEN_2);

    CHECK_CAPS(TEST_findTargetCapSet(testCapSet, CapabilityAtom::hlsl));
    CHECK_CAPS((int)!TEST_findTargetCapSet(testCapSet, CapabilityAtom::glsl));
    CHECK_CAPS(TEST_findTargetStage(testCapSet, CapabilityAtom::hlsl, CapabilityAtom::fragment));
    CHECK_CAPS(TEST_targetCapSetWithSpecificAtomInStage(
        testCapSet,
        CapabilityAtom::hlsl,
        CapabilityAtom::fragment,
        CapabilityAtom::_sm_6_5));
    CHECK_CAPS(TEST_targetCapSetWithSpecificAtomInStage(
        testCapSet,
        CapabilityAtom::hlsl,
        CapabilityAtom::fragment,
        CapabilityAtom::_sm_5_0));

    // ------------------------------------------------------------

    testCapSet = CapabilitySet(CapabilityName::TEST_GEN_3);

    CHECK_CAPS(TEST_findTargetCapSet(testCapSet, CapabilityAtom::glsl));
    CHECK_CAPS(TEST_findTargetStage(testCapSet, CapabilityAtom::glsl, CapabilityAtom::fragment));
    CHECK_CAPS(TEST_targetCapSetWithSpecificAtomInStage(
        testCapSet,
        CapabilityAtom::glsl,
        CapabilityAtom::fragment,
        CapabilityAtom::_GL_NV_shader_texture_footprint));
    CHECK_CAPS(TEST_targetCapSetWithSpecificAtomInStage(
        testCapSet,
        CapabilityAtom::glsl,
        CapabilityAtom::fragment,
        CapabilityAtom::_GL_NV_compute_shader_derivatives));

    // ------------------------------------------------------------

    testCapSet = CapabilitySet(CapabilityName::TEST_GEN_4);

    CHECK_CAPS(TEST_findTargetCapSet(testCapSet, CapabilityAtom::glsl));
    CHECK_CAPS(TEST_findTargetStage(testCapSet, CapabilityAtom::glsl, CapabilityAtom::fragment));
    CHECK_CAPS(TEST_targetCapSetWithSpecificAtomInStage(
        testCapSet,
        CapabilityAtom::glsl,
        CapabilityAtom::fragment,
        CapabilityAtom::_GL_NV_shader_texture_footprint));
    CHECK_CAPS(TEST_targetCapSetWithSpecificAtomInStage(
        testCapSet,
        CapabilityAtom::glsl,
        CapabilityAtom::fragment,
        CapabilityAtom::_GL_ARB_shader_image_size));

    // ------------------------------------------------------------

    testCapSet = CapabilitySet(CapabilityName::TEST_GEN_5);

    CHECK_CAPS(TEST_findTargetCapSet(testCapSet, CapabilityAtom::hlsl));
    CHECK_CAPS(TEST_targetCapSetWithSpecificAtomInStage(
        testCapSet,
        CapabilityAtom::hlsl,
        CapabilityAtom::fragment,
        CapabilityAtom::_sm_6_5));
    CHECK_CAPS(TEST_targetCapSetWithSpecificAtomInStage(
        testCapSet,
        CapabilityAtom::hlsl,
        CapabilityAtom::fragment,
        CapabilityAtom::_sm_6_4));
    CHECK_CAPS(TEST_targetCapSetWithSpecificAtomInStage(
        testCapSet,
        CapabilityAtom::hlsl,
        CapabilityAtom::fragment,
        CapabilityAtom::_sm_6_0));
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
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(
        testCapSetA,
        CapabilityAtom::hlsl,
        CapabilityAtom::vertex,
        {CapabilityAtom::textualTarget,
         CapabilityAtom::hlsl,
         CapabilityAtom::vertex,
         CapabilityAtom::_sm_4_0,
         CapabilityAtom::_sm_4_1}));

    // ------------------------------------------------------------

    testCapSetA = CapabilitySet(CapabilityName::TEST_JOIN_3A);
    testCapSetB = CapabilitySet(CapabilityName::TEST_JOIN_3B);
    testCapSetA.join(testCapSetB);

    CHECK_CAPS((int)!TEST_findTargetCapSet(testCapSetA, CapabilityAtom::spirv_1_0));
    CHECK_CAPS(TEST_findTargetCapSet(testCapSetA, CapabilityAtom::glsl));
    CHECK_CAPS(
        (int)!TEST_findTargetStage(testCapSetA, CapabilityAtom::glsl, CapabilityAtom::raygen));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(
        testCapSetA,
        CapabilityAtom::glsl,
        CapabilityAtom::fragment,
        {CapabilityAtom::textualTarget,
         CapabilityAtom::glsl,
         CapabilityAtom::fragment,
         CapabilityAtom::_GLSL_130,
         CapabilityAtom::_GLSL_140}));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(
        testCapSetA,
        CapabilityAtom::glsl,
        CapabilityAtom::vertex,
        {CapabilityAtom::textualTarget,
         CapabilityAtom::glsl,
         CapabilityAtom::vertex,
         CapabilityAtom::_GLSL_130,
         CapabilityAtom::_GLSL_140}));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(
        testCapSetA,
        CapabilityAtom::hlsl,
        CapabilityAtom::fragment,
        {CapabilityAtom::textualTarget,
         CapabilityAtom::hlsl,
         CapabilityAtom::fragment,
         CapabilityAtom::_sm_4_0,
         CapabilityAtom::_sm_4_1}));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(
        testCapSetA,
        CapabilityAtom::hlsl,
        CapabilityAtom::vertex,
        {CapabilityAtom::textualTarget,
         CapabilityAtom::hlsl,
         CapabilityAtom::vertex,
         CapabilityAtom::_sm_4_0}));

    // ------------------------------------------------------------

    testCapSetA = CapabilitySet(CapabilityName::TEST_JOIN_4A);
    testCapSetB = CapabilitySet(CapabilityName::TEST_JOIN_4B);
    testCapSetA.join(testCapSetB);

    CHECK_CAPS(TEST_findTargetCapSet(testCapSetA, CapabilityAtom::glsl));
    CHECK_CAPS(TEST_targetCapSetWithSpecificSetInStage(
        testCapSetA,
        CapabilityAtom::glsl,
        CapabilityAtom::fragment,
        {CapabilityAtom::textualTarget,
         CapabilityAtom::glsl,
         CapabilityAtom::fragment,
         CapabilityAtom::_GLSL_130,
         CapabilityAtom::_GLSL_140,
         CapabilityAtom::_GLSL_150,
         CapabilityAtom::_GL_EXT_texture_query_lod,
         CapabilityAtom::_GL_EXT_texture_shadow_lod}));

    // ------------------------------------------------------------
}

void TEST_CapabilitySet()
{
    TEST_CapabilitySet_addAtom();
    TEST_CapabilitySet_join();
}

/*
/// Test Capabilities

alias TEST_ADD_1 = _sm_4_1 | _GLSL_130 | spirv_1_1 | metal;
alias TEST_ADD_2 = _sm_4_1 |& _sm_4_0 + compute_fragment;
alias TEST_ADD_3 = _GLSL_130 + compute_fragment_geometry_vertex;

alias TEST_GEN_1 = _sm_6_5 + fragment | _sm_6_0 + vertex;
alias TEST_GEN_2 = _sm_6_5 + fragment;
alias TEST_GEN_3 = GL_NV_shader_texture_footprint + GL_NV_compute_shader_derivatives + fragment
| _GL_NV_shader_texture_footprint + fragment; alias TEST_GEN_4 = GL_ARB_shader_image_size |&
GL_NV_shader_texture_footprint + fragment; alias TEST_GEN_5 = sm_6_0 + compute_fragment| sm_6_5;

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

// Will cause capability generator failiure
alias TEST_ERROR_GEN_1 = GL_NV_shader_texture_footprint + GL_NV_compute_shader_derivatives +
fragment | _GL_NV_shader_texture_footprint + _GL_NV_shader_atomic_fp16_vector + fragment; alias
TEST_ERROR_GEN_2 = GL_NV_shader_texture_footprint | GL_NV_ray_tracing_motion_blur; alias
TEST_ERROR_GEN_3 = GL_ARB_shader_image_size | GL_NV_shader_texture_footprint + fragment; alias
TEST_ERROR_GEN_4 = _sm_6_5 + fragment + vertex + cpp;

///
*/
#undef CHECK_CAPS

#endif

} // namespace Slang
