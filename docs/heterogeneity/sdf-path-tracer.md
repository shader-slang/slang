# An SDF path tracer, one step at a time

<!-- prettier-ignore -->
??? info "Sources"
    The sphere-tracing algorithm follows John C. Hart's
    [Sphere Tracing: A Geometric Method for the Antialiased Ray Tracing of Implicit Surfaces](https://doi.org/10.1007/s003710050084).
    The smooth union uses the polynomial smooth minimum described in Inigo Quilez's
    [Smooth Minimum](https://iquilezles.org/articles/smin/). This walkthrough presents
    these methods as interactive `h-slang` examples.

We will build an image of a sphere from a function that measures distance. Along the
way, we will learn how to describe a surface, find it along a ray, and light it.
These are the first pieces of a path tracer; this chapter ends with two shaded spheres
that blend into one smooth shape.

Each code cell is an editable Slang file. Its filename appears above the editor.
Press ![Run](images/snippet-run.svg "Run") or **Ctrl+Enter** (**⌘+Enter** on macOS) to see its image. Later cells
can import the current code from earlier cells, even if you have not run those cells
yet. After an edit, rerun the cells whose images you want to update. ![Reset](images/snippet-reset.svg "Reset") restores
one cell's original code.

All the images use compute dispatches: one GPU invocation calculates one pixel.
The folded `[host] main(Window w)` in each cell sets up the image and displays it.
The folded rendering helper contains the compute dispatch; expand it to follow the
pixel calculations separately from the window setup. No graphics background is needed;
the comments explain the coordinates as we introduce them.

## 1. Describe the sphere with a distance

A **signed distance function**, or **SDF**, takes a point in space and returns its
distance to the nearest surface. Its sign tells us which side we are on:

- **Negative:** inside the object.
- **Zero:** on its surface.
- **Positive:** outside the object.

For a sphere centred at the origin, `length(p)` is the distance from the centre to
the point `p`. Subtracting the sphere's radius gives its signed distance. With radius
1, the centre has distance −1, the point `(1, 0, 0)` has distance 0, and `(2, 0, 0)`
has distance +1. These distances use the same units as our coordinates.

The first image is a flat slice through this 3D function at `z = 0`.
**Orange means negative (inside); blue means positive (outside).** The thin white
boundary marks distances close to zero.

<!-- runnable: sphere canvas=16:9 -->

```slang
import slang.runtime;

// Public functions can be used by other cells through `import sphere`.
public float sphereDistance(float3 p)
{
    float radius = 1.0;
    // Distance from the centre, minus the radius: negative inside, positive outside.
    return length(p) - radius;
}

// Compute one output pixel per invocation. Expand to follow the image calculation.
[host]
void renderSphereSlice(RWTexture2D<float4> image, uint2 resolution, float sliceZ)
{ //!FOLD
    Grid(resolution).dispatch<8, 8>((uint2 pixel) =>
    {
        // Sample pixel centres. Dividing both axes by height keeps the circle round.
        float2 xy = (2.0 * (float2(pixel) + 0.5) - float2(resolution))
            / float(resolution.y) * 1.5;
        float distance = sphereDistance(float3(xy, sliceZ));
        float3 color = distance < 0.0
            ? float3(0.85, 0.32, 0.16) // Inside: orange.
            : float3(0.12, 0.35, 0.65); // Outside: blue.
        if (abs(distance) < 0.015)
            color = float3(1.0); // A narrow band around the surface.
        image[pixel] = float4(color, 1.0);
    });
}

[host]
void main(Window w)
{ //!FOLD
    // The host creates a canvas surface and an image for the compute shader to fill.
    let surface = createSurface<TextureFormat::rgba8Unorm>(w);
    let presented = acquireSurfaceTexture(surface);
    uint width, height;
    presented.GetDimensions(width, height);
    let resolution = uint2(width, height);
    let image = RWTexture2D<float4>::alloc(width, height);
    float sliceZ = 0.0; // Move this plane along the z axis to explore the sphere.

    renderSphereSlice(image, resolution, sliceZ);

    blitTexture2D(presented, image);
    presentSurface(surface);
}
```

**Try it:** change `radius` to `0.7` and press ![Run](images/snippet-run.svg "Run") in the cell. Then expand `main` and try
`sliceZ = 0.5`. Why does the cross-section shrink? At a slice farther from the centre
than the radius, there is no inside region at all. Press ![Reset](images/snippet-reset.svg "Reset") in the cell before continuing.

## 2. Follow a ray until it reaches the surface

A **ray** starts at an origin and travels in a direction. A point along it is
`origin + t * direction`. We normalize the direction to length 1 so that `t` measures
distance in our scene's units.

Our camera starts outside the sphere. At each point along a ray, the SDF tells us
how far away the nearest surface is. We can advance by that distance without
crossing the surface. Repeating this is called **sphere tracing**. It also works
for other shapes when their functions provide valid distance bounds.

We stop when the distance is smaller than a hit tolerance. A maximum distance and
a step budget keep rays that miss from running forever. The function returns `true`
for a hit and writes the point it found into `hitPosition` through an `out` parameter.

This cell imports the first cell's function. Its image is a **binary intersection
mask**: white pixels hit the sphere; black pixels miss.

<!-- runnable: tracing canvas=16:9 -->

```slang
import slang.runtime;
import sphere; // This is the current contents of the sphere.slang cell above.

// The caller supplies a unit direction and an origin outside the sphere.
public bool sphereTrace(float3 origin, float3 direction, out float3 hitPosition)
{
    float hitTolerance = 0.001; // Close enough to count as touching the surface.
    float maxDistance = 10.0;  // Stop looking beyond this distance from the camera.
    int maxSteps = 96;         // A fixed budget also bounds work near the silhouette.
    float t = 0.0;
    hitPosition = origin;

    for (int step = 0; step < maxSteps; ++step)
    {
        hitPosition = origin + t * direction;
        float distance = sphereDistance(hitPosition);
        if (distance < hitTolerance)
            return true;

        t += distance; // Advance by the distance to the nearest surface.
        if (t > maxDistance)
            return false;
    }
    return false; // The ray used its step budget without finding a surface.
}

// Aim rays through a screen in front of the camera, preserving the image's aspect ratio.
public float3 cameraRay(uint2 pixel, uint2 resolution)
{
    float2 screen = (2.0 * (float2(pixel) + 0.5) - float2(resolution))
        / float(resolution.y);
    screen.y = -screen.y; // Image rows grow downwards; our scene's y axis points up.
    return normalize(float3(screen, 1.5)); // The camera looks along positive z.
}

// Compute one output pixel per invocation. Expand to follow the image calculation.
[host]
void renderIntersectionMask(RWTexture2D<float4> image, uint2 resolution)
{ //!FOLD
    Grid(resolution).dispatch<8, 8>((uint2 pixel) =>
    {
        float3 origin = float3(0.0, 0.0, -3.0); // Camera behind the sphere, looking forward.
        float3 hitPosition;
        bool hit = sphereTrace(origin, cameraRay(pixel, resolution), hitPosition);
        image[pixel] = float4(hit ? float3(1.0) : float3(0.0), 1.0);
    });
}

[host]
void main(Window w)
{ //!FOLD
    let surface = createSurface<TextureFormat::rgba8Unorm>(w);
    let presented = acquireSurfaceTexture(surface);
    uint width, height;
    presented.GetDimensions(width, height);
    let resolution = uint2(width, height);
    let image = RWTexture2D<float4>::alloc(width, height);

    renderIntersectionMask(image, resolution);

    blitTexture2D(presented, image);
    presentSurface(surface);
}
```

**Try it:** change the radius in `sphere.slang` to `0.7`, then press ![Run](images/snippet-run.svg "Run") in **this** cell.
The white silhouette becomes smaller without copying any sphere code. You can also
reduce `maxSteps` to see why rays near the edge can need more steps. Press ![Reset](images/snippet-reset.svg "Reset") in both cells
before continuing. Keep the camera outside the sphere for this version of the tracer.

## 3. Estimate a normal and add light

The mask tells us where the sphere is, but every hit has the same colour. To add
shading, we need the surface's **normal**: a unit vector pointing out of the surface.

The SDF increases as we move outwards. Its **gradient** points in the direction of
greatest increase, so normalizing that gradient gives us a normal. We can estimate
each component by sampling nearby points. Along x, for example, the rate of change is
`(distance(p + dx) - distance(p - dx)) / (2 * epsilon)`. Repeating this along y and z
uses six samples in total. This is a **central finite difference**.

We then measure how much the normal faces the light using a dot product. A value of
1 means directly facing the light, 0 means sideways, and a negative value means facing
away. Clamping negative values to zero gives simple diffuse shading. We add a small
constant fill so the unlit side stays visible; it is an artistic approximation of
indirect light, which a later path-tracing chapter will compute.

<!-- runnable: shading canvas=16:9 -->

```slang
import slang.runtime;
import sphere;  // Reuse the distance function.
import tracing; // Reuse sphereTrace and cameraRay, without running the other cell's main.

public float3 surfaceNormal(float3 p)
{
    float epsilon = 0.002; // Distance between the centre and each neighbouring sample.
    float3 dx = float3(epsilon, 0.0, 0.0);
    float3 dy = float3(0.0, epsilon, 0.0);
    float3 dz = float3(0.0, 0.0, epsilon);

    // Estimate how quickly distance changes along each of the three axes.
    float3 gradient = float3(
        sphereDistance(p + dx) - sphereDistance(p - dx),
        sphereDistance(p + dy) - sphereDistance(p - dy),
        sphereDistance(p + dz) - sphereDistance(p - dz)) / (2.0 * epsilon);
    return normalize(gradient); // Shading needs a direction with length 1.
}

public float3 shadeSurface(float3 position)
{
    float3 normal = surfaceNormal(position);
    // Direction FROM the surface TOWARD the light: above, left, and towards the camera.
    float3 toLight = normalize(float3(-0.6, 0.8, -1.0));
    float diffuse = max(dot(normal, toLight), 0.0);
    float3 surfaceColor = float3(0.25, 0.65, 0.35);
    return surfaceColor * (0.1 + 0.9 * diffuse); // Constant fill plus directional light.
}

// Compute one output pixel per invocation. Expand to follow the image calculation.
[host]
void renderShadedSphere(RWTexture2D<float4> image, uint2 resolution)
{ //!FOLD
    Grid(resolution).dispatch<8, 8>((uint2 pixel) =>
    {
        float3 origin = float3(0.0, 0.0, -3.0);
        float3 hitPosition;
        float3 color = float3(0.025, 0.035, 0.05); // Background for rays that miss.
        if (sphereTrace(origin, cameraRay(pixel, resolution), hitPosition))
            color = shadeSurface(hitPosition);

        // Approximate display encoding: lighting was calculated in linear colour.
        image[pixel] = float4(pow(color, float3(1.0 / 2.2)), 1.0);
    });
}

[host]
void main(Window w)
{ //!FOLD
    let surface = createSurface<TextureFormat::rgba8Unorm>(w);
    let presented = acquireSurfaceTexture(surface);
    uint width, height;
    presented.GetDimensions(width, height);
    let resolution = uint2(width, height);
    let image = RWTexture2D<float4>::alloc(width, height);

    renderShadedSphere(image, resolution);

    blitTexture2D(presented, image);
    presentSurface(surface);
}
```

**Try it:** change the light's x component from `-0.6` to `0.6`. The bright side
moves from left to right. Then choose another `surfaceColor`. To explore the normal
estimate, try increasing `epsilon` to `0.2`; large offsets blur local detail, while
extremely small offsets can lose precision when subtracting nearly equal distances.

## 4. Blend two spheres into one shape

We can move a sphere without changing its distance function. To place its centre at
`centre`, evaluate `sphereDistance(p - centre)`: subtracting the centre expresses the
sample point in the sphere's local coordinates. Evaluate this twice to get distances
to a sphere on the left and another on the right.

The **union** of two objects contains everything inside either one. Taking
`min(left, right)` combines their distance functions, but leaves a sharp crease where
the two surfaces meet. A **smooth union** rounds that join so the spheres look as if
they are melting into each other.

Our smooth minimum starts with the smaller distance, then subtracts a small correction
where the two distances are similar. `blendWidth` sets the width of that transition
in scene units. A larger value fills out the join; zero gives the ordinary union.
This changes the geometry, so both ray intersections and normals must sample the
combined field. Blending only the pixel colours would leave the crease in place.

Press ![Run](images/snippet-run.svg "Run") in this cell to replace the single sphere with a connected, shaded pair. The visible
functions define the new shape. The folded helpers repeat the tracing and shading
algorithms from the earlier cells, with `sceneDistance` replacing `sphereDistance`
in both the tracer and the six normal samples. The camera and lighting stay the same.

<!-- runnable: blending canvas=16:9 -->

```slang
import slang.runtime;
import sphere;  // Reuse the original unit sphere at two different positions.
import tracing; // Reuse cameraRay from the intersection-mask cell.

public float smoothUnion(float a, float b, float blendWidth)
{
    if (blendWidth <= 0.0)
        return min(a, b); // An ordinary union, with no rounding at the join.

    // h is 1 when the distances agree, and 0 outside the blending region.
    float h = max(blendWidth - abs(a - b), 0.0) / blendWidth;
    // Lowering the distance fills the join. The squared weight makes it smooth.
    return min(a, b) - 0.25 * blendWidth * h * h;
}

public float sceneDistance(float3 p)
{
    float halfSeparation = 0.9; // Centres are at x = -0.9 and x = +0.9.
    float blendWidth = 0.6;     // Increase this to make the connecting neck thicker.
    float left = sphereDistance(p - float3(-halfSeparation, 0.0, 0.0));
    float right = sphereDistance(p - float3(halfSeparation, 0.0, 0.0));
    return smoothUnion(left, right, blendWidth);
}

// The same sphere-tracing loop, now querying the whole scene.
bool traceScene(float3 origin, float3 direction, out float3 hitPosition)
{ //!FOLD
    float hitTolerance = 0.001;
    float maxDistance = 10.0;
    int maxSteps = 96;
    float t = 0.0;
    hitPosition = origin;
    for (int step = 0; step < maxSteps; ++step)
    {
        hitPosition = origin + t * direction;
        float distance = sceneDistance(hitPosition);
        if (distance < hitTolerance)
            return true;
        t += distance;
        if (t > maxDistance)
            return false;
    }
    return false;
}

// The same six samples, taken from the combined field so the join shades smoothly.
float3 sceneNormal(float3 p)
{ //!FOLD
    float epsilon = 0.002;
    float3 dx = float3(epsilon, 0.0, 0.0);
    float3 dy = float3(0.0, epsilon, 0.0);
    float3 dz = float3(0.0, 0.0, epsilon);
    float3 gradient = float3(
        sceneDistance(p + dx) - sceneDistance(p - dx),
        sceneDistance(p + dy) - sceneDistance(p - dy),
        sceneDistance(p + dz) - sceneDistance(p - dz)) / (2.0 * epsilon);
    return normalize(gradient);
}

// Keep the previous light and material, using the new surface normal.
float3 shadeScene(float3 position)
{ //!FOLD
    float3 normal = sceneNormal(position);
    float3 toLight = normalize(float3(-0.6, 0.8, -1.0));
    float diffuse = max(dot(normal, toLight), 0.0);
    float3 surfaceColor = float3(0.25, 0.65, 0.35);
    return surfaceColor * (0.1 + 0.9 * diffuse);
}

[host]
void renderBlendedSpheres(RWTexture2D<float4> image, uint2 resolution)
{ //!FOLD
    Grid(resolution).dispatch<8, 8>((uint2 pixel) =>
    {
        float3 origin = float3(0.0, 0.0, -3.0);
        float3 hitPosition;
        float3 color = float3(0.025, 0.035, 0.05);
        if (traceScene(origin, cameraRay(pixel, resolution), hitPosition))
            color = shadeScene(hitPosition);
        image[pixel] = float4(pow(color, float3(1.0 / 2.2)), 1.0);
    });
}

[host]
void main(Window w)
{ //!FOLD
    let surface = createSurface<TextureFormat::rgba8Unorm>(w);
    let presented = acquireSurfaceTexture(surface);
    uint width, height;
    presented.GetDimensions(width, height);
    let resolution = uint2(width, height);
    let image = RWTexture2D<float4>::alloc(width, height);

    renderBlendedSpheres(image, resolution);

    blitTexture2D(presented, image);
    presentSurface(surface);
}
```

**Try it:** set `blendWidth` to `0.0` and press ![Run](images/snippet-run.svg "Run") again to reveal the crease. Then try
`0.3`, `0.6`, and `1.0` to see the connecting neck grow. With `blendWidth = 0.6`,
change `halfSeparation` to `1.2`: the spheres move far enough apart for the neck to
break. Press ![Reset](images/snippet-reset.svg "Reset") before continuing. The radius still comes from `sphere.slang`, so edits
to the first cell affect both spheres here.

The combined field is a conservative distance bound outside this shape, rather than
an exact distance everywhere. It still supports our sphere-tracing steps, and its
gradient supplies normals for the blended surface.

We now have composable shapes, a ray/surface intersection, and normals for shading.
The next chapters can use these same pieces to sample environment lighting and follow
multiple light bounces, accumulating samples into a full path-traced image.
