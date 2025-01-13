#!/bin/sh
MAGICKCWWWPATH="www/Magick++"
APIPATH="www/api/Magick++"

cd ..
> diff.log
#cd config && doxygen Magick++.dox &> /dev/null

# drawable.html has classes... (sub-classes, constructors,...)
# so these are all files with function signatures that we can compare with the doxygen generated files
DOC_HTML=("Blob.html" "CoderInfo.html" "Geometry.html" "Image++.html" "Montage.html" "Pixels.html")
DOXY_HTML=("Blob.html" "CoderInfo.html" "Geometry.html" "Image.html" "Montage.html" "Pixels.html")


for i in "${!DOC_HTML[@]}"; do
  #python3 scripts/compare_signatures.py $MAGICKCWWWPATH/${DOC_HTML[$i]} $APIPATH/${DOXY_HTML[$i]} >> diff.log 2>&1
  python3 scripts/compare_signatures.py $MAGICKCWWWPATH/${DOC_HTML[$i]} $APIPATH/classMagick_1_1${DOXY_HTML[$i]} >> diff.log
done

#python3 scripts/compare_signatures.py $MAGICKCWWWPATH/"Blob.html" $APIPATH/"classMagick_1_1Blob.html" >> diff.log
#python3 scripts/compare_signatures.py $MAGICKCWWWPATH/"CoderInfo.html" $APIPATH/"CoderInfo_8h.html" >> diff.log
#python3 scripts/compare_signatures.py $MAGICKCWWWPATH/"Geometry.html" $APIPATH/"Geometry_8h.html" >> diff.log
#python3 scripts/compare_signatures.py $MAGICKCWWWPATH/"Image++.html" $APIPATH/"Image_8h.html" >> diff.log
#python3 scripts/compare_signatures.py $MAGICKCWWWPATH/"Montage.html" $APIPATH/"Montage_8h.html" >> diff.log
#python3 scripts/compare_signatures.py $MAGICKCWWWPATH/"Pixels.html" $APIPATH/"Pixels_8h.html" >> diff.log
##python3 scripts/compare_signatures.py $MAGICKCWWWPATH/"STL.html" $APIPATH/"STL_8h.html" >> diff.log
##python3 scripts/compare_signatures.py $MAGICKCWWWPATH/"TypeMetric.html" $APIPATH/"TypeMetric_8h.html" >> diff.log


if [ -s diff.log ]; then
  # cat diff.log
  echo "Documentation is outdated. Please update!" >&2
  exit 1
else
  echo "Documentation is up to date."
fi