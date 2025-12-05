#include "functions.h"
#include "../core/core.h"
#include "../logger/logger.h"

namespace Git::Functions
{
LUA_FUNCTION(Clone)
{
    time_t Time = std::time(nullptr);
    std::string Token = GetGithubAccessToken();
    std::string URL = LUA->CheckString(1);
    std::string Directory = LUA->CheckString(2);
    std::string Path = Core::RelativePathToFullPath(Directory);
    std::string TempPath = Core::RelativePathToFullPath(std::string("TEMP_CLONE_").append(std::to_string(Time)));

    std::thread([=]() { HandleGitClone(URL, Directory, Path, TempPath, Token); }).detach();

    return 0;
}

LUA_FUNCTION(Pull)
{
    std::string Token = GetGithubAccessToken();
    std::string Directory = LUA->CheckString(1);
    std::string Path = Core::RelativePathToFullPath(Directory);

    std::thread([=]() { HandleGitPull(Directory, Path, Token); }).detach();

    return 0;
}

LUA_FUNCTION(Checkout)
{
    std::string Token = GetGithubAccessToken();
    std::string Directory = LUA->CheckString(1);
    std::string Path = Core::RelativePathToFullPath(Directory);
    std::string Head = LUA->CheckString(2);

    std::thread([=]() { HandleGitCheckout(Directory, Path, Head, Token); }).detach();

    return 0;
}

LUA_FUNCTION(Add)
{
    std::string Token = GetGithubAccessToken();
    std::string Directory = LUA->CheckString(1);
    std::string Path = Core::RelativePathToFullPath(Directory);
    std::string File = LUA->CheckString(2);

    std::thread([=]() { HandleGitAdd(Directory, Path, File, Token); }).detach();

    return 0;
}

LUA_FUNCTION(Commit)
{
    std::string Token = GetGithubAccessToken();
    std::string Directory = LUA->CheckString(1);
    std::string Path = Core::RelativePathToFullPath(Directory);
    std::string Message = LUA->CheckString(2);
    std::string AuthorName = LUA->IsType(3, GarrysMod::Lua::Type::String) ? LUA->GetString(3) : std::string();
    std::string AuthorEmail = LUA->IsType(4, GarrysMod::Lua::Type::String) ? LUA->GetString(4) : std::string();

    std::thread([=]() { HandleGitCommit(Directory, Path, Message, AuthorName, AuthorEmail, Token); }).detach();

    return 0;
}

LUA_FUNCTION(Push)
{
    std::string Token = GetGithubAccessToken();
    std::string Directory = LUA->CheckString(1);
    std::string Path = Core::RelativePathToFullPath(Directory);

    std::thread([=]() { HandleGitPush(Directory, Path, Token); }).detach();

    return 0;
}

LUA_FUNCTION(GetBranch)
{
    std::string Token = GetGithubAccessToken();
    std::string Directory = LUA->CheckString(1);
    std::string Path = Core::RelativePathToFullPath(Directory);
    GitRepository Repository(Path, Token);

    if (!Repository.Valid())
        return 0;

    std::string Branch = Repository.GetBranch();

    LUA->PushString(Branch.c_str());

    return 1;
}

LUA_FUNCTION(GetShortHash)
{
    std::string Token = GetGithubAccessToken();
    std::string Directory = LUA->CheckString(1);
    std::string Path = Core::RelativePathToFullPath(Directory);
    GitRepository Repository(Path, Token);

    if (!Repository.Valid())
        return 0;

    std::string Hash = Repository.GetShortHash();

    LUA->PushString(Hash.c_str());

    return 1;
}

std::string Pastelize(const std::string& Text)
{
    static std::string Colors[] = {
        "\x1B[38;5;225m",
        "\x1B[38;5;217m",
        "\x1B[38;5;223m",
        "\x1B[38;5;151m",
        "\x1B[38;5;159m",
        "\x1B[38;5;153m",
        "\x1B[38;5;189m"
    };

    std::string Output;
    int ColorIndex = 0;

    for (const char &Character : Text)
    {
        if (Character == ' ')
        {
            Output += Character;
            continue;
        }

        Output += Colors[ColorIndex];
        Output += Character;
        ColorIndex = (ColorIndex + 1) % 7;
    }

    Output += "\x1B[0m";
    return Output;
}

void FormatString(char *Buffer, const char *Format, ...)
{
    va_list Arguments;
    va_start(Arguments, Format);
    vsprintf(Buffer, Format, Arguments);
    va_end(Arguments);
}

std::string ProgressBar(double Percent)
{
    const int PROGRESS_BAR_SIZE = 40;
    int Progress = PROGRESS_BAR_SIZE * Percent;
    std::string Bar;

    for (int Index = 0; Index < PROGRESS_BAR_SIZE; ++Index)
        Bar.append(Index < Progress ? "=" : " ");

    char Buffer[128];
    FormatString(Buffer, "[%s] %d%%", Bar.c_str(), (int)(Percent * 100));

    return std::string(Buffer);
}

bool ShouldLogPercent(int PercentInt, int &LastPercent)
{
    if (LastPercent == -1)
    {
        LastPercent = PercentInt;

        return true;
    }

    if (PercentInt == LastPercent)
        return false;

    if (PercentInt >= 100)
    {
        LastPercent = 100;

        return true;
    }

    if (PercentInt == 0)
    {
        LastPercent = 0;

        return false;
    }

    if (PercentInt / 5 > LastPercent / 5)
    {
        LastPercent = PercentInt;

        return true;
    }

    return false;
}

int OnCloneFetchTransferProgress(const git_indexer_progress *Progress, void *)
{
    static int LastPercent = -1;

    if (Progress->total_objects == 0)
        return 0;

    double Percent = (double)Progress->received_objects / (double)Progress->total_objects;
    int PercentInt = (int)(Percent * 100);

    if (!ShouldLogPercent(PercentInt, LastPercent))
        return 0;

    std::string Bar = ProgressBar(Percent);

    Git::Logger::Log(Git::Logger::Info("Cloning repository: {yellow}%s"), Bar.c_str());

    LastPercent = PercentInt;

    return 0;
}

void OnCloneCheckoutProgress(const char *Path, size_t CompletedSteps, size_t TotalSteps, void *)
{
    static int LastPercent = -1;

    if (TotalSteps == 0)
        return;

    double Percent = (double)CompletedSteps / (double)TotalSteps;
    int PercentInt = (int)(Percent * 100);

    if (!ShouldLogPercent(PercentInt, LastPercent))
        return;

    std::string Bar = ProgressBar(Percent);

    Git::Logger::Log(Git::Logger::Info("Checkout: {yellow}%d{white}/{yellow}%d {white}%s"), CompletedSteps, TotalSteps,
                     Bar.c_str());

    LastPercent = PercentInt;
}

int DiffSummaryCallback(const git_diff_delta *Delta, float, void *)
{
    const char *OldPath = Delta->old_file.path ? Delta->old_file.path : "";
    const char *NewPath = Delta->new_file.path ? Delta->new_file.path : "";

    switch (Delta->status)
    {
    case GIT_DELTA_MODIFIED:
        Git::Logger::Log(Git::Logger::Info(" M %s"), NewPath);
        break;
    case GIT_DELTA_ADDED:
        Git::Logger::Log(Git::Logger::Info(" A %s"), NewPath);
        break;
    case GIT_DELTA_DELETED:
        Git::Logger::Log(Git::Logger::Info(" D %s"), OldPath);
        break;
    case GIT_DELTA_RENAMED:
        Git::Logger::Log(Git::Logger::Info(" R {cyan}%s{white} -> {cyan}%s"), OldPath, NewPath);
        break;
    case GIT_DELTA_COPIED:
        Git::Logger::Log(Git::Logger::Info(" C {cyan}%s{white} -> {cyan}%s"), OldPath, NewPath);
        break;
    }

    return 0;
}

int CredentialToken(git_credential **Output, const char *, const char *, unsigned int, void *Payload)
{
    const char *Token = (const char *)Payload;

    return git_credential_userpass_plaintext_new(Output, "git", Token);
}

int CertificateCheck(git_cert *, int, const char *, void *)
{
    return 0;
}

std::string GetGithubAccessToken()
{
    std::string TokenPath = Git::Core::RelativePathToFullPath("git.token");

    if (!std::filesystem::exists(TokenPath))
    {
        return std::string();
    }

    std::ifstream TokenFile(TokenPath);
    std::string Token;
    TokenFile >> Token;

    return Token;
}

void PrintGitDiffSummary(git_repository *Repository, const git_oid &OldOid, const git_oid &NewOid)
{
    git_commit *OldCommit = nullptr, *NewCommit = nullptr;
    git_tree *OldTree = nullptr, *NewTree = nullptr;
    git_diff *Difference = nullptr;
    char A[16], B[16];

    git_commit_lookup(&OldCommit, Repository, &OldOid);
    git_commit_lookup(&NewCommit, Repository, &NewOid);
    git_commit_tree(&OldTree, OldCommit);
    git_commit_tree(&NewTree, NewCommit);
    git_diff_tree_to_tree(&Difference, Repository, OldTree, NewTree, nullptr);

    size_t Count = git_diff_num_deltas(Difference);

    git_oid_tostr(A, sizeof(A), &OldOid);
    git_oid_tostr(B, sizeof(B), &NewOid);
    Git::Logger::Log(Git::Logger::Info("Updating {yellow}%s{white}..{yellow}%s"), A, B);

    if (Count > 200)
        Git::Logger::Log(Git::Logger::Info("Fast-forward"));
    else
        git_diff_foreach(Difference, DiffSummaryCallback, nullptr, nullptr, nullptr, nullptr);

    git_diff_free(Difference);
    git_tree_free(OldTree);
    git_tree_free(NewTree);
    git_commit_free(OldCommit);
    git_commit_free(NewCommit);
}

void CopyFilesInto(const std::filesystem::path &Source, const std::filesystem::path &Destination)
{
    for (const auto &Entry : std::filesystem::recursive_directory_iterator(Source))
    {
        std::filesystem::path EntryPath = Entry.path();

        std::filesystem::path RelativePath = std::filesystem::relative(EntryPath, Source);
        std::filesystem::path DestinationPath = Destination / RelativePath;
        std::error_code ErrorCode;

        if (Entry.is_directory())
        {
            std::filesystem::create_directories(DestinationPath, ErrorCode);
            continue;
        }

        std::filesystem::create_directories(DestinationPath.parent_path(), ErrorCode);

        std::filesystem::copy_file(EntryPath, DestinationPath,
                                   std::filesystem::copy_options::overwrite_existing |
                                       std::filesystem::copy_options::copy_symlinks,
                                   ErrorCode);
    }
}

void HandleGitClone(std::string URL, std::string Directory, std::string Path, std::string TempPath, std::string Token)
{
    Logger::Log(Logger::Info("Cloning repository {cyan}%s{white} to {yellow}%s{white}..."), URL.c_str(), Path.c_str());

    git_repository *Repository = nullptr;
    git_clone_options Options = GIT_CLONE_OPTIONS_INIT;

    Options.fetch_opts.callbacks.transfer_progress = OnCloneFetchTransferProgress;
    Options.checkout_opts.progress_cb = OnCloneCheckoutProgress;
    Options.checkout_opts.progress_payload = nullptr;
    Options.fetch_opts.callbacks.certificate_check = CertificateCheck;

    if (!Token.empty())
    {
        const char *TokenPtr = Token.c_str();
        Options.fetch_opts.callbacks.credentials = CredentialToken;
        Options.fetch_opts.callbacks.payload = (void *)TokenPtr;
    }

    int Error = git_clone(&Repository, URL.c_str(), TempPath.c_str(), &Options);

    if (Error != 0)
    {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to clone repository {cyan}%s{white} to {yellow}%s{white}: {red}%s"),
                    URL.c_str(), Path.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        git_repository_free(Repository);

        return;
    }

    git_repository_free(Repository);

    if (!std::filesystem::exists(Path))
    {
        std::filesystem::rename(TempPath, Path);
    }
    else
    {
        Logger::Log(Logger::Info("Copying files into {cyan}%s{white}..."), Path.c_str());
        CopyFilesInto(TempPath, Path);
        std::filesystem::remove_all(TempPath);
    }

    Logger::Log(Logger::Success("Repository cloned successfully {cyan}%s{white} to {yellow}%s{white}."), URL.c_str(),
                Path.c_str());
}

void HandleGitPull(std::string Directory, std::string Path, std::string Token)
{
    GitRepository Repository(Path, Token);

    if (!Repository.Valid())
    {
        Logger::Log(Logger::Error("Not a valid Git repository {cyan}%s{white}."), Path.c_str());
        return;
    }

    GitCodes Code = Repository.Pull();

    switch (Code)
    {
    case GitCodes::FAST_FORWARD_SUCCESS: {
        Logger::Log(Logger::Success("Repository {cyan}%s{white} fast-forwarded."), Path.c_str());
        break;
    }
    case GitCodes::MERGE_SUCCESS: {
        Logger::Log(Logger::Success("Repository {cyan}%s{white} merged successfully."), Path.c_str());
        break;
    }
    case GitCodes::UP_TO_DATE: {
        Logger::Log(Logger::Success("Repository {cyan}%s{white} up to date."), Path.c_str());
        break;
    }
    case GitCodes::ORIGIN_LOOKUP_FAILED: {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to lookup origin {cyan}%s{white}: {red}%s"), Path.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    case GitCodes::REMOTE_FETCH_FAILED: {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to fetch remote {cyan}%s{white}: {red}%s"), Path.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    case GitCodes::HEAD_FETCH_FAILED: {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to fetch head {cyan}%s{white}: {red}%s"), Path.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    case GitCodes::HEAD_READ_FAILED: {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to read head {cyan}%s{white}: {red}%s"), Path.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    case GitCodes::HEAD_LOOKUP_FAILED: {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to lookup head {cyan}%s{white}: {red}%s"), Path.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    case GitCodes::FAST_FORWARD_FAILED: {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to fast-forward {cyan}%s{white}: {red}%s"), Path.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    case GitCodes::TREE_LOOKUP_FAILED: {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to lookup tree {cyan}%s{white}: {red}%s"), Path.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    case GitCodes::MERGE_CONFLICTS_FOUND: {
        Logger::Log(Logger::Error("Merge conflicts found in {cyan}%s{white}. Please resolve conflicts before pulling."),
                    Path.c_str());

        break;
    }
    case GitCodes::MERGE_FAILED: {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to merge {cyan}%s{white}: {red}%s"), Path.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    }
}

void HandleGitCheckout(std::string Directory, std::string Path, std::string Head, std::string Token)
{
    GitRepository Repository(Path, Token);

    if (!Repository.Valid())
    {
        Logger::Log(Logger::Error("Not a valid Git repository {cyan}%s{white}."), Path.c_str());
        return;
    }

    GitCodes Code = Repository.Checkout(Head);

    switch (Code)
    {
    case GitCodes::CHECKOUT_SUCCESS: {
        Logger::Log(Logger::Success("Checked out {cyan}%s{white} in {yellow}%s{white}."), Head.c_str(), Path.c_str());

        break;
    }
    case GitCodes::TARGET_LOOKUP_FAILED: {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to lookup target {cyan}%s{white}: {red}%s"), Head.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    case GitCodes::TREE_INDEX_FAILED: {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to index tree {cyan}%s{white}: {red}%s"), Head.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    case GitCodes::CHECKOUT_FAILED: {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to checkout {cyan}%s{white}: {red}%s"), Head.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    }
}

void HandleGitAdd(std::string Directory, std::string Path, std::string File, std::string Token)
{
    GitRepository Repository(Path, Token);

    if (!Repository.Valid())
    {
        Logger::Log(Logger::Error("Not a valid Git repository {cyan}%s{white}."), Path.c_str());
        return;
    }

    GitCodes Code = Repository.Add(File, Path);

    switch (Code)
    {
    case GitCodes::ADD_SUCCESS: {
        Logger::Log(Logger::Success("Added {yellow}%s{white} to {cyan}%s{white}."), File.c_str(), Path.c_str());
        break;
    }
    case GitCodes::NOTHING_TO_ADD: {
        Logger::Log(Logger::Info("No changes to add in {cyan}%s{white}."), Path.c_str());
        break;
    }
    case GitCodes::REPOSITORY_INDEX_FAILED: {
        const git_error *ErrorStack = git_error_last();
        Logger::Log(Logger::Error("Failed to index repository {cyan}%s{white}: {red}%s"), Path.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    case GitCodes::FILE_NOT_FOUND: {
        Logger::Log(Logger::Error("File {cyan}%s{white} not found in {yellow}%s{white}."), File.c_str(), Path.c_str());

        break;
    }
    case GitCodes::ADD_FAILED: {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to add {cyan}%s{white} to {yellow}%s{white}: {red}%s"), File.c_str(),
                    Path.c_str(), (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    }
}

void HandleGitCommit(std::string Directory, std::string Path, std::string Message, std::string AuthorName,
                     std::string AuthorEmail, std::string Token)
{
    GitRepository Repository(Path, Token);

    if (!Repository.Valid())
    {
        Logger::Log(Logger::Error("Not a valid Git repository {cyan}%s{white}."), Path.c_str());
        return;
    }

    GitCodes Code = Repository.Commit(Message, AuthorName, AuthorEmail);

    switch (Code)
    {
    case GitCodes::COMMIT_SUCCESS: {
        std::string Branch = Repository.GetBranch();
        std::string Hash = Repository.GetShortHash();

        Logger::Log(Logger::Success("[{cyan}%s {yellow}%s{white}] {gray}%s{white}"), Branch.c_str(), Hash.c_str(),
                    Message.c_str());
        break;
    }
    case GitCodes::NOTHING_TO_COMMIT: {
        Logger::Log(Logger::Info("Nothing to commit in {cyan}%s{white}."), Path.c_str());
        break;
    }
    case GitCodes::REPOSITORY_INDEX_FAILED: {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to index repository {cyan}%s{white}: {red}%s"), Path.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    case GitCodes::COMMIT_FAILED: {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to commit {cyan}%s{white}: {red}%s"), Message.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    }
}

void HandleGitPush(std::string Directory, std::string Path, std::string Token)
{
    GitRepository Repository(Path, Token);

    if (!Repository.Valid())
    {
        Logger::Log(Logger::Error("Not a valid Git repository {cyan}%s{white}."), Path.c_str());
        return;
    }

    GitCodes Code = Repository.Push();

    switch (Code)
    {
    case GitCodes::NOTHING_TO_PUSH: {
        Logger::Log(Logger::Info("Nothing to push in {cyan}%s{white}."), Path.c_str());
        break;
    }
    case GitCodes::ORIGIN_LOOKUP_FAILED: {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to lookup origin {cyan}%s{white}: {red}%s"), Path.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    case GitCodes::REMOTE_FETCH_FAILED: {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to fetch remote {cyan}%s{white}: {red}%s"), Path.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    case GitCodes::PUSH_FAILED: {
        const git_error *ErrorStack = git_error_last();

        Logger::Log(Logger::Error("Failed to push {cyan}%s{white}: {red}%s"), Path.c_str(),
                    (ErrorStack && ErrorStack->message) ? ErrorStack->message : "Unknown error");

        break;
    }
    }
}
} // namespace Git::Functions