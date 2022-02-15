#!/usr/bin/env python

#   Copyright 2015-2016 Scott Bezek and the splitflap contributors
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

import logging
import os
import re
import subprocess
import sys
import tempfile
import time

from contextlib import contextmanager

electronics_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
repo_root = os.path.dirname(electronics_root)
sys.path.append(repo_root)

from xvfbwrapper import Xvfb
from util import file_util, rev_info

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

class PopenContext(subprocess.Popen):
    def __enter__(self):
        return self
    def __exit__(self, type, value, traceback):
        if self.stdout:
            self.stdout.close()
        if self.stderr:
            self.stderr.close()
        if self.stdin:
            self.stdin.close()
        if type:
            self.terminate()
        # Wait for the process to terminate, to avoid zombies.
        self.wait()

def xdotool(command):
    return subprocess.check_output(['xdotool'] + command)

def wait_for_window(name, window_regex, additional_commands=None, timeout=10):
    if additional_commands is not None:
        commands = additional_commands
    else:
        commands = []

    DELAY = 0.5
    logger.info('Waiting for %s window...', name)
    for i in range(int(timeout/DELAY)):
        try:
            xdotool(['search', '--name', window_regex] + commands)
            logger.info('Found %s window', name)
            return
        except subprocess.CalledProcessError:
            pass
        time.sleep(DELAY)
    raise RuntimeError('Timed out waiting for %s window' % name)

@contextmanager
def recorded_xvfb(video_filename, **xvfb_args):
    with Xvfb(**xvfb_args):
        with PopenContext([
                'recordmydesktop',
                '--no-sound',
                '--no-frame',
                '--on-the-fly-encoding',
                '-o', video_filename], close_fds=True) as screencast_proc: 
            yield
            screencast_proc.terminate()


def get_versioned_contents(filename):
    with open(filename, 'r') as f:
        original_contents = f.read()
        date = rev_info.git_date()
        date_long = rev_info.git_date(short=False)
        rev = rev_info.git_short_rev()
        logger.info('Replacing placeholders with %s and %s' % (date, rev))
        return original_contents, original_contents \
            .replace('Date ""', 'Date "%s"' % date_long) \
            .replace('DATE: YYYY-MM-DD TIME TZ', 'DATE: %s' % date_long) \
            .replace('DATE: YYYY-MM-DD', 'DATE: %s' % date) \
            .replace('Rev ""', 'Rev "%s"' % rev) \
            .replace('COMMIT: deadbeef', 'COMMIT: %s' % rev)


@contextmanager
def versioned_file(filename):
    original_contents, versioned_contents = get_versioned_contents(filename)
    with open(filename, 'w') as temp_schematic:
        logger.debug('Writing to %s', filename)
        temp_schematic.write(versioned_contents)
    try:
        yield
    finally:
        with open(filename, 'w') as temp_schematic:
            logger.debug('Restoring %s', filename)
            temp_schematic.write(original_contents)


@contextmanager
def patch_config(filename, replacements):
    if not os.path.exists(filename):
        yield
        return

    with open(filename, 'r') as f:
        original_contents = f.read()

    new_contents = original_contents
    for (key, value) in replacements.items():
        pattern = '^' + re.escape(key) + '=(.*)$'
        new_contents = re.sub(pattern, key + '=' + value, new_contents, flags=re.MULTILINE)

    with open(filename, 'w') as f:
        logger.debug('Writing to %s', filename)
        f.write(new_contents)
    try:
        yield
    finally:
        with open(filename, 'w') as f:
            logger.debug('Restoring %s', filename)
            f.write(original_contents)

