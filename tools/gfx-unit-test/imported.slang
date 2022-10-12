struct TestFunction
            {
                void simpleElementAdd(RWStructuredBuffer<float> buffer, uint index)
                {
                    var input = buffer[index];
                    buffer[index] = input + 1.0f;
                }
            };