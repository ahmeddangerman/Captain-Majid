"""Check SDL capture with a locally supplied private content pack."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("executable", type=Path)
    parser.add_argument("pack", type=Path)
    parser.add_argument("private_temp", type=Path)
    args = parser.parse_args()
    env = os.environ.copy()
    if os.name == "nt":
        # Catch unbundled runtime dependencies hidden by a developer PATH.
        env["PATH"] = str(Path(env["SYSTEMROOT"]) / "System32")
    with tempfile.TemporaryDirectory(dir=args.private_temp) as directory:
        capture = Path(directory) / "scene.bmp"
        command = [str(args.executable.resolve()), str(args.pack.resolve()),
                   "--shoot", "--capture", str(capture)]
        result = subprocess.run(command, capture_output=True, text=True, env=env)
        assert result.returncode == 0, (result.returncode, result.stdout, result.stderr)
        data = capture.read_bytes()
        assert data[:2] == b"BM" and len(data) > 640 * 480 * 3
        again = subprocess.run(command, capture_output=True, text=True, env=env)
        assert again.returncode != 0
        assert capture.read_bytes() == data
        invalid = Path(directory) / "invalid.cmjpack"
        invalid.write_bytes(b"not a shot content pack")
        missing_output = Path(directory) / "invalid.bmp"
        rejected = subprocess.run([str(args.executable.resolve()), str(invalid),
                                   "--capture", str(missing_output)],
                                  capture_output=True, text=True, env=env)
        assert rejected.returncode != 0 and not missing_output.exists()
        assert "Shot scene stopped:" in rejected.stderr
    print("Shot scene capture, no-overwrite and invalid-pack checks passed")


if __name__ == "__main__":
    main()
