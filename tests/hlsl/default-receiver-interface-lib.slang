// Non-test helper module for default-receiver-on-immutable-value.hlsl (no //TEST: directive).
// Its interface requirement has the Slang dialect's default `in` receiver.
module default_receiver_interface_lib;

public interface IShape
{
    int area();
}
