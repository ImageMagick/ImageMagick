#!/bin/bash

set +x # Do not leak information

# Exit immediately if one of the files given as arguments is not there
# because we don't want to delete the existing release if we don't have
# the new files that should be uploaded 
for file in "$@"
do
    if [ ! -e "$file" ]
    then echo "$file is missing, giving up." >&2; exit 1
    fi
done

if [ $# -eq 0 ]; then
    echo "No artifacts to use for release, giving up."
    exit 0
fi

if command -v sha256sum >/dev/null 2>&1 ; then
  shatool="sha256sum"
elif command -v shasum >/dev/null 2>&1 ; then
  shatool="shasum -a 256" # macOS fallback
else
  echo "Neither sha256sum nor shasum is available, cannot check hashes"
fi

RELEASE_BODY=""
GIT_REPO_SLUG="$REPO_SLUG"

GIT_COMMIT="$TRAVIS_COMMIT"
GIT_TAG="$TRAVIS_TAG"

if [ ! -z "$TRAVIS_REPO_SLUG" ] ; then
  GIT_REPO_SLUG="$TRAVIS_REPO_SLUG"
  RELEASE_BODY="Travis CI build log: ${TRAVIS_BUILD_WEB_URL}"
elif [ ! -z "$GITHUB_ACTIONS" ] ; then
  GIT_COMMIT="$GITHUB_SHA"
  GIT_REPO_SLUG="$GITHUB_REPOSITORY"
  if [[ "$GITHUB_REF" == "refs/tags/"* ]] ; then
    GIT_TAG="${GITHUB_REF#refs/tags/}"
  fi
  RELEASE_BODY="GitHub Actions build log: $GITHUB_SERVER_URL/$GITHUB_REPOSITORY/actions/runs/$GITHUB_RUN_ID"
fi

if [ ! -z "$UPLOADTOOL_BODY" ] ; then
  RELEASE_BODY="$UPLOADTOOL_BODY"
fi

# The calling script (usually .travis.yml) can set a suffix to be used for
# the tag and release name. This way it is possible to have a release for
# the output of the CI/CD pipeline (marked as 'continuous') and also test
# builds for other branches.
# If this build was triggered by a tag, call the result a Release
if [ ! -z "$UPLOADTOOL_SUFFIX" ] ; then
  if [ "$UPLOADTOOL_SUFFIX" = "$GIT_TAG" ] ; then
    RELEASE_NAME="$GIT_TAG"
    RELEASE_TITLE="Release build ($GIT_TAG)"
    is_prerelease="false"
  else
    RELEASE_NAME="continuous-$UPLOADTOOL_SUFFIX"
    RELEASE_TITLE="Continuous build ($UPLOADTOOL_SUFFIX)"
    if [ -z "$UPLOADTOOL_ISPRERELEASE" ] ; then
      is_prerelease="false"
    else
      is_prerelease="true"
    fi

  fi
else
  # ,, is a bash-ism to convert variable to lower case
  case $(tr '[:upper:]' '[:lower:]' <<< "$GIT_TAG") in
    "")
      # Do not use "latest" as it is reserved by GitHub
      RELEASE_NAME="continuous"
      RELEASE_TITLE="Continuous build"
      if [ -z "$UPLOADTOOL_ISPRERELEASE" ] ; then
        is_prerelease="false"
      else
        is_prerelease="true"
      fi
      ;;
    *-alpha*|*-beta*|*-rc*)
      RELEASE_NAME="$GIT_TAG"
      RELEASE_TITLE="Pre-release build ($GIT_TAG)"
      is_prerelease="true"
      ;;
    *)
      RELEASE_NAME="$GIT_TAG"
      RELEASE_TITLE="Release build ($GIT_TAG)"
      is_prerelease="false"
      ;;
  esac
fi

if [ "$ARTIFACTORY_BASE_URL" != "" ]; then
  echo "ARTIFACTORY_BASE_URL set, trying to upload to artifactory"

  if [ "$ARTIFACTORY_API_KEY" == "" ]; then
    echo "Please set ARTIFACTORY_API_KEY"
    exit 1
  fi

  files="$@"

  # artifactory doesn't support any kind of "artifact description", so we're uploading a text file containing the
  # relevant details along with the other artifacts
  tempdir=$(mktemp -d)
  info_file="$tempdir"/build-info.txt
  echo "Travis CI build log: ${TRAVIS_BUILD_WEB_URL}" > "$info_file"
  files+=("$info_file")

  set +x

  for file in ${files[@]}; do
    url="${ARTIFACTORY_BASE_URL}/travis-${TRAVIS_BUILD_NUMBER}/"$(basename "$file")
    md5sum=$(md5sum "$file" | cut -d' ' -f1)
    sha1sum=$(sha1sum "$file" | cut -d' ' -f1)
    sha256sum=$(sha256sum "$file" | cut -d' ' -f1)
    echo "Uploading $file to $url"
    hashsums=(-H "X-Checksum-Md5:$md5sum")
    hashsums+=(-H "X-Checksum-Sha1:$sha1sum")
    hashsums+=(-H "X-Checksum-Sha256:$sha256sum")
    if ! curl -H 'X-JFrog-Art-Api:'"$ARTIFACTORY_API_KEY" "${hashsums[@]}" -T "$file" "$url"; then
      echo "Failed to upload file, exiting"
      rm -r "$tempdir"
      exit 1
    fi
    echo
    echo "MD5 checksum: $md5sum"
    echo "SHA1 checksum: $sha1sum"
    echo "SHA256 checksum: $sha256sum"
  done
  rm -r "$tempdir"
fi

# Do not upload non-master branch builds
# if [ "$GIT_TAG" != "$TRAVIS_BRANCH" ] && [ "$TRAVIS_BRANCH" != "master" ]; then export TRAVIS_EVENT_TYPE=pull_request; fi
if [ "$TRAVIS_EVENT_TYPE" == "pull_request" ] ; then
  echo "Release uploading disabled for pull requests"
  if [ "$ARTIFACTORY_BASE_URL" != "" ]; then
    echo "Releases have already been uploaded to Artifactory, exiting"
    exit 0
  else
    echo "Release uploading disabled for pull requests, uploading to transfersh.com instead"
    rm -f ./uploaded-to
    for FILE in "$@" ; do
      BASENAME="$(basename "${FILE}")"
      curl --upload-file $FILE "https://transfersh.com/$BASENAME" > ./one-upload
      echo "$(cat ./one-upload)" # this way we get a newline
      echo -n "$(cat ./one-upload)\\n" >> ./uploaded-to # this way we get a \n but no newline
    done
  fi
#  review_url="https://api.github.com/repos/${GIT_REPO_SLUG}/pulls/${TRAVIS_PULL_REQUEST}/reviews"
#  if [ -z $UPLOADTOOL_PR_BODY ] ; then
#    body="Travis CI has created build artifacts for this PR here:"
#  else
#    body="$UPLOADTOOL_PR_BODY"
#  fi
#  body="$body\n$(cat ./uploaded-to)\nThe link(s) will expire 14 days from now."
#  review_comment=$(curl -X POST \
#    --header "Authorization: token ${GITHUB_TOKEN}" \
#    --data '{"commit_id": "'"$GIT_COMMIT"'","body": "'"$body"'","event": "COMMENT"}' \
#    $review_url)
#  if echo $review_comment | grep -q "Bad credentials" 2>/dev/null ; then
#    echo '"Bad credentials" response for --data {"commit_id": "'"$GIT_COMMIT"'","body": "'"$body"'","event": "COMMENT"}'
#  fi
  $shatool "$@"
  exit 0
fi

if [ ! -z "$TRAVIS_REPO_SLUG" ] ; then
  echo "Running on Travis CI"
  echo "TRAVIS_COMMIT: $TRAVIS_COMMIT"
  if [ -z "$GITHUB_TOKEN" ] ; then
    echo "\$GITHUB_TOKEN missing, please set it in the Travis CI settings of this project"
    echo "You can get one from https://github.com/settings/tokens"
    exit 1
  fi
elif [ ! -z "$GITHUB_ACTIONS" ] ; then
  echo "Running on GitHub Actions"
  if [ -z "$GITHUB_TOKEN" ] ; then
    echo "\$GITHUB_TOKEN missing, please add the following to your run action:"
    echo "env:"
    echo "  GITHUB_TOKEN: \${{ secrets.GITHUB_TOKEN }}"
    exit 1
  fi
else
  echo "Not running on known CI"
  if [ -z "$GIT_REPO_SLUG" ] ; then
    read -r -p "Repo Slug (GitHub and Travis CI username/reponame): " GIT_REPO_SLUG
  fi
  if [ -z "$GITHUB_TOKEN" ] ; then
    read -r -s -p "Token (https://github.com/settings/tokens): " GITHUB_TOKEN
  fi
fi

tag_url="https://api.github.com/repos/$GIT_REPO_SLUG/git/refs/tags/$RELEASE_NAME"
tag_infos=$(curl -XGET --header "Authorization: token ${GITHUB_TOKEN}" "${tag_url}")
echo "tag_infos: $tag_infos"
tag_sha=$(echo "$tag_infos" | grep '"sha":' | head -n 1 | cut -d '"' -f 4 | cut -d '{' -f 1)
echo "tag_sha: $tag_sha"

release_url="https://api.github.com/repos/$GIT_REPO_SLUG/releases/tags/$RELEASE_NAME"
echo "Getting the release ID..."
echo "release_url: $release_url"
release_infos=$(curl -XGET --header "Authorization: token ${GITHUB_TOKEN}" "${release_url}")
echo "release_infos: $release_infos"
release_id=$(echo "$release_infos" | grep "\"id\":" | head -n 1 | tr -s " " | cut -f 3 -d" " | cut -f 1 -d ",")
echo "release ID: $release_id"
upload_url=$(echo "$release_infos" | grep '"upload_url":' | head -n 1 | cut -d '"' -f 4 | cut -d '{' -f 1)
echo "upload_url: $upload_url"
release_url=$(echo "$release_infos" | grep '"url":' | head -n 1 | cut -d '"' -f 4 | cut -d '{' -f 1)
echo "release_url: $release_url"
target_commit_sha=$(echo "$release_infos" | grep '"target_commitish":' | head -n 1 | cut -d '"' -f 4 | cut -d '{' -f 1)
echo "target_commit_sha: $target_commit_sha"

if [ "$GIT_COMMIT" != "$target_commit_sha" ] ; then

  echo "GIT_COMMIT != target_commit_sha, hence deleting $RELEASE_NAME..."
  
  if [ ! -z "$release_id" ]; then
    delete_url="https://api.github.com/repos/$GIT_REPO_SLUG/releases/$release_id"
    echo "Delete the release..."
    echo "delete_url: $delete_url"
    curl -XDELETE \
        --header "Authorization: token ${GITHUB_TOKEN}" \
        "${delete_url}"
  fi

  # echo "Checking if release with the same name is still there..."
  # echo "release_url: $release_url"
  # curl -XGET --header "Authorization: token ${GITHUB_TOKEN}" \
  #     "$release_url"

  if [ "$RELEASE_NAME" == "continuous" ] ; then
    # if this is a continuous build tag, then delete the old tag
    # in preparation for the new release
    echo "Delete the tag..."
    delete_url="https://api.github.com/repos/$GIT_REPO_SLUG/git/refs/tags/$RELEASE_NAME"
    echo "delete_url: $delete_url"
    curl -XDELETE \
        --header "Authorization: token ${GITHUB_TOKEN}" \
        "${delete_url}"
  fi

  echo "Create release..."

  release_infos=$(curl -H "Authorization: token ${GITHUB_TOKEN}" \
       --data '{"tag_name": "'"$RELEASE_NAME"'","target_commitish": "'"$GIT_COMMIT"'","name": "'"$RELEASE_TITLE"'","body": "'"$RELEASE_BODY"'","draft": false,"prerelease": '$is_prerelease'}' "https://api.github.com/repos/$GIT_REPO_SLUG/releases")

  echo "$release_infos"

  unset upload_url
  upload_url=$(echo "$release_infos" | grep '"upload_url":' | head -n 1 | cut -d '"' -f 4 | cut -d '{' -f 1)
  echo "upload_url: $upload_url"

  unset release_url
  release_url=$(echo "$release_infos" | grep '"url":' | head -n 1 | cut -d '"' -f 4 | cut -d '{' -f 1)
  echo "release_url: $release_url"

fi # if [ "$GIT_COMMIT" != "$tag_sha" ]

if [ -z "$release_url" ] ; then
	echo "Cannot figure out the release URL for $RELEASE_NAME"
	exit 1
fi

echo "Upload binaries to the release..."

# Need to URL encode the basename, so we have this function to do so
urlencode() {
  # urlencode <string>
  old_lc_collate=$LC_COLLATE
  LC_COLLATE=C
  local length="${#1}"
  for (( i = 0; i < length; i++ )); do
    local c="${1:$i:1}"
    case $c in
      [a-zA-Z0-9.~_-]) printf '%s' "$c" ;;
      *) printf '%%%02X' "'$c" ;;
    esac
  done
  LC_COLLATE=$old_lc_collate
}

for FILE in "$@" ; do
  FULLNAME="${FILE}"
  BASENAME="$(basename "${FILE}")"
  curl -H "Authorization: token ${GITHUB_TOKEN}" \
       -H "Accept: application/vnd.github.manifold-preview" \
       -H "Content-Type: application/octet-stream" \
       --data-binary "@$FULLNAME" \
       "$upload_url?name=$(urlencode "$BASENAME")"
  echo ""
done

$shatool "$@"

if [ "$GIT_COMMIT" != "$tag_sha" ] ; then
  echo "Publish the release..."

  release_infos=$(curl -H "Authorization: token ${GITHUB_TOKEN}" \
       --data '{"draft": false}' "$release_url")

  echo "$release_infos"
fi # if [ "$GIT_COMMIT" != "$tag_sha" ]
