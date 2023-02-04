import os
import sys
if __name__ == '__main__':
    if 'PIPENV_ACTIVE' not in os.environ:
        sys.exit(f'This script should be run in a Pipenv.\n\nRun it as:\npipenv run python {os.path.basename(__file__)}')

from pathlib import Path
import shutil
import subprocess

def run():
    SCRIPT_PATH = Path(__file__).absolute().parent
    REPO_ROOT = SCRIPT_PATH.parent

    proto_path = REPO_ROOT / 'proto'

    nanopb_path = REPO_ROOT / 'thirdparty' / 'nanopb'

    # Make sure nanopb submodule is available
    if not os.path.isdir(nanopb_path):
        print(f'Nanopb checkout not found! Make sure you have inited/updated the submodule located at {nanopb_path}', file=sys.stderr)
        exit(1)

    nanopb_generator_path = nanopb_path / 'generator' / 'nanopb_generator.py'
    c_generated_output_path = REPO_ROOT / 'firmware' / 'src' / 'proto_gen'

    proto_files = [f for f in os.listdir(proto_path) if f.endswith('.proto')]
    assert len(proto_files) > 0, 'No proto files found!'

    # Generate C files via nanopb
    subprocess.check_call(['python3', nanopb_generator_path, '-D', c_generated_output_path] + proto_files, cwd=proto_path)


    # Use nanopb's packaged protoc to generate python bindings
    protoc_path = nanopb_path / 'generator' / 'protoc'
    python_generated_output_path = REPO_ROOT / 'software' / 'python' / 'proto_gen'
    python_generated_output_path.mkdir(parents=True, exist_ok=True)
    subprocess.check_call([protoc_path, '--version'])
    subprocess.check_call([
        protoc_path,
        '--python_out',
        python_generated_output_path,
    ] + proto_files, cwd=proto_path)

    # Copy nanopb's compiled options proto
    shutil.copy2(nanopb_path / 'generator' / 'proto' / 'nanopb_pb2.py', python_generated_output_path)


if __name__ == '__main__':
    run()
