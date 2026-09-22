// A property stands where a group is expected.

#include "Test/negative/Config.h"

int main()
{
    const char* name = CPL_PROP_FULL_NAME(Negative::Config, standalone, width);
    return name == NULL ? 1 : 0;
}
