#pragma once
#include "../includes.h"
#include "../logger/logger.h"

namespace Git::Core
{
void Initialize(GarrysMod::Lua::ILuaBase *LUA);
void Shutdown(GarrysMod::Lua::ILuaBase *LUA);

std::string RelativePathToFullPath(const std::string &RelativePath);
} // namespace Git::Core