// A storage stands where a group is expected, so no storage registers "inner.height".

#include "Test/negative/Config.h"

int main()
{
    const char* name = CPL_PROP_FULL_NAME(Negative::StorageConfig, inner, height);
    return name == NULL ? 1 : 0;
}
