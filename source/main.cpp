#include "git/core/core.h"

GMOD_MODULE_OPEN()
{
    Git::Core::Initialize(LUA);

    return 0;
}

GMOD_MODULE_CLOSE()
{
    Git::Core::Shutdown(LUA);

    return 0;
}