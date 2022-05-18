# 
# Copyright 2022 blastdoor7
# 

set(HUNTER_CONFIGURATION_TYPES Release CACHE STRING "Build type of Hunter packages")

include(HunterGate)

HunterGate(
    URL https://github.com/cpp-pm/hunter/archive/v0.23.239.tar.gz
    SHA1 135567a8493ab3499187bce1f2a8df9b449febf3
    LOCAL
)
