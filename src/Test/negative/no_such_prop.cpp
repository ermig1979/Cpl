// The property does not exist in the group.

#include "Test/negative/Config.h"

int main()
{
    const char* name = CPL_PROP_FULL_NAME(Negative::Config, first, missing);
    return name == NULL ? 1 : 0;
}
