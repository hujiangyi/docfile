#!/bin/bash
cpfile () {
    echo "cp -rf \"md/book/$1.md\" \"../github_mydoc/$1.md\""
    if [ -f md/book/$1.md ]; then
        cp -rf "md/book/$1.md" "../github_mydoc/$1.md"
    fi
    if [ -d md/book/$1.files ]; then
        cp -rf "md/book/$1.files" "../github_mydoc/$1.files"
    fi
    if [ -f md/book/$1.mdg ]; then
        cp -rf "md/book/$1.mdg" "../github_mydoc/$1.mdg"
    fi
}
IFS=$'\n' 
OLDIFS="$IFS"
for f in `cat files`;  
do  
FILENAME=${f%.*}
RENAME=${FILENAME//\ /_}
SUBPATH=${RENAME%/*}
echo 'test.sh file:' $f
echo 'test.sh FILENAME ' $FILENAME
echo 'test.sh RENAME ' $RENAME
echo 'test.sh SUBPATH ' $SUBPATH
rm -rf ../github_doctomd/book/*
rm -rf ../github_doctomd/md/*
mkdir -p "../github_doctomd/book/$SUBPATH"
echo "cp -rf \"$f\" \"../github_doctomd/book/$f\""
cp -rf "$f" "../github_doctomd/book/$f"
cd ../github_doctomd/
./mult-doc-to-md.sh "../github_doctomd/book/$f"
rm -rf "../github_mydoc/$f"
cpfile $RENAME
cd ../github_docfile
done 
cd ../github_mydoc
IFS="$OLDIFS"
