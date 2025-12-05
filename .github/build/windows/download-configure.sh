#/bin/bash
set -e

download_release()
{
  local project=$1
  local release=$2
  local file=$3

  echo "Downloading $file from release $release of $project"

  curl -sS -L "https://github.com/ImageMagick/$project/releases/download/$release/$file" -o "$file"
}

download_configure()
{
  local version=$1

  mkdir -p "Configure"
  cd "Configure"

  download_release "Configure" "$version" "Configure.Release.x64.exe"
  download_release "Configure" "$version" "Configure.Release.arm64.exe"
  download_release "Configure" "$version" "Configure.Release.x86.exe"
  download_release "Configure" "$version" "files.zip"
  unzip -o "files.zip" || rm "files.zip"

  cd ..
}

download_configure "2025.12.05.1723"
