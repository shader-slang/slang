// setter-method.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Make sure we provide a user a diagnostic if they
// try to declare a setter method without `mutating`
// (even if we don't support `mutating` yet).

struct Sphere
{
    float3 center;
    float radius;

    void setCenter(float3 value)
    {
        center = value;
//diag: ^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
//diag:        ^ left of '=' is not an l-value
//diag:        ^ left of '=' is not an l-value.
    }

    void setRadius(float value)
    {
        this.radius = value;
//diag:      ^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
//diag:             ^ left of '=' is not an l-value
//diag:             ^ left of '=' is not an l-value.
    }
};
