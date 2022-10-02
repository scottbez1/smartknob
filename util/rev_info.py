#!/usr/bin/env python3

import argparse
import datetime
import subprocess

def git_short_rev():
    try:
        return subprocess.check_output([
            'git',
            'rev-parse',
            '--short',
            'HEAD',
        ]).decode('utf-8').strip()
    except Exception:
        raise RuntimeError("Could not read git revision. Make sure you have git installed and you're working with a git clone of the repository.")

def current_date():
    return datetime.date.today().strftime('%Y-%m-%d')

def git_date(short=True):
    try:
        iso = subprocess.check_output([
            'git',
            'log',
            '-1',
            '--format=%ci',
            'HEAD',
        ]).decode('utf-8').strip()
        if short:
            return iso.split(' ')[0]
        else:
            return iso
    except Exception:
        raise RuntimeError("Could not read git commit date. Make sure you have git installed and you're working with a git clone of the repository.")

def git_release_version(search_prefix, fallback=None):
    try:
        tags = subprocess.check_output([
            'git',
            'tag',
            '--points-at',
            'HEAD',
        ]).decode('utf-8').splitlines()
        for tag in tags:
            if tag.startswith(search_prefix):
                return tag[len(search_prefix):]
        return fallback
    except Exception:
        raise RuntimeError("Could not read git release tags. Make sure you have git installed and you're working with a git clone of the repository.")

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(required=True, dest='option')
    
    parser_git_short_rev = subparsers.add_parser('git_short_rev')
    
    parser_git_date = subparsers.add_parser('git_date')
    parser_git_date.add_argument('--short', action='store_true')

    parser_git_release_version = subparsers.add_parser('git_release_version')
    parser_git_release_version.add_argument('search_prefix')

    args = parser.parse_args()
    if args.option == 'git_short_rev':
        print(git_short_rev())
    elif args.option == 'git_date':
        print(git_date(short=args.short))
    elif args.option == 'git_release_version':
        print(git_release_version(args.search_prefix, fallback='v#.#'))
    else:
        raise RuntimeError('Unexpected option')
