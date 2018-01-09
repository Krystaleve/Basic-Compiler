#!/bin/bash

if [ $# -eq 0 ]; then
    if GIT_SEQUENCE_EDITOR=$0 git rebase -p -i --root; then
      revert_command='git reset --keep $(git reflog | grep "rebase -i (start)" -A1 -m1 | sed -n "2s/\s.*//p")'
      echo "Revert with following command:"
      echo "  $revert_command"
    fi
elif [ $# -eq 1 ]; then
    sed -i "s/^pick \(\\w\+\) \(.*\(README\|SCRIPT\)\)/drop \\1 \\2/g w /dev/stderr" $1
else
    echo "Wrong number of arguments"
fi

