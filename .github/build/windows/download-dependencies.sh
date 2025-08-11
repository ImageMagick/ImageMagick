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

download_dependencies()
{
  local version=$1
  local artifact=$2

  mkdir -p "Dependencies"
  cd "Dependencies"

  download_release "Dependencies" "$version" "$artifact"
  unzip -o "$artifact" -d "../Artifacts" || {
    exit_code=$?
    if [[ $exit_code -ne 0 && $exit_code -ne 1 ]]; then
      echo "Unzip failed with exit code $exit_code"
      exit $exit_code
    fi
  }

  cd ..
}

dependenciesArtifact=""

while [[ $# -gt 0 ]]; do
  case $1 in
    --dependencies-artifact)
      dependenciesArtifact="$2"
      shift 2
      ;;
    *)
      echo "Unknown option: $1"
      exit 1
      ;;
  esac
done

if [[ -z "$dependenciesArtifact" ]]; then
  echo "Error: The --dependencies-artifact option is required."
  exit 1
fi

download_dependencies "2025.08.11.0502" "$dependenciesArtifact"
