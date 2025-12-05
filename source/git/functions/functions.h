#pragma once
#include "../includes.h"
#include "../git/git.h"

namespace Git::Functions
{
int Clone(lua_State *L);
int Pull(lua_State *L);
int Checkout(lua_State *L);
int Add(lua_State *L);
int Commit(lua_State *L);
int Push(lua_State *L);
int GetBranch(lua_State *L);
int GetShortHash(lua_State *L);

std::string Pastelize(const std::string& Text);
void FormatString(char *Buffer, const char *Format, ...);
std::string ProgressBar(double Percent);
bool ShouldLogPercent(int PercentInt, int &LastPercent);
int OnCloneFetchTransferProgress(const git_indexer_progress *Progress, void *);
void OnCloneCheckoutProgress(const char *Path, size_t CompletedSteps, size_t TotalSteps, void *);
int CredentialToken(git_credential **Out, const char *, const char *, unsigned int AllowedTypes, void *Payload);
int CertificateCheck(git_cert *, int, const char *, void *);
std::string GetGithubAccessToken();
void PrintGitDiffSummary(git_repository *Repository, const git_oid &OldOid, const git_oid &NewOid);
void CopyFilesInto(const std::filesystem::path &Source, const std::filesystem::path &Destination);
void HandleGitClone(std::string URL, std::string Directory, std::string Path, std::string TempPath, std::string Token);
void HandleGitPull(std::string Directory, std::string Path, std::string Token);
void HandleGitCheckout(std::string Directory, std::string Path, std::string Head, std::string Token);
void HandleGitAdd(std::string Directory, std::string Path, std::string File, std::string Token);
void HandleGitCommit(std::string Directory, std::string Path, std::string Message, std::string AuthorName, std::string AuthorEmail, std::string Token);
void HandleGitPush(std::string Directory, std::string Path, std::string Token);
} // namespace Git::Functions