// This definition depends on the other module's extern type.
import "link-time-type-cycle.slang";
export struct A = B;
