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