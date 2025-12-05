#include "core.h"
#include "../functions/functions.h"

#if defined GIT_32_SERVER
IFileSystem *g_pFullFileSystem = nullptr;
#endif

namespace Git::Core
{
void Initialize(GarrysMod::Lua::ILuaBase *LUA)
{
    g_pFullFileSystem = InterfacePointers::Internal::Server::FileSystem();

    if (!g_pFullFileSystem)
    {
        Logger::Log(Logger::Error("Failed to get {red}IFileSystem {white}pointer!"));
        return;
    }

    Logger::Log(Logger::Success("gmsv_git loaded."));
    Logger::Log(Logger::Info("Version: {green}" GIT_VERSION));
    git_libgit2_init();
    LUA->CreateTable();
    {
        LUA->PushCFunction(Functions::Clone);
        LUA->SetField(-2, "Clone");

        LUA->PushCFunction(Functions::Pull);
        LUA->SetField(-2, "Pull");

        LUA->PushCFunction(Functions::Checkout);
        LUA->SetField(-2, "Checkout");

        LUA->PushCFunction(Functions::Add);
        LUA->SetField(-2, "Add");

        LUA->PushCFunction(Functions::Commit);
        LUA->SetField(-2, "Commit");

        LUA->PushCFunction(Functions::Push);
        LUA->SetField(-2, "Push");

        LUA->PushCFunction(Functions::GetBranch);
        LUA->SetField(-2, "GetBranch");
    }
    LUA->SetField(GarrysMod::Lua::INDEX_GLOBAL, "git");
}

void Shutdown(GarrysMod::Lua::ILuaBase *LUA)
{
    Logger::Log(Logger::Info("Shutting down Git..."));
    git_libgit2_shutdown();
    LUA->PushNil();
    LUA->SetField(GarrysMod::Lua::INDEX_GLOBAL, "git");
}

std::string RelativePathToFullPath(const std::string &RelativePath)
{
    char RootFilePath[MAX_PATH];

    if (!g_pFullFileSystem->RelativePathToFullPath_safe("garrysmod/", nullptr, RootFilePath))
        return std::string();

    return std::string(RootFilePath) + RelativePath;
}
} // namespace Git::Core