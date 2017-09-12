git_rev ()
{
    d=`date +%Y%m%d`
    c=`git rev-list --full-history --all --abbrev-commit | wc -l | sed -e 's/^ *//'`
    h=`git rev-list --full-history --all --abbrev-commit | head -1`
    echo ${c}:${h}:${d}
}
git_rev
