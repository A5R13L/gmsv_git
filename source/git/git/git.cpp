#include "git.h"
#include "../functions/functions.h"
#include "../logger/logger.h"

class GitRemote
{
  public:
    GitRemote(git_repository *Repository, const char *Name) : Repository(Repository), Remote(nullptr)
    {
        if (git_remote_lookup(&Remote, Repository, Name) != 0)
        {
            git_remote_free(Remote);
            Remote = nullptr;
        }
    }

    ~GitRemote()
    {
        if (Remote)
            git_remote_free(Remote);

        Remote = nullptr;
    }

    git_remote *GetRemote()
    {
        return Remote;
    }

  private:
    git_repository *Repository;
    git_remote *Remote;
};

class GitAnnotatedCommit
{
  public:
    GitAnnotatedCommit(git_repository *Repository, const char *RemoteBranchName)
        : Repository(Repository), AnnotatedCommit(nullptr)
    {
        git_reference *Reference = nullptr;

        if (git_reference_lookup(&Reference, Repository, RemoteBranchName) == 0)
            if (git_annotated_commit_from_ref(&AnnotatedCommit, Repository, Reference) != 0)
            {
                git_annotated_commit_free(AnnotatedCommit);
                AnnotatedCommit = nullptr;
            }

        git_reference_free(Reference);
    }

    ~GitAnnotatedCommit()
    {
        if (AnnotatedCommit)
            git_annotated_commit_free(AnnotatedCommit);

        AnnotatedCommit = nullptr;
    }

    git_annotated_commit *GetAnnotatedCommit()
    {
        return AnnotatedCommit;
    }

  private:
    git_repository *Repository;
    git_annotated_commit *AnnotatedCommit;
};

class GitHead
{
  public:
    GitHead(git_repository *Repository) : Repository(Repository), Head(nullptr)
    {
        if (git_repository_head(&Head, Repository) != 0)
        {
            git_reference_free(Head);
            Head = nullptr;
        }
    }

    ~GitHead()
    {
        if (Head)
            git_reference_free(Head);

        Head = nullptr;
    }

    git_reference *GetHead()
    {
        return Head;
    }

  private:
    git_repository *Repository;
    git_reference *Head;
};

class GitTree
{
  public:
    GitTree(git_repository *Repository, const git_oid *Id) : Repository(Repository), Tree(nullptr)
    {
        if (git_tree_lookup(&Tree, Repository, Id) != 0)
        {
            git_tree_free(Tree);
            Tree = nullptr;
        }
    }

    ~GitTree()
    {
        if (Tree)
            git_tree_free(Tree);

        Tree = nullptr;
    }

    git_tree *GetTree()
    {
        return Tree;
    }

  private:
    git_repository *Repository;
    git_tree *Tree;
};

class GitParsedTree
{
  public:
    GitParsedTree(git_repository *Repository, const char *Path) : Repository(Repository), Object(nullptr)
    {
        if (git_revparse_single(&Object, Repository, Path) != 0)
        {
            git_object_free(Object);
            Object = nullptr;
        }
    }

    ~GitParsedTree()
    {
        if (Object)
            git_object_free(Object);

        Object = nullptr;
    }

    git_object *GetObject()
    {
        return Object;
    }

  private:
    git_repository *Repository;
    git_object *Object;
};

class GitFastForward
{
  public:
    GitFastForward(git_reference *Head, git_oid *Id) : Head(Head), Id(Id), NewHead(nullptr)
    {
        if (git_reference_set_target(&NewHead, Head, Id, "Fast-forward merge") != 0)
        {
            git_reference_free(NewHead);
            NewHead = nullptr;
        }
    }

    ~GitFastForward()
    {
        if (NewHead)
            git_reference_free(NewHead);

        NewHead = nullptr;
    }

    git_reference *GetNewHead()
    {
        return NewHead;
    }

  private:
    git_reference *Head;
    git_oid *Id;
    git_reference *NewHead;
};

class GitIndex
{
  public:
    GitIndex(git_repository *Repository) : Repository(Repository), Index(nullptr)
    {
        if (git_repository_index(&Index, Repository) != 0)
        {
            git_index_free(Index);
            Index = nullptr;
        }
    }

    ~GitIndex()
    {
        if (Index)
            git_index_free(Index);

        Index = nullptr;
    }

    git_index *GetIndex()
    {
        return Index;
    }

  private:
    git_repository *Repository;
    git_index *Index;
};

class GitSignature
{
  public:
    GitSignature(git_repository *Repository, const char *Name, const char *Email)
        : Repository(Repository), Signature(nullptr)
    {
        if (git_signature_now(&Signature, Name, Email) != 0)
        {
            git_signature_free(Signature);
            Signature = nullptr;
        }
    }

    ~GitSignature()
    {
        if (Signature)
            git_signature_free(Signature);

        Signature = nullptr;
    }

    git_signature *GetSignature()
    {
        return Signature;
    }

  private:
    git_repository *Repository;
    git_signature *Signature;
};

class GitCommit
{
  public:
    GitCommit(git_repository *Repository, const git_oid *Id) : Repository(Repository), Commit(nullptr)
    {
        if (git_commit_lookup(&Commit, Repository, Id) != 0)
        {
            git_commit_free(Commit);
            Commit = nullptr;
        }
    }

    ~GitCommit()
    {
        if (Commit)
            git_commit_free(Commit);

        Commit = nullptr;
    }

    git_commit *GetCommit()
    {
        return Commit;
    }

  private:
    git_repository *Repository;
    git_commit *Commit;
};

GitRepository::GitRepository(const std::string &Path, const std::string &Token) : Repository(nullptr), Token(Token)
{
    int Error = git_repository_open(&Repository, Path.c_str());

    if (Error != 0)
    {
        git_repository_free(Repository);
        Repository = nullptr;
        return;
    }
}

GitRepository::~GitRepository()
{
    if (Repository)
        git_repository_free(Repository);

    Repository = nullptr;
}

git_repository *GitRepository::GetRepository()
{
    return Repository;
}

bool GitRepository::Valid()
{
    return Repository != nullptr;
}

std::string GitRepository::GetBranch()
{
    if (!Repository)
        return std::string();

    const char *BranchName = "HEAD";
    git_reference *Head = nullptr;

    if (git_repository_head(&Head, Repository) == 0)
        if (git_branch_name(&BranchName, Head) != 0)
        {
            git_reference_free(Head);
            Head = nullptr;
            return std::string();
        }

    if (Head)
        git_reference_free(Head);

    return std::string(BranchName);
}

std::string GitRepository::GetShortHash()
{
    if (!Repository)
        return std::string();

    git_oid Oid;

    if (git_reference_name_to_id(&Oid, Repository, "HEAD") != 0)
        return std::string();

    char ShortHash[41];

    if (git_oid_tostr(ShortHash, sizeof(ShortHash), &Oid) == nullptr)
        return std::string();

    return std::string(ShortHash, 7);
}

std::string GitRepository::GetToken()
{
    return Token;
}

GitCodes GitRepository::Pull()
{
    if (!Repository)
        return GitCodes::HEAD_LOOKUP_FAILED;

    git_fetch_options FetchOptions = GIT_FETCH_OPTIONS_INIT;
    git_merge_options MergeOptions = GIT_MERGE_OPTIONS_INIT;
    git_checkout_options CheckoutOptions = GIT_CHECKOUT_OPTIONS_INIT;

    if (!Token.empty())
    {
        const char *TokenPtr = Token.c_str();
        FetchOptions.callbacks.credentials = Git::Functions::CredentialToken;
        FetchOptions.callbacks.payload = (void *)TokenPtr;
    }

    FetchOptions.callbacks.certificate_check = Git::Functions::CertificateCheck;

    GitHead LocalHead(Repository);

    if (!LocalHead.GetHead())
        return GitCodes::HEAD_LOOKUP_FAILED;

    const git_oid *OldHeadPtr = git_reference_target(LocalHead.GetHead());
    git_oid OldHeadOid = OldHeadPtr ? *OldHeadPtr : git_oid{};

    const char *LocalBranchName = git_reference_shorthand(LocalHead.GetHead());
    std::string LocalBranchRef = std::string("refs/heads/").append(LocalBranchName);
    std::string RemoteBranchRef = std::string("refs/remotes/origin/").append(LocalBranchName);
    GitRemote Remote(Repository, "origin");

    if (!Remote.GetRemote())
        return GitCodes::ORIGIN_LOOKUP_FAILED;

    if (git_remote_fetch(Remote.GetRemote(), nullptr, &FetchOptions, nullptr) != 0)
        return GitCodes::REMOTE_FETCH_FAILED;

    GitAnnotatedCommit RemoteCommit(Repository, RemoteBranchRef.c_str());

    if (!RemoteCommit.GetAnnotatedCommit())
        return GitCodes::HEAD_READ_FAILED;

    const git_annotated_commit *Commits[] = {RemoteCommit.GetAnnotatedCommit()};
    git_merge_analysis_t Analysis;
    git_merge_preference_t Preference;

    if (git_merge_analysis(&Analysis, &Preference, Repository, Commits, 1) != 0)
        return GitCodes::MERGE_FAILED;

    if (Analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE)
        return GitCodes::UP_TO_DATE;

    const git_oid *TargetOid = git_annotated_commit_id(RemoteCommit.GetAnnotatedCommit());

    if (Analysis & GIT_MERGE_ANALYSIS_FASTFORWARD)
    {
        Git::Functions::PrintGitDiffSummary(Repository, OldHeadOid, *TargetOid);

        GitCommit TargetCommit(Repository, TargetOid);

        if (!TargetCommit.GetCommit())
            return GitCodes::FAST_FORWARD_FAILED;

        git_tree *RawTree = nullptr;

        if (git_commit_tree(&RawTree, TargetCommit.GetCommit()) != 0)
        {
            git_tree_free(RawTree);
            return GitCodes::FAST_FORWARD_FAILED;
        }

        GitTree TargetTree(Repository, git_object_id((git_object *)RawTree));
        git_tree_free(RawTree);

        GitIndex Index(Repository);

        if (git_index_read_tree(Index.GetIndex(), TargetTree.GetTree()) != 0)
            return GitCodes::FAST_FORWARD_FAILED;

        if (git_index_write(Index.GetIndex()) != 0)
            return GitCodes::FAST_FORWARD_FAILED;

        CheckoutOptions.checkout_strategy = GIT_CHECKOUT_SAFE;

        if (git_checkout_index(Repository, Index.GetIndex(), &CheckoutOptions) != 0)
            return GitCodes::FAST_FORWARD_FAILED;

        git_reference *BranchReference = nullptr;

        if (git_reference_lookup(&BranchReference, Repository, LocalBranchRef.c_str()) != 0)
        {
            if (git_reference_create(&BranchReference, Repository, LocalBranchRef.c_str(), TargetOid, 0, nullptr) != 0)
            {
                git_reference_free(BranchReference);
                return GitCodes::FAST_FORWARD_FAILED;
            }
        }

        git_reference *UpdatedReference = nullptr;

        if (git_reference_set_target(&UpdatedReference, BranchReference, TargetOid, "fast-forward") != 0)
        {
            git_reference_free(BranchReference);
            git_reference_free(UpdatedReference);
            return GitCodes::FAST_FORWARD_FAILED;
        }

        git_reference_free(BranchReference);
        git_reference_free(UpdatedReference);
        git_repository_set_head(Repository, LocalBranchRef.c_str());
        return GitCodes::FAST_FORWARD_SUCCESS;
    }

    if (Analysis & GIT_MERGE_ANALYSIS_NORMAL)
    {
        CheckoutOptions.checkout_strategy = GIT_CHECKOUT_SAFE;
        Git::Functions::PrintGitDiffSummary(Repository, OldHeadOid, *TargetOid);

        if (git_merge(Repository, Commits, 1, &MergeOptions, &CheckoutOptions) != 0)
            return GitCodes::MERGE_FAILED;

        GitIndex Index(Repository);

        if (git_index_has_conflicts(Index.GetIndex()))
            return GitCodes::MERGE_CONFLICTS_FOUND;

        git_oid TreeOid;

        if (git_index_write_tree(&TreeOid, Index.GetIndex()) != 0)
            return GitCodes::MERGE_FAILED;

        GitTree Tree(Repository, &TreeOid);
        GitSignature Signature(Repository, "GMSV-GIT", "server@local");
        GitHead ParentHead(Repository);
        GitCommit ParentCommit(Repository, git_reference_target(ParentHead.GetHead()));
        git_oid CommitOid;

        if (git_commit_create_v(&CommitOid, Repository, "HEAD", Signature.GetSignature(), Signature.GetSignature(),
                                nullptr, "Merge remote changes", Tree.GetTree(), 1, ParentCommit.GetCommit()) != 0)
            return GitCodes::MERGE_FAILED;

        return GitCodes::MERGE_SUCCESS;
    }

    return GitCodes::UP_TO_DATE;
}

GitCodes GitRepository::Checkout(const std::string &Head)
{
    if (!Repository)
        return GitCodes::TARGET_LOOKUP_FAILED;

    GitParsedTree Tree(Repository, Head.c_str());

    if (!Tree.GetObject())
        return GitCodes::TREE_INDEX_FAILED;

    git_oid TreeOid = *git_object_id(Tree.GetObject());
    GitCommit Commit(Repository, &TreeOid);

    if (!Commit.GetCommit())
        return GitCodes::TREE_INDEX_FAILED;

    git_tree *RawTree = nullptr;

    if (git_commit_tree(&RawTree, Commit.GetCommit()) != 0)
        return GitCodes::CHECKOUT_FAILED;

    GitTree CheckoutTree(Repository, git_object_id((git_object *)RawTree));
    GitIndex Index(Repository);
    git_tree_free(RawTree);

    if (git_index_read_tree(Index.GetIndex(), CheckoutTree.GetTree()) != 0)
        return GitCodes::CHECKOUT_FAILED;

    git_checkout_options CheckoutOptions = GIT_CHECKOUT_OPTIONS_INIT;
    CheckoutOptions.checkout_strategy = GIT_CHECKOUT_SAFE;

    if (git_checkout_index(Repository, Index.GetIndex(), &CheckoutOptions) != 0)
        return GitCodes::CHECKOUT_FAILED;

    std::string BranchRef = std::string("refs/heads/") + Head;
    git_reference *Temp = nullptr;

    if (git_reference_lookup(&Temp, Repository, BranchRef.c_str()) == 0)
        git_repository_set_head(Repository, BranchRef.c_str());
    else
        git_repository_set_head_detached(Repository, &TreeOid);

    if (Temp)
        git_reference_free(Temp);

    return GitCodes::CHECKOUT_SUCCESS;
}

GitCodes GitRepository::Add(const std::string &File, const std::string &Path)
{
    if (!Repository)
        return GitCodes::ADD_FAILED;

    git_index *Index = nullptr;
    git_tree *OldTree = nullptr;
    git_tree *NewTree = nullptr;
    git_commit *HeadCommit = nullptr;
    git_oid OldTreeOid, NewTreeOid, ParentOid;

    if (git_repository_index(&Index, Repository) != 0)
        return GitCodes::REPOSITORY_INDEX_FAILED;

    bool IsAll = (File == "." || File == "*");

    if (git_reference_name_to_id(&ParentOid, Repository, "HEAD") == 0 &&
        git_commit_lookup(&HeadCommit, Repository, &ParentOid) == 0)
        git_commit_tree(&OldTree, HeadCommit);

    if (IsAll)
    {
        if (git_index_add_all(Index, nullptr, GIT_INDEX_ADD_DEFAULT, nullptr, nullptr) != 0)
            goto AddFail;
    }
    else
    {
        std::string FullPath = Path;
        if (Path.back() != '/')
            FullPath.push_back('/');
        FullPath.append(File);

        if (!std::filesystem::exists(FullPath))
            return GitCodes::FILE_NOT_FOUND;

        if (git_index_add_bypath(Index, File.c_str()) != 0)
            goto AddFail;
    }

    if (git_index_write_tree(&NewTreeOid, Index) != 0)
        goto AddFail;

    if (git_tree_lookup(&NewTree, Repository, &NewTreeOid) != 0)
        goto AddFail;

    if (OldTree && git_oid_equal(git_tree_id(OldTree), &NewTreeOid))
    {
        git_index_free(Index);
        git_tree_free(OldTree);
        git_tree_free(NewTree);
        git_commit_free(HeadCommit);

        return GitCodes::NOTHING_TO_ADD;
    }

    if (git_index_write(Index) != 0)
        goto AddFail;

    git_index_free(Index);
    git_tree_free(OldTree);
    git_tree_free(NewTree);
    git_commit_free(HeadCommit);

    return GitCodes::ADD_SUCCESS;

AddFail:
    git_index_free(Index);
    git_tree_free(OldTree);
    git_tree_free(NewTree);
    git_commit_free(HeadCommit);

    return GitCodes::ADD_FAILED;
}

GitCodes GitRepository::Commit(const std::string &Message, const std::string &AuthorName,
                               const std::string &AuthorEmail)
{
    if (!Repository)
        return GitCodes::COMMIT_FAILED;

    git_index *Index = nullptr;
    git_tree *Tree = nullptr;
    git_tree *HeadTree = nullptr;
    git_signature *Signature = nullptr;
    git_commit *HeadCommit = nullptr;
    git_diff *Diff = nullptr;
    git_diff_stats *Stats = nullptr;
    git_tree *ParentTree = nullptr;
    git_oid TreeOid, CommitOid, ParentOid;

    const char *Name = AuthorName.empty() ? "server" : AuthorName.c_str();
    const char *Email = AuthorEmail.empty() ? "server@local" : AuthorEmail.c_str();

    if (git_repository_index(&Index, Repository) != 0)
        return GitCodes::REPOSITORY_INDEX_FAILED;

    if (git_index_write_tree(&TreeOid, Index) != 0)
        goto CommitFail;

    if (git_tree_lookup(&Tree, Repository, &TreeOid) != 0)
        goto CommitFail;

    if (git_reference_name_to_id(&ParentOid, Repository, "HEAD") == 0 &&
        git_commit_lookup(&HeadCommit, Repository, &ParentOid) == 0 && git_commit_tree(&HeadTree, HeadCommit) == 0 &&
        git_oid_equal(git_tree_id(HeadTree), &TreeOid))
        goto CommitNothing;

    if (git_signature_now(&Signature, Name, Email) != 0)
        goto CommitFail;

    if (HeadCommit)
        git_commit_tree(&ParentTree, HeadCommit);

    if (git_diff_tree_to_tree(&Diff, Repository, ParentTree, Tree, nullptr) == 0 &&
        git_diff_get_stats(&Stats, Diff) == 0)
    {
        size_t Files = git_diff_stats_files_changed(Stats);
        size_t Insertions = git_diff_stats_insertions(Stats);
        size_t Deletions = git_diff_stats_deletions(Stats);

        Git::Logger::Log(Git::Logger::Info("{yellow}%zu{white} file%s changed, {yellow}%zu{white} insertion%s(+), "
                                           "{yellow}%zu{white} deletion%s(-)"),
                         Files, Files == 1 ? "" : "s", Insertions, Insertions == 1 ? "" : "s", Deletions,
                         Deletions == 1 ? "" : "s");
    }

    git_diff_stats_free(Stats);
    git_diff_free(Diff);
    git_tree_free(ParentTree);

    if (git_commit_create_v(&CommitOid, Repository, "HEAD", Signature, Signature, nullptr, Message.c_str(), Tree,
                            HeadCommit ? 1 : 0, HeadCommit) != 0)
        goto CommitFail;

    git_index_free(Index);
    git_tree_free(Tree);
    git_tree_free(HeadTree);
    git_signature_free(Signature);
    git_commit_free(HeadCommit);

    return GitCodes::COMMIT_SUCCESS;

CommitNothing:
    git_index_free(Index);
    git_tree_free(Tree);
    git_tree_free(HeadTree);
    git_commit_free(HeadCommit);

    return GitCodes::NOTHING_TO_COMMIT;

CommitFail:
    git_index_free(Index);
    git_tree_free(Tree);
    git_tree_free(HeadTree);
    git_signature_free(Signature);
    git_commit_free(HeadCommit);

    return GitCodes::COMMIT_FAILED;
}
GitCodes GitRepository::Push()
{
    if (!Repository)
        return GitCodes::PUSH_FAILED;

    git_remote *Remote = nullptr;
    git_oid OldLocalOid, OldRemoteOid, NewLocalOid;
    git_push_options PushOptions = GIT_PUSH_OPTIONS_INIT;

    if (!Token.empty())
    {
        const char *TokenPtr = Token.c_str();
        PushOptions.callbacks.credentials = Git::Functions::CredentialToken;
        PushOptions.callbacks.payload = (void *)TokenPtr;
    }

    PushOptions.callbacks.certificate_check = Git::Functions::CertificateCheck;

    if (git_remote_lookup(&Remote, Repository, "origin") != 0)
        return GitCodes::ORIGIN_LOOKUP_FAILED;

    std::string Branch = GetBranch();
    std::string LocalRef = "refs/heads/" + Branch;
    std::string RemoteRef = "refs/remotes/origin/" + Branch;

    bool HaveLocal = git_reference_name_to_id(&OldLocalOid, Repository, LocalRef.c_str()) == 0;
    bool HaveRemote = git_reference_name_to_id(&OldRemoteOid, Repository, RemoteRef.c_str()) == 0;

    if (HaveLocal && HaveRemote && git_oid_equal(&OldLocalOid, &OldRemoteOid))
    {
        git_remote_free(Remote);
        return GitCodes::NOTHING_TO_PUSH;
    }

    std::string RefSpec = LocalRef + ":" + LocalRef;
    const char *RefList[] = {RefSpec.c_str()};
    const git_strarray RefSpecs = {(char **)RefList, 1};
    int Error = git_remote_push(Remote, &RefSpecs, &PushOptions);

    if (Error != 0)
    {
        git_remote_free(Remote);
        return GitCodes::PUSH_FAILED;
    }

    if (git_reference_name_to_id(&NewLocalOid, Repository, LocalRef.c_str()) != 0)
    {
        git_remote_free(Remote);
        return GitCodes::PUSH_FAILED;
    }

    char OldOid[8] = "0000000";
    char NewOid[8] = "0000000";

    if (HaveRemote)
        git_oid_tostr(OldOid, sizeof(OldOid), &OldRemoteOid);

    git_oid_tostr(NewOid, sizeof(NewOid), &NewLocalOid);

    const char *RemoteUrl = git_remote_url(Remote);

    Git::Logger::Log(Git::Logger::Info("To {cyan}%s"), RemoteUrl);

    Git::Logger::Log(Git::Logger::Info("   {cyan}%s{white}..{cyan}%s{white} {yellow}%s{white} -> {yellow}%s{white}"),
                     OldOid, NewOid, Branch.c_str(), Branch.c_str());

    git_remote_free(Remote);
    return GitCodes::PUSH_SUCCESS;
}
