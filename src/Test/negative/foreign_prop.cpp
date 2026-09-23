// The property exists, but in another group.

#include "Test/negative/Config.h"

int main()
{
    const char* name = CPL_PROP_FULL_NAME(Negative::Config, first, height);
    return name == NULL ? 1 : 0;
}
