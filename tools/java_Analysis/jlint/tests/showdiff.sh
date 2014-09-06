#!/bin/sh
# write all the log differences into the file ./log/test.diff
# returns 0 if no difference, otherwise return 1

RETURN="0"
DIR=`dirname $0`

# find all logfiles and assign to $OUTS
OUTS=`find $DIR/log -name '*.out'`

for FILE in $OUTS
    do
	LOG=`echo $FILE | sed 's,\.out$,.log,'`

	# if corresponding log file exists and is regular file
        if [ -f $LOG ] 
	then
	    `cmp --silent $LOG $FILE`
	    if	[ "$?" = "1" ]
	    # the two files differ
	    then
		RETURN="1"
		echo "############################################################" >> $DIFFFILE
		echo "diff of $FILE $LOG is: " >> $DIFFFILE
		echo "############################################################" >> $DIFFFILE
		`diff $FILE $LOG >> $DIFFFILE`
            fi
        fi
done  #for

exit $RETURN
