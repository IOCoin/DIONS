I/O Coin development tree for DIONS (Decentralized Input Output Name Server Blockchain Upgrade)
===========================

I/O Coin is a POS-Based Blockchain.

Features:
* Symbol: IOC
* 16 Million POW coins
* 22 Million total coins
* 60 second blocktime
* Dinamic interest 1-2%, based on the percentage of coins staking
* Rewards .01 IOC per transaction
* 2014 Fair Launch via X11 PoW/PoS
* 1250 I/O block reward, first 30 blocks half (625)
* 12815 PoW block limit
* P2P Port: 33764
* RPC Port: 33765
* Dedicated Node: 178.62.109.73
* Enhanced API allowing registration of aliases and associated data,
  aliases may be kept encrypted or public. Aliases, encrypted or public may
  send and receive.
* New API includes a messaging layer for encrypted communications in addition
  to plain text messaging. Encryption uses AES 256 encryption together with RSA
  encryption (4096).


Development process
===========================

Developers work in their own trees, then submit pull requests when
they think their feature or bug fix is ready.

The patch will be accepted if there is broad consensus that it is a
good thing.  Developers should expect to rework and resubmit patches
if they don't match the project's coding conventions (see coding.txt)
or are controversial.

The master branch is regularly built and tested, but is not guaranteed
to be completely stable. Tags are regularly created to indicate new
stable release versions of I/O Coin.

Feature branches are created when there are major new features being
worked on by several people.

From time to time a pull request will become outdated. If this occurs, and
the pull is no longer automatically mergeable; a comment on the pull will
be used to issue a warning of closure. The pull will be closed 15 days
after the warning if action is not taken by the author. Pull requests closed
in this manner will have their corresponding issue labeled 'stagnant'.

Issues with no commits will be given a similar warning, and closed after
15 days from their last activity. Issues closed in this manner will be
labeled 'stale'.


===
// Copyright (c) 2017 IODigital foundation 
