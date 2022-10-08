#!/bin/bash
find ./code -regextype posix-egrep -regex "\./code/(src|tests|include)/.*\.(h|hpp|c|cc|cpp)?" \
    | xargs clang-format-14 -i --verbose