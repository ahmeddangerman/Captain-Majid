"""Check SDL capture with a locally supplied private content pack."""
import argparse
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("executable", type=Path)
    parser.add_argument("pack", type=Path)
    parser.add_argument("private_temp", type=Path)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(dir=args.private_temp) as directory:
        capture = Path(directory) / "scene.bmp"
        command = [str(args.executable.resolve()), str(args.pack.resolve()),
                   "--shoot", "--capture", str(capture)]
        result = subprocess.run(command, capture_output=True, text=True)
        assert result.returncode == 0, (result.returncode, result.stdout, result.stderr)
        data = capture.read_bytes()
        assert data[:2] == b"BM" and len(data) > 640 * 480 * 3
        again = subprocess.run(command, capture_output=True, text=True)
        assert again.returncode != 0
        assert capture.read_bytes() == data
    print("Shot scene capture and no-overwrite checks passed")


if __name__ == "__main__":
    main()
