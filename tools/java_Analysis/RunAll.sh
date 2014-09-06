#!/bin/bash 

	### Description: 
	###		Converts all APK files to java class files
	###		Analyzes the .class and .java files


	### Check to make sure that an argument is actually passed in
	### This input paramter will be the input location of the apk files to be analyzed
	EXPECTED_ARGS=1
	E_BADARGS=65
	if [ $# -ne $EXPECTED_ARGS ]
	then
	  echo "Usage: `basename $0` {arg}"
	  echo "No Expected Input Parameter found"
	#  exit $E_BADARGS
		exit
	fi

	inputLocation=$1 ### The passed in input location of the .apk files which are to be analyzed

	#### Check to make sure that java is installed
	#### Probably change this to check for open jdk ?

	if type -p java; then
	    echo "Java Found"
	else
	    echo "Java not found"
		exit
	fi

	#### Input location for all apk files
	## This will need to be changed based on the final configuration
	#inputLocation=../testAndroidApps
	logDir=logs
	logFile=runAll.log

	## Make sure that the logs directory exists
	mkdir -p $logDir 

	### Delete the log file if it exists
	rm -f $logDir/*.log  ##This is now in the toolsScript.sh instead - Shannon

	touch $logDir/$logFile

	date1=$(date +"%s") # Start Run Time
	#### Add Starting message into Logs
	echo "RunAll Start:" `date` >> $logDir/$logFile


	### Check to make sure that there are files to analyze
	apkCount=`find $inputLocation -type f -name '*.apk' | wc -l`
	if [[ $apkCount<1 ]]
	then
			### Nothing to do
			echo "No APK files to analyze:" `date` >> $logDir/$logFile
			echo "No APK files to analyze"
	else

		### Perform java conversion of APK files to java
		echo "Java Conversion:" `date` >> $logDir/$logFile
#		./tools/java_Analysis/APK_to_JAVA/convert_APK_Java.sh $inputLocation

		### Find the clones in the system
		echo "Clones:" `date` >> $logDir/$logFile
			#	./tools/java_Analysis/CloneDetection/runclones.sh $inputLocation
#		./tools/java_Analysis/CloneDetection/SimCad-2.2/run_simcad.sh tools/java_Analysis/javaOutput


		#### CheckStyle
		echo "CheckStyle:" `date` >> $logDir/$logFile
		./tools/java_Analysis/checkstyle/checkStyle.sh

		### Run JLint
		echo "Start Running JLint:" `date` >> $logDir/$logFile
		./tools/java_Analysis/jlint/run_jlint.sh


		### Remove the created javaoutput.
		echo "Remove Java Output" `date` >> $logDir/$logFile
#		rm -rf tools/java_Analysis/javaOutput/*

	fi

	date2=$(date +"%s")
	diff=$(($date2-$date1))
	echo "RunAll End:" `date` >> $logDir/$logFile
	echo "Total Time $(($diff / 60)) minutes and $(($diff % 60)) seconds."  >> $logDir/$logFile




#### Todo: 


