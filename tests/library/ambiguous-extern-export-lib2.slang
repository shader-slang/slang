
module "ambiguous-extern-export-lib2.slang";

public extern static const int call_data_len;
public extern static const int[call_data_len] call_group_vector;

public static int[call_data_len] call_id_2 = {};

public void initCallId2()
{
    for (int i = 0; i < call_data_len; i++)
    {
        call_id_2[i] = call_group_vector[i];
    }
}
