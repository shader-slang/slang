// Together with the A definition, this creates a cycle only after composition.
import "link-time-type-cycle.slang";
export struct B = A;
