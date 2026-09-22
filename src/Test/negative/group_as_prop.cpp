// A group nested into a group stands where a property is expected. The name it builds,
// "first.nested", is not a key any storage registers: BuildMap walks two levels only.

#include "Test/negative/Config.h"

int main()
{
    const char* name = CPL_PROP_FULL_NAME(Negative::Config, first, nested);
    return name == NULL ? 1 : 0;
}
