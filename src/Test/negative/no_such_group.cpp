// The group does not exist in the configuration.

#include "Test/negative/Config.h"

int main()
{
    const char* name = CPL_PROP_FULL_NAME(Negative::Config, missing, width);
    return name == NULL ? 1 : 0;
}
