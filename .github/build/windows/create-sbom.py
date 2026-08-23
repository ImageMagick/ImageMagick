import json
import os
import sys
import uuid

if len(sys.argv) != 2:
    print("Usage: create-sbom.py <output>")
    sys.exit(1)

output_file = sys.argv[1]
version = os.environ["VERSION"]
timestamp = os.environ["TIMESTAMP"]

with open("Artifacts/sbom.json", "r") as f:
    sbom = json.load(f)

sbom["serialNumber"] = "urn:uuid:" + str(uuid.uuid4())
sbom["metadata"]["timestamp"] = timestamp
sbom["metadata"]["component"]["version"] = version

with open(output_file, "w") as f:
    json.dump(sbom, f, indent=2)

print(f"SBOM created: {output_file}")
