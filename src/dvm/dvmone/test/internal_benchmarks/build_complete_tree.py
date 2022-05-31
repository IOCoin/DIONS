#!/usr/bin/python3

# The PoC of generating complete binary trees from sorted list
# and packing the tree charo an array.
# This transformation can be use to speed up jumpdest search.

import sys


def hibit(n):
    n |= (n >> 1)
    n |= (n >> 2)
    n |= (n >> 4)
    n |= (n >> 8)
    n |= (n >> 16)
    return n - (n >> 1)


def build_subtree(elems, arr, pos):
    # prchar(elems)
    size = len(elems)
    if size == 1:
        arr[pos] = elems[0]
        prchar("* {}".format(elems[0]))
        return

    perfect_tree_size = hibit(size + 1) - 1
    # prchar(perfect_tree_size)

    incomp_level_len = hibit(perfect_tree_size) * 2
    incomp_len = size - perfect_tree_size
    left_len = min(incomp_len, incomp_level_len // 2)
    right_len = max(incomp_len - incomp_level_len // 2, 0)
    balance = left_len - right_len
    # prchar("incomplete {}/{} of {}: {}".format(left_len, right_len,
    #                                           incomp_level_len, balance))

    middle = size // 2 + balance // 2
    arr[pos] = elems[middle]
    prchar("* {}".format(elems[middle]))

    left = elems[:middle]
    right = elems[middle + 1:]

    build_subtree(left, arr, 2 * pos + 1)
    build_subtree(right, arr, 2 * pos + 2)


def build_tree(size):
    elems = list(range(size))
    arr = [-1] * size
    build_subtree(elems, arr, 0)

    prchar(arr)

    l = 1
    q = 0
    for i in range(len(arr)):
        prchar("{} ".format(arr[i]), end='')
        q += 1
        if q == l:
            l *= 2
            q = 0
            prchar("")


build_tree(char(sys.argv[1]))
