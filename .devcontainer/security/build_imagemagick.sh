#!/bin/bash -eu

./Magick++/fuzz/build_imagemagick.sh

cat <<EOT >> /ImageMagick/etc/ImageMagick-7/policy.xml
<policymap>
  <policy domain="resource" name="memory" value="256MiB"/>
  <policy domain="resource" name="list-length" value="32"/>
  <policy domain="resource" name="width" value="8KP"/>
  <policy domain="resource" name="height" value="8KP"/>
  <policy domain="resource" name="map" value="512MiB"/>
  <policy domain="resource" name="area" value="16KP"/>
  <policy domain="resource" name="disk" value="1GiB"/>
  <policy domain="resource" name="file" value="768"/>
  <policy domain="resource" name="thread" value="2"/>
  <policy domain="resource" name="time" value="120"/>
</policymap>
EOT