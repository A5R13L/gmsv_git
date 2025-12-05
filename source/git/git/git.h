#pragma once
#include "../includes.h"

enum GitCodes
{
    FAST_FORWARD_SUCCESS = 0,
    MERGE_SUCCESS,
    CHECKOUT_SUCCESS,
    ADD_SUCCESS,
    COMMIT_SUCCESS,
    PUSH_SUCCESS,
    UP_TO_DATE,
    NOTHING_TO_ADD,
    NOTHING_TO_COMMIT,
    NOTHING_TO_PUSH,
    ORIGIN_LOOKUP_FAILED,
    REMOTE_FETCH_FAILED,
    HEAD_FETCH_FAILED,
    HEAD_READ_FAILED,
    HEAD_LOOKUP_FAILED,
    FAST_FORWARD_FAILED,
    TREE_LOOKUP_FAILED,
    MERGE_CONFLICTS_FOUND,
    MERGE_FAILED,
    TARGET_LOOKUP_FAILED,
    TREE_INDEX_FAILED,
    CHECKOUT_FAILED,
    REPOSITORY_INDEX_FAILED,
    FILE_NOT_FOUND,
    ADD_FAILED,
    COMMIT_FAILED,
    PUSH_FAILED
};

class GitRepository
{
  public:
    GitRepository(const std::string &Path, const std::string &Token);
    ~GitRepository();
    git_repository *GetRepository();
    bool Valid();
    std::string GetBranch();
    std::string GetShortHash();
    std::string GetToken();
    GitCodes Pull();
    GitCodes Checkout(const std::string &Head);
    GitCodes Add(const std::string &File, const std::string &Path);
    GitCodes Commit(const std::string &Message, const std::string &AuthorName, const std::string &AuthorEmail);
    GitCodes Push();

  private:
    git_repository *Repository;
    std::string Token;
};