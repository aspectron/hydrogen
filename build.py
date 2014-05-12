#!/usr/bin/env python

import os
import sys

def main():
    jsx_root = os.environ.get('JSX_ROOT', os.path.join(os.path.pardir, 'jsx'))
    jsx_root = os.path.abspath(jsx_root)
    jsx_extern = os.path.join(jsx_root, 'extern')

    try:
        sys.path.insert(0, jsx_extern)
        from build_utils import parse_args, build_project
    except:
        sys.exit('Error: JSX root not found in {}'.format(jsx_root))

    args = parse_args()
    if not args.gyp_file:
        args.gyp_file = 'hydrogen.gyp'
    if not args.jsx_root:
        args.jsx_root = jsx_root

    build_project(args)


if __name__ == '__main__':
    main()

