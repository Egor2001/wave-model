#!/usr/bin/python3

# usage:
# ./... DOMAIN_RANK TILE_RANK

import sys

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print('Usage: ', sys.argv[0], ' DOMAIN_RANK TILE_RANK')
        exit()
