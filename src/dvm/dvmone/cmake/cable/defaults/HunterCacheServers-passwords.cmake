# Cable: CMake Bootstrap Library.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0. See the LICENSE file.

# Hunter passwords file used by HunterCacheServers.cmake.
# Do not include directly.

hunter_upload_password(
    # REPO_OWNER + REPO = https://github.com/blastdoor7/hunter-cache
    REPO_OWNER blastdoor7
    REPO hunter-cache

    # USERNAME = https://github.com/hunter-cache-bot
    USERNAME hunter-cache-bot

    # PASSWORD = GitHub token saved as a secure environment variable
    PASSWORD "$ENV{HUNTER_CACHE_TOKEN}"
)
