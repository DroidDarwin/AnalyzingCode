#! /bin/sh

NROFTESTS=4
DIR=`dirname $0`
ERRFILE=$DIR/log/test.err
DIFFFILE=$DIR/log/test.diff

export DIR
export ERRFILE
export DIFFFILE

case "$1" in

    "--help" )

	echo "testall.sh: Usage: ./testall {option}"
	echo "Options: "
	echo "   --help : print this text"
	echo "   --valgrind  run valgrind on testsuite"
        ;;

    "" )

	echo "running runtest.sh"
	`$DIR/runtest.sh $NROFTESTS`
	[ "$?" != "0" ] && echo "runtest.sh exited abnormally" && exit -1

	echo "running showdiff.sh"
	`$DIR/showdiff.sh`
	case "$?" in
	    "0" )
		echo "     Outputs do not differ"
		;;
	    "1" )
		echo "     diff found:"
		cat $DIFFFILE
		;;
	    * )
		echo "     exited abnormally"
		exit -1
		;;
	esac
    
	echo "running showerror.sh"
	`$DIR/showerror.sh`
	case "$?" in
	    "0" )
		echo "     no error found"
		;;
	    "1" )
		echo "     error found:"
		cat $ERRFILE
		;;
	    * )
		echo "     exited abnormally"
		exit -1
		;;
	esac
	;;

    "--valgrind" )

	echo "running valgrind runtest.sh"
	`valgrind -v $DIR/runtest.sh $NROFTESTS`
	[ "$?" != "0" ] && echo "valgrind runtest.sh exited abnormally" && 
								    exit -1
		    
	echo "running showdiff.sh"
	`$DIR/showdiff.sh`
	case "$?" in
	    "0" )
		echo "     Outputs do not differ"
		;;
	    "1" )
		echo "     diff found:"
		cat $DIFFFILE
		;;
	    * )
		echo "     exited abnormally"
		exit -1
		;;
	esac
		    
	echo "running showerror.sh"
	`$DIR/showerror.sh`
	case "$?" in
	    "0" )
		echo "     no error found"
		;;
	    "1" )
		echo "     error found:"
		cat $ERRFILE
		;;
	    * )
		echo "     exited abnormally"
		exit -1
		;;
	esac
	;;
      
    * )
	echo "testall.sh: Usage: ./testall {option}"
	echo "Try 'testall.sh --help' for more information."
	;;
esac

exit 0
