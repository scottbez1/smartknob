#!/usr/bin/env python3

#   Copyright 2015-2021 Scott Bezek and the splitflap contributors
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

import argparse
import logging
import os
import psutil
import sys
import time

electronics_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
repo_root = os.path.dirname(electronics_root)
sys.path.append(repo_root)

from util import file_util
from export_util import (
    patch_config,
    PopenContext,
    versioned_file,
    xdotool,
    wait_for_window,
    recorded_xvfb,
)

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

RENDER_TIMEOUT = 10 * 60


def _wait_for_pcbnew_idle():
    start = time.time()
    while time.time() < start + RENDER_TIMEOUT:
        for proc in psutil.process_iter():
            if proc.name() == 'pcbnew':
                cpu = proc.cpu_percent(interval=1)
                print(f'CPU={cpu}', flush=True)
                if cpu < 5:
                    print('Render took %d seconds' % (time.time() - start))
                    return
        time.sleep(1)
    raise RuntimeError('Timeout waiting for pcbnew to go idle')


def _zoom_in():
    xdotool([
        'click',
        '4',
    ])
    time.sleep(0.2)


def _invoke_view_option(index):
    command = ['key', 'alt+v'] + ['Down']*index + ['Return']
    xdotool(command)
    time.sleep(2)


_transforms = {
    'z+': ('Zoom in', _zoom_in),
    'rx+': ('Rotate X Clockwise', lambda: _invoke_view_option(4)),
    'rx-': ('Rotate X Counterclockwise', lambda: _invoke_view_option(5)),
    'ry+': ('Rotate Y Clockwise', lambda: _invoke_view_option(6)),
    'ry-': ('Rotate Y Counterclockwise', lambda: _invoke_view_option(7)),
    'rz+': ('Rotate Z Clockwise', lambda: _invoke_view_option(8)),
    'rz-': ('Rotate Z Counterclockwise', lambda: _invoke_view_option(9)),
    'ml': ('Move left', lambda: _invoke_view_option(10)),
    'mr': ('Move right', lambda: _invoke_view_option(11)),
    'mu': ('Move up', lambda: _invoke_view_option(12)),
    'md': ('Move down', lambda: _invoke_view_option(13)),
}


def _pcbnew_export_3d(output_file, width, height, transforms):
    if os.path.exists(output_file):
        os.remove(output_file)

    wait_for_window('pcbnew', 'Pcbnew ', additional_commands=['windowfocus'])

    time.sleep(1)

    logger.info('Open 3d viewer')
    xdotool(['key', 'alt+3'])

    wait_for_window('3D Viewer', '3D Viewer', additional_commands=['windowfocus'])

    time.sleep(3)

    # Maximize window
    xdotool(['search', '--name', '3D Viewer', 'windowmove', '0', '0'])
    xdotool(['search', '--name', '3D Viewer', 'windowsize', str(width), str(height)])

    time.sleep(3)

    for transform in transforms:
        description, func = _transforms[transform]
        logger.info(description)
        func()

    logger.info('Wait for rendering...')

    _wait_for_pcbnew_idle()

    time.sleep(5)

    logger.info('Export current view')
    xdotool([
        'key',
        'alt+f',
        'Return',
    ])

    logger.info('Enter build output filename')
    xdotool([
        'key',
        'ctrl+a',
    ])
    xdotool(['type', output_file])

    logger.info('Save')
    xdotool(['key', 'Return'])

    logger.info('Wait before shutdown')
    time.sleep(2)


def export_3d(filename, suffix, width, height, transforms, raytrace, virtual, color_soldermask, color_silk, color_board, color_copper, release_prefix):
    pcb_file = os.path.abspath(filename)
    output_dir = os.path.join(electronics_root, 'build')
    file_util.mkdir_p(output_dir)

    screencast_output_file = os.path.join(output_dir, 'export_3d_screencast.ogv')

    name, _ = os.path.splitext(os.path.basename(pcb_file))
    if suffix:
        name = name + '-' + suffix
    output_file = os.path.join(output_dir, f'{name}-3d.png')

    settings = {
        'canvas_type': '1',
        'RenderEngine': '1' if raytrace else '0',
        'ShowFootprints_Virtual': '1' if virtual else '0',
        'Render_RAY_Backfloor': '0',
        'Render_RAY_ProceduralTextures': '0',
    }
    def apply_color(name, values):
        components = ['Red', 'Green', 'Blue']
        for component, value in zip(components, values):
            settings[name + '_' + component] = str(value)

    apply_color('SMaskColor', color_soldermask)
    apply_color('SilkColor', color_silk)
    apply_color('BoardBodyColor', color_board)
    apply_color('CopperColor', color_copper)

    with patch_config(os.path.expanduser('~/.config/kicad/pcbnew'), settings):
        with versioned_file(pcb_file, release_prefix):
            with recorded_xvfb(screencast_output_file, width=width, height=height, colordepth=24):
                with PopenContext(['pcbnew', pcb_file], close_fds=True) as pcbnew_proc:
                    _pcbnew_export_3d(output_file, width, height, transforms)
                    pcbnew_proc.terminate()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('pcb')
    parser.add_argument('--suffix', default='')
    parser.add_argument('--width', type=int, default=2560)
    parser.add_argument('--height', type=int, default=1440)
    parser.add_argument('--skip-raytrace', action='store_true')
    parser.add_argument('--skip-virtual', action='store_true', help='Don\'t render virtual footprints')
    parser.add_argument('--color-soldermask', type=float, nargs=3, help='Soldermask color as 3 floats from 0-1', default=[0, 0, 0])
    parser.add_argument('--color-silk', type=float, nargs=3, help='Silkscreen color as 3 floats from 0-1', default=[1, 1, 1])
    parser.add_argument('--color-board', type=float, nargs=3, help='PCB substrate color as 3 floats from 0-1', default=[0.764705882, 0.729411765, 0.607843137])
    parser.add_argument('--color-copper', type=float, nargs=3, help='Copper color as 3 floats from 0-1', default=[0.7, 0.7, 0.7])
    parser.add_argument('--release-prefix', type=str, required=True, help='Tag prefix to check if this is a tagged/versioned release. E.g. "releases/" for tags like "releases/v1.0"')

    # Use subparsers to for an optional nargs="*" choices argument (workaround for https://bugs.python.org/issue9625)
    subparsers = parser.add_subparsers(dest='which')
    transform_parser = subparsers.add_parser('transform', help='Apply one or more transforms before capturing image')
    transform_parser.add_argument('transform', nargs='+', choices=list(_transforms.keys()))

    args = parser.parse_args()

    transforms = args.transform if args.which == 'transform' else []

    export_3d(args.pcb, args.suffix, args.width, args.height, transforms, not args.skip_raytrace, not args.skip_virtual, args.color_soldermask, args.color_silk, args.color_board, args.color_copper, args.release_prefix)
