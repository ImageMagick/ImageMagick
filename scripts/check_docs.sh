#!/bin/bash

cd ..
cd config && doxygen Magick++.dox &> /dev/null

# documentation files with function signatures that we can compare with their corresponding doxygen file (currently)
DOC_HTML=("Blob.html" "CoderInfo.html" "Geometry.html" "Image++.html" "Montage.html" "Pixels.html")
DOXY_HTML=("Blob.html" "CoderInfo.html" "Geometry.html" "Image.html" "Montage.html" "Pixels.html")  # Image.html is the only one that differs from the doxygen file


for i in "${!DOC_HTML[@]}"; do
  python3 scripts/compare_signatures.py www/Magick++/"${DOC_HTML[$i]}" www/api/Magick++/classMagick_1_1"${DOXY_HTML[$i]}"
  echo ""
done
