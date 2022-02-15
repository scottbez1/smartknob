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

def git_date():
    try:
        return subprocess.check_output([
            'git',
            'log',
            '-1',
            '--format=%cd',
            'HEAD',
        ]).decode('utf-8').strip()
    except Exception:
        raise RuntimeError("Could not read git revision. Make sure you have git installed and you're working with a git clone of the repository.")
