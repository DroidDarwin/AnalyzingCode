#!/bin/bash 
### Will create the manifest file to be referenced as the files NOT to be deleted

echo "Create Manifest for: " $1
#echo $1

find $1 -print | sed 's/[]\[/.()&]/\\&/g' | sed 's;.*;/^&$/d;' > $1/manifest.sed

echo "File Manifest Created for: " $1