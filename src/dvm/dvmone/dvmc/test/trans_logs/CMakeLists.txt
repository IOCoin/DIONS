# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

set(PREFIX ${PROJECT_NAME}/trans_logs)

add_test(NAME ${PREFIX}/trans_log-static COMMAND dvmc-trans_log-static)
add_test(NAME ${PREFIX}/trans_log-dynamic-load COMMAND dvmc-trans_log $<TARGET_FILE:dvmc::trans_log-vm>)
