#!/bin/bash
MAGICKCWWWPATH="www/Magick++"
APIPATH="www/api/Magick++"

cd ..
# cd config && doxygen Magick++.dox &> /dev/null

# these are all files with function signatures that we can compare with the doxygen generated files currently
DOC_HTML=("Blob.html" "CoderInfo.html" "Geometry.html" "Image++.html" "Montage.html" "Pixels.html")
DOXY_HTML=("Blob.html" "CoderInfo.html" "Geometry.html" "Image.html" "Montage.html" "Pixels.html")


for i in "${!DOC_HTML[@]}"; do
  python3 scripts/compare_signatures.py "${MAGICKCWWWPATH}"/"${DOC_HTML[$i]}" "${APIPATH}"/classMagick_1_1"${DOXY_HTML[$i]}"
  echo ""
done
