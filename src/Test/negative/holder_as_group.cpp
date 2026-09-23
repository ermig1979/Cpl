// The group is declared with CPL_PARAM_HOLDER and names its node "renamed", so the key the storage
// registers is "renamed.height" while the macro builds "group.height".

#include "Test/negative/Config.h"

int main()
{
    const char* name = CPL_PROP_FULL_NAME(Negative::HolderConfig, group, height);
    return name == NULL ? 1 : 0;
}
