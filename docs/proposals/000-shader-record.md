SP #000: Disambiguate `uniform` via `[[push_constant]]` and `[[shader_record]]` 
=======================================================================

This document proposes a first step toward resolving ambiguities and bugs that
arise from the current usage of `uniform` (and to a lesser extent, `varying`)
in Slang, particularly in ray tracing pipelines.

<!-- It suggests introducing new types such as `PushConstant<T>` and
`ShaderRecord<T>`, which can be used to unify compile-time mappings across APIs
like Direct3D and Vulkan. These new types act as clearer, more consistent
markers of data binding points, compared to the over-loaded semantics
of `uniform` parameters today. -->

It suggests promoting Vulkan's `[[push_constant]] uniform T` and 
`[[shader_record]] uniform T` global annotations to more universal `[[push_constant]] T` and 
`[[shader_record]] T` entrypoint parameter annotations. These annotations can then be used 
to unify compile-time mappings across APIs like Direct3D, Vulkan, OptiX, etc. These 
new annotations act as clearer, more consistent markers of data binding points, 
compared to the over-loaded semantics of `uniform` parameters today.

Additionally, this document proposes annotations to more clearly distinguish between 
payload and hit attribute entry point parameters, extending the use of `[[payload]] T` and 
by introducing `[[hit_attribute]] T` respectively. This would allow users to more explicitly 
mark payload parameters as being constant, eg `void anyhit([[payload]] in MyConstPayload p)`, 
where current usage, `void anyhit(in MyConstPayload p)`, would otherwise incorrectly map 
`p` to hit attribute, and `void anyhit(MyConstPayload p)` would map `p` to the shader record. 

Status
------

**Status**: Work-in-progress / Design Review

**Implementation**: 
No implementation yet; to be proposed in future PR(s).

**Author**:  
*Nate Morrical*

**Reviewer**:  
*Tess Foley, Slang Team & Community*

Background
----------

Historically, shading languages like RenderMan Shading Language (RIB) had a 
relatively simple distinction between `uniform` values (meaning, constant over
a surface) and `varying` values (those that vary per-vertex or per-fragment). 
In more modern GPU pipelines, however, the notion of what is "uniform" and what 
"varies" has become nuanced---shaders run at different rates (e.g., per-thread, 
per-warp, per-thread-group, per-intersected-geometry, recursion depth, etc.), 
and these concepts vary further depending on the API (Vulkan vs. Direct3D) and the 
pipeline stage (vertex, fragment, compute, or ray tracing).

In ray tracing specifically, the term `uniform` has an additional burden:
- For `raygen` or `closesthit` or `anyhit` or `intersection` shaders,
  parameters marked `uniform` are mapped to the per-shader-record data
  (in Vulkan, via the *shader binding table* record).
- For compute shaders or other pipelines, the same `uniform` today
  maps to push constants (e.g., Vulkan's push constants or D3D root constants).

This has led to confusion and mismatch in portability:
- In Vulkan, certain rapidly-changing parameters go into push constants, while other, 
  more fixed parameters (eg vertex/index buffer pointers) go into the shader record.
  When mixed entry points are present (for example, a compute shader animating 
  triangle vertex positions, followed by subsequent RT entrypoints to render that 
  mesh), what uniforms map to where is inconsistent. 
  - Additionally, with mixed entry point setups, (iiuc) there is no way to leverage 
    Vulkan's per-entry-point-type push constants mechanism for ray tracing entry points. 
    Raygen push constants map to closest hit push constants, which map to closest hit, etc...
    Instead of pushing constants to a specific stage type, users today must instead 
    update all SBT entries, eg for all closest hits, to achieve the same expected behavior.
- In Direct3D, by contrast, these concepts do not have direct 1:1 equivalents
  in HLSL or DXIL. Instead, developers simulate push constants/shader-record
  data via constant buffer (`ConstantBuffer<T>`) or resource descriptors,
  with CPU-side code controlling the *root signature* (including local root
  signatures for ray tracing).

Certain annotations today currently lead to undefined behavior and system crashes.
- For example, consider the following, where annotations are supplied but ignored. 
    ```
    [shader("raygeneration")]
    void simpleRayGen([push_constants] ConstData params, [shader_record] RayGenData record) { ... }
    ```
    - This causes undefined behavior, as both implicitly add the keyword `uniform`, then both map to the shader record, and 
    annotations are otherwise ignored.

Moreover, the keywords `uniform` and `varying` do not convey the actual
"rate" or "lifetime scope" well here. One developer's "uniform" might need to be
"per-warp" or "per-thread" in another context. 

Beyond this, the subtle distinction in default behavior between `in T` and `inout T` on ray tracing
entrypoints can quickly lead to undefined behavior, where the user's intention to mark a 
payload value as constant instead remaps the payload to either hit attribute registers, or the shader 
binding table. 


Related Work
------------
In the original **RenderMan Shading Language** (RSL), the syntax of a declaration was
`[class] [type] [ "[ n ]" ]` where class may be `constant,`, `uniform`, `varying`, 
or `vertex`. For traditional shaders, `uniform` and `varying` had very precise meanings
rooted in how RenderMan handled per-primitive or per-sample shading. Pixar's documentation 
and the RenderMan specification at the time defined them as:
- `uniform` variables are those whose values are constant over whatever portion of the 
  surface begin shaded, while `varying` variables are those that may take on different values at different locations on the 
  surface being shaded.
  > For example, shaders inherit a color and a transparency from the graphics state. 
    These values do not change from point to point on the surface and are thus uniform variables. 
    Color and opacity can also be specified at the vertices of geometric primitives (see Section 5, 
    Geometric Primitives). In this case they are bilinearly interpolated across the surface, and 
    therefore are varying variables. [RISpec, Section 11](https://hradec.com/ebooks/CGI/RPS_13.5/prman_technical_rendering/users_guide/RISpec-html/section11.html)
- As the language evolved, ambiguities emerged with respect to the "rate" that values vary. 
  For example, `facevarying` was added in a [subsequent revision](https://hradec.com/ebooks/CGI/RPS_13.5/prman_technical_rendering/users_guide/RISpec-html/appendix.I.html) 
  to disambiguiate certain interpolation behaviors for subdivision surfaces.
  > Associated with each geometric primitive definition are additional primitive variables that 
    are passed to their shaders. These variables may define quantities that are constant over 
    the surface (class constant), piecewise-constant but with separate values per subprimitive 
    (class uniform), bilinearly interpolated (class varying and facevarying), or fully interpolated 
    (class vertex). If the primitive variable is uniform, there is one value per surface facet. 
    If the primitive variable is varying, there are four values per surface facet, one for each corner 
    of the unit square in parameter space (except polygons, which are a special case). On parametric 
    primitives (quadrics and patches), varying primitive variables are bilinearly interpolated across 
    the surface of the primitive. Colors, opacities, and shading normals are all examples of varying 
    primitive variables.  If a primitive variable is facevarying it will be linearly interpolated. 
    [More here](https://hradec.com/ebooks/CGI/RPS_13.5/prman_technical_rendering/users_guide/RISpec-html/section5.html#primitive%20variables)

As shading languages evolved, the rate to which something varies or remains uniform has continued to grow
in complexity.
- **GLSL**: Introduced specialized input/output qualifiers for geometry,
  tessellation, etc., but only partially addresses the nuance of multiple
  compute or ray tracing rates, let alone mixed entry points.
- **HLSL**: In Direct3D 12, "root constants" and "local root signatures" can
  emulate Vulkan's push constants and shader record. HLSL, however, does not
  have a built-in type or keyword that designates "this is root-constant data."
- **Slang**: Currently allows global-scope `cbuffer` or `ConstantBuffer<T>` with
  attributes like `[[vk::push_constant]]`, or `[[vk::shader_record]]` to map to
  Vulkan's push constants and the shader binding table. However, having a plain
  `uniform` parameter in an entry point can become ambiguous when compiling
  to multiple backends or mixing multiple entry points in the same Slang module.
  Global attributed uniform buffers prevent more localized and reduced usage of 
  per-dispatch constant values. 

The following attempts to disambiguate the two overloaded uses of `uniform` that 
Slang users face today.

Proposed Approach
-----------------

<!-- 1. **Introduce `PushConstant<T>`**   -->
1. **Extend the use of `[[push_constant]] T` to entry point parameters**  
   We would extend the use of `[[push_constant]]` to become semantically equivalent to:
   ```
   [[vk::push_constant]] ConstantBuffer<T> 
   ```
   
   This attribute would be used like so:

   ```
   [shader("raygen")]
   void entrypoint_1([[push_constant]] FirstParams p) {...}
   
   [shader("closesthit")]
   void entrypoint_2([[push_constant]] SecondParams p) {...}
   ```

   Note, usage of this annotation would allow users to dop the keyword `uniform` entirely, in favor 
   of the more rate-specific binding nomenclature. 

   For targets that do not have a first-class notion of push constants, 
   Slang would map types marked with this annotation to a normal constant buffer 
   or equivalent. By extending the use of this annotation, we give developers a clear, 
   explicit signal in their Slang code that a given parameter is intended to be 
   "per dispatch/draw" or "root constant" data. 
   
   Additionally, this resolves ambiguities where `[[push_constant]]` parameters proceed 
   shader record parameters, leading to undefined behavior when both are unintentionally mapped to the 
   shader record. 

   From here, we would amend the comment,
    > Mark a global variable as a Vulkan push constant.
   which appears in Slang's vscode extension to reflect the updated usage, and intended universal
   behavior across targets. The annotation being universal would then signal that behavior is well defined across 
   all possible backends, rather than specifically to Vulkan.

<!-- 2. **Introduce `ShaderRecord<T>`** -->
2. **Extend the use of `[[shader_record]] T`**
    Likewise, we would extend the use of `[[shader_record]]` to become systematically equivalent to:
    ```
    [[vk::shader_record]] ConstantBuffer<T>
    // or
    void entrypoint([[vk::push_constant]] uniform T, ...)
    ```
    Again, this annotation would drop the keyword `uniform` entirely.

    This annotation clarifies that the data is intended to reside in the shader binding 
    table record for ray tracing pipelines in Vulkan and in OptiX. For Direct3D 12, Slang 
    would map this to a local root signature. There, the new annotation is purely a semantic 
    wrapper around `ConstantBuffer<T>` but with additional reflection information letting 
    the application code handle it properly across APIs.

3. **Map all Vulkan Entry-Point `uniform` Parameters to `[[push_constant]]`**
    * Define bare `uniform` as `[[push_constant]]` for all entry points, including ray
    tracing shaders. This is the most common usage pattern, so I'd argue it makes sense for 
    this to be default behavior. Ensuring consistency of behavior of `uniform` across all entry
    point types when no rate-specific attribute is specified will help disambiguate current 
    usage in mixed compute and RT entry point setups. Then, encourage developers to declare 
    typed parameters as `[[push_constant]]` or `[[shader_record]]` to remove ambiguity.

4. **Enhance Reflection**
    Slang's reflection mechanism should expose a distinct "kind" or category for 
    entry point parameters marked as `[[push_constant]]` vs. `[[shader_record]]`. 
    The application code can then inspect reflection data to see how the compiler decided to place
    each parameter.

5. **Consolidate Push Constants / Shader Record Data**
    * As an optimization, or even a required step in certain backends, multiple 
    `[[push_constant]]` declarations in a single scope could be fused into one physical 
    push-constant region at the IR/legalization step.
    * Similarly, `[[shader_record]]` annotated declarations for a single entry point could be 
    linearized into a single region, with each field laid out in memory consecutively.
    This design ensures we do not end up with multiple push constants or multiple 
    local root signatures overshadowing each other.

6. **Introduce `[[hit_attribute]]`**
    A new user-facing annotation that is only legal for ray tracing entry points, and that is
    semantically equivalent to:
    ```
    void entrypoint(in T param)
    ```
    This annotation clarifies that the given parameter is intended to map to the hit attribute 
    registers assigned by either built-in intersectors or user-geometry intersectors (as opposed 
    to payload registers with constant usage, or to the shader record when uniform is omitted 
    entirely).

7. **Extend `[[payload]]`**
    Similar to `[[shader_record]]` and `[[push_constant]]`, we would extend the use of the `[[payload]]` 
    annotation to clarify that an entry point parameter is intended to map to user-driven push constant registers. 
    This would be semantically equivalent to:
    ```
    void entrypoint(inout T param)
    ```
    And would be used like so:
    ```
    void entrypoint([[payload]] T param)
    ```
    
    This annotation clarifies that the given parameter is intended to map to the payload registers
    assigned by the user. `in`, `out`, and `inout` would all become legal attributes on the variable, with
    `inout` being the default. 
    
    This would resolve the ambiguity regarding `in` and implicit `uniform` incorrectly mapping to hit attributes
    and shader records. 


Detailed Explanation
--------------------
1. **Language-Level Model**
    In a future ideal version of Slang, we might introduce explicit syntax for 
    specifying data "rates," e.g. `[[thread_group]]`, `[[wave]]`, `[[lane]]`, or more. 
    This is consistent with advanced GPU programming models that differentiate per-thread, 
    per-wavefront, per-group, per-dispatch, etc. However, this proposal focuses on 
    disambiguating push constant uniforms from shader record uniforms, as these are the most 
    pressing distinction.

2. **Entry-Point Parameter Rules**
    * If an entry point parameter is annotated with `[[push_constant]]`, Slang recognizes it as 
    data bound as push constants (Vulkan), launch parameters (OptiX), or root constants (D3D).
    Any additional `uniform` attribute following `[[push_constant]]` is optional, as `uniform` is 
    already implied.
    * If an entry point parameter is annotated with `[[shader_record]]`, Slang recognizes it as 
    data bound as part of the local record (Vulkan's and OptiX's SBT, D3D's local root signature).
    Similarly, any additional `uniform` attribute following `[[shader_record]]` is optional, as `uniform` is 
    already implied. 
    * If the function parameter is labeled `uniform`, but does not specify one of the 
    above explicit types, the compiler may insert an "implicit" `[[push_constant]]` to optimize
    performance based on the target language. 
    * Default functionality of `uniform` will be to map to `[[push_constant]]` for all entry point 
    types. 

3. **IR and Reflection Impact**
    * For Vulkan, the compiler can remap annotations to the necessary `[[vk::push_constant]]` or 
    `[[vk::shader_record]]` attributes under the hood.
    * For D3D, the compiler would still produce a `ConstantBuffer<T>` or resource 
    binding in reflection metadata, but with an additional 
    `slang::TypeReflection::Kind::PushConstant` or `slang::TypeReflection::Kind::ShaderRecord` hint. 
    This allows app code to unify or alias these buffers with local or global root 
    signatures. 
    <!--- I'm not very knowledgeable of Slang's reflection capabilities. This probably 
    needs a second look... --->
    
4. **Migration Strategy**
    * Existing Slang code using `[[vk::push_constant]] ConstantBuffer<T>` or 
    `[[vk::shader_record]] ConstantBuffer<T>` remains valid; it just becomes a more 
    verbose variant of the promoted annotations.
    * We would want to legalize the use of `[[vk::push_constant]]` and `[[vk::shader_record]]` 
    as recognized attributes of entry point parameters. (Today, these compile, but don't seem to 
    be respected.)
    * Legacy usage of bare uniform parameters in ray tracing entry points is rare. Still, 
    we might want to emit a warning and guidance on migration to more rate-specific annotations 
    while the change is new.

5. **Example**
Rather than this:
```
[shader("anyhit")]
void myAnyHitShader(
    in T1 a, 
    inout T2 b, 
    T3 c, 
    uniform T4 d
) {...}
```

We would now support the following:

```
[shader("anyhit")]
void myAnyHitShader(
    [[hit_attribute]] T1 a, 
    [[payload]] inout T2 b, 
    [[push_constant]] T3 c, 
    [[shader_record]] T4 d
) {...}
```

* On Vulkan, constant "uniform" data is now compiled to two distinct binding regions:
    * `T3` in the push constant region 
    * `T4` in the shader binding table record
* On D3D, the same code would reflect as two `ConstantBuffer<T>` regions, with 
reflection metadata marking them as "intended for push constant" vs. "intended 
for shader record."
* `[[payload]]` and `[[hit_attribute]]` distinguish which registers map to `T1` and `T2`
* Any remaining unannotated parameters would default to `[[push_constant]]`, which would match
current behavior with other entry point types.  

It would also now be possible to specify different push constant structures in a mixed ray tracing
entrypoint setup. 

Alternatives Considered
-----------------------
1. Keep Using uniform as is
    * We could continue to rely on uniform parameters, automatically mapping them to 
    different memory regions based on stage. However, this has proven confusing, and has 
    negative performance implications, especially in complex pipelines mixing ray tracing,
    compute, and graphics. It also does not convey the notion of "what is uniform over what," 
    which is critical in advanced GPU code.

2. Global Scope or Separate Slang Modules
    * One could define separate .slang files for each entry point or set of parameters,
    so that only one usage of `[[vk::push_constant]]` or `[[vk::shader_record]]` is 
    visible at a time. This can work for small projects, but breaks down at scale and
    is not a satisfying long-term fix.

3. `SV_Payload` and `SV_Attributes`
    * At one point in time, we had `SV_RayPayload`, which has since been removed 
    (but still seemingly exists in our OptiX examples?). Technically, both ray payload
    and hit attribute register offsets are considered to be "system values", though personally
    I find I prefer `([[annotation]] T myT)` over `(T myT : SV_SystemValue)`.

4. `[[constant]]` rather than `[[push_constant]]`
    * "Push Constant" verbage comes from Vulkan, however, other target API like OptiX/CUDA have
    similar concepts. In OptiX, such parameters are called "launch parameters". Using a more general
    name like `[[constant]]` could help clarify that the attribute is meant for mroe than just Vulkan.
    Still, extending `[[push_constant]]` might be a more natural approach, seeing as we already have 
    this annotation. 

By extending `[[push_constant]]` and `[[shader_record]]` annotations, we offer a clearer 
language-level model that reduces the ambiguity around uniform and bridges the gap 
across GPU backends and pipeline stages. This proposal is a stepping stone toward a 
more comprehensive "rates" system in Slang, while providing immediate, practical 
improvements to developers.