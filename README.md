# gmsv_git

A Garry's Mod module that integrates Git into your server.

# Authorization

Placing a file named `git.token` with a PAT (Personal Access Token) will allow Git to access private repositories.

# API

```lua
    git.Clone("repository_url", "destination") -- Blank for /garrysmod
```

```lua
    git.Pull("destination") -- Pulls any changes
```

```lua
    git.Checkout("destination", "branch/commit") -- Checkouts a branch or commit
```

```lua
    git.Add("destination", "file/./*") -- Adds a file for staging (. for everything outside of .gitignore, * for everything)
```

```lua
    git.Commit("destination", "message", "author name", "author email") -- Creates a commit for staged files.
```

```lua
    git.Push("destination") -- Pushes staged commits.
```

```lua
    git.GetBranch() -- Returns the current branch.
```

```lua
    git.GetShortHash() -- Returns the 7-character hash of the latest commit.
```