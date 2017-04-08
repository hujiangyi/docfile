#!/bin/bash
if [ -z $1 ]; then
    echo "file path is need."
    exit 1
fi

rm -rf ../github_doctomd/book/*
rm -rf ../github_doctomd/md/*
cp -rf $1/ ../github_doctomd/book/$1
cd ../github_doctomd/
./mult-doc-to-md.sh book/
rm -rf ../github_mydoc/$1
cp -rf md/book/$1/ ../github_mydoc/$1
cd ../github_mydoc
#gitbook build .
#rm -rf ../github_mydoc_pages/*
#cp -rf _book/ ../github_mydoc_pages/
#cd ../github_mydoc_pages/
#git commit -a -m "auto commit"
#git fetch
#git rebase
#git push
