import os
import sys
import uuid

if len(sys.argv) != 2:
    print("Usage: create-sbom.py <output>")
    sys.exit(1)

output_file = sys.argv[1]
commit = os.environ["MAGICK_COMMIT"]
timestamp = os.environ["MAGICK_TIMESTAMP"]
version = os.environ["MAGICK_VERSION"]

with open("Artifacts/sbom.json", "r", encoding="utf-8", newline="") as f:
    sbom = f.read()

replacements = {
    "[MAGICK_SERIAL]": "urn:uuid:" + str(uuid.uuid4()),
    "[MAGICK_TIMESTAMP]": timestamp,
    "[MAGICK_VERSION]": version,
    "[MAGICK_COMMIT]": commit,
}
for marker, value in replacements.items():
    if marker not in sbom:
        sys.exit(f"Missing SBOM marker: {marker}")
    sbom = sbom.replace(marker, value)

with open(output_file, "w", encoding="utf-8", newline="") as f:
    f.write(sbom)

print(f"SBOM created: {output_file}")
