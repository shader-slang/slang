// slang-ast-modifier.cpp
#include "slang-ast-modifier.h"
#include "slang-ast-expr.h"

namespace Slang
{
const OrderedDictionary<DeclRefBase*, SubtypeWitness*>& DifferentiableAttribute::getMapTypeToIDifferentiableWitness()
{
    for (Index i = m_mapToIDifferentiableWitness.getCount(); i < m_typeToIDifferentiableWitnessMappings.getCount(); i++)
        m_mapToIDifferentiableWitness.add(m_typeToIDifferentiableWitnessMappings[i].key, m_typeToIDifferentiableWitnessMappings[i].value);
    return m_mapToIDifferentiableWitness;
}
} // namespace Slang
