#!/bin/sh
# output the names of the files that cause jlint to crash

DIR=`dirname $0`
RETURN="0"

# find all errfiles and assign to $OUTS
OUTS=`find $DIR/log -name '*.errfiles'`

for FILE in $OUTS
    do 
	if [ -s $FILE ]
	# if file is not of zero size
	then
	    RETURN="1"
	    ERR=`echo $FILE | sed 's,.errfiles$,.err,'`
	    TEST=`basename $FILE | sed -e 's,.errfiles,,'`
	    echo "for filenames of files that caused an error in test$TEST look in: $FILE"  >>ERRFILE
	    echo "for error types look in : $ERR" >>ERRFILE
	    echo "##############################################################" >> ERRFILE
        fi
    done  #for

exit $RETURN
