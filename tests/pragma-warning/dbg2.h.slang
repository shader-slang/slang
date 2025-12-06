module "dbg2.h";

// load-bearing comment load-bearing comment load-bearing comment load-bearing comment
public bool bar(float a, out float b)
{
  if(a <= 0.0)
  {
#pragma warning(disable: 41018) // Disable warning 41018 : returning without initializing some variables/parameters
    return false;
#pragma warning(default: 41018)
  }

  b = a;

  return true;
}
