# Cell link fixture

[Function](cell:link-types.slang#target)
[Constant](cell:link-types.slang#setting)
[Interface](cell:link-types.slang#IShape)
[Member](cell:link-types.slang#Outer.single)
[Overloads](cell:link-types.slang#Outer.work)
[Missing](cell:link-types.slang#removed)
[Whole cell](cell:link-types.slang)
[Caller](cell:link-caller.slang#main)
[Generic](cell:link-types.slang#identity)

??? example "Deep Dive: Definitions"

    <!-- module: link-types -->

    ```slang
    public static const float setting = 1.0;

    public interface IShape
    {
        float distance(float3 position);
    }

    public struct Outer
    {
        public float single(float value) { return value; }
        public float work(float value) { return value; }
        public int work(int value) { return value; }
    }

    public float target(float value) { return value + setting; }
    public T identity<T>(T value) { return value; }
    ```

<!-- runnable: link-caller backend=none -->

```slang
import "link-types";
[host] void main() { print(target(2.0)); }
```
