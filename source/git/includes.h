#pragma once
#include <GarrysMod/Lua/LuaBase.h>
#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/InterfacePointers.hpp>
#include <filesystem_base.h>
#include <string>
#include <iostream>
#include <functional>
#include <filesystem>
#include <fstream>
#include <thread>
#include <map>
#include <git2.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifndef GIT_VERSION
#define GIT_VERSION "unknown"
#endif

namespace Git
{
} // namespace Git