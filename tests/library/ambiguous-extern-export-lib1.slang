module "ambiguous-extern-export-lib1.slang";

public extern static const int call_data_len;
public extern static const int[call_data_len] call_group_vector;
public static int[call_data_len] call_id_1 = {};

public void initCallId1()
{
    for (int i = 0; i < call_data_len; i++)
    {
        call_id_1[i] = call_group_vector[i];
    }
}
