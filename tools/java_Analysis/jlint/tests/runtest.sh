#!/bin/sh
# run all the testcases

LOGDIR=$DIR/log

TEST=$1
I=1

# delete old test.diff file
    [ -f $LOGDIR/test.diff ] && rm $LOGDIR/test.diff
# delete all error and log files before running a new test!
for F in log err errfiles
do
    find $LOGDIR/ -name *.$F -exec rm {} \;
done

while [ "$I" -le $TEST ] 
do
    if [ -d $DIR/test$I ]
    then
	find $DIR/test$I/ -name '*.class' | \
	sed -e 's,\$,\\$,g' | sort -d | sed -e "s,\(.*\), ../jlint \1 >>$LOGDIR/$I.log 2>>$LOGDIR/$I.err," | sh 2>>$LOGDIR/$I.errfiles
	# sort -d so that jlint will analyze the class files always in the 
	# same order. 
	# output of jlint will be written in $I.log
	# errors occuring during runtime are written into $I.err
	# names of the class files which produced an error in jlint are
	# written into $I.errfiles
    fi
    I=`expr $I + 1`
done

exit 0
