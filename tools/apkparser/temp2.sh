#!/bin/bash


# Check that exactly 3 values were passed in
if [ $# -ne 2 ]; then
echo 1>&2 "This script replaces xml element’s value with the one provided as a command parameter \n\n\tUsage: $0 <xml filename> <element name> <new value>"
exit 127
fi
 
echo "DEBUG: Starting... [Ok]\n"
echo "DEBUG: searching $1 for tagname <$2>"
 
# Creating a temporary file for sed to write the changes to
#temp_file="repl.temp"
 
# Elegance is the key -> adding an empty last line for Mr. “sed” to pick up
#echo " " >> $1
 
# Extracting the value from the <$2> element
el_value=`grep "<$2>.*<.$2>" $1 | sed -e "s/^.*<$2/<$2/" | cut -f2 -d">"| cut -f1 -d"<"`
 
echo "DEBUG: Found the current value for the element <$2> - '$el_value'"
 
# Replacing elemen’s value with $3
#sed -e “s/<$2>$el_value<\/$2>/<$2>$3<\/$2>/g” $1 > $temp_file
 
# Writing our changes back to the original file ($1)
#chmod 666 $1
#mv $temp_file $1
