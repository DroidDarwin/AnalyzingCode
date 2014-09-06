#!/bin/bash 

###	Description:
###		Will be invoked from a conjob and will fire everything off

	logDir=logs
	logFile=StartAll.log

	## Make sure that the logs directory exists
	mkdir -p $logDir 

	### Delete the log file if it exists
	rm -f $logDir/*.log  

	touch $logDir/$logFile

	date1=$(date +"%s") # Start Run Time
	#### Add Starting message into Logs
	echo "StartAll:" `date` >> $logDir/$logFile

	## Start scraper here
	echo "Start Scraper:" `date` >> $logDir/$logFile
	# cd scraper/
	# scrapy crawl googleplay
	# cd ..
	# ./cleanDatabase.sh
	echo "Scraper Completed:" `date` >> $logDir/$logFile


	### Analyze the collected apk files
	echo "Start toolsScript:" `date` >> $logDir/$logFile
	./toolsScript.sh
	echo "End toolsScript:" `date` >> $logDir/$logFile


	#### Log the conclusion time
	date2=$(date +"%s")
	diff=$(($date2-$date1))
	echo "StartAll Total Running Time $(($diff / 60)) minutes and $(($diff % 60)) seconds."  >> $logDir/$logFile
	echo "StartAll End:" `date` >> $logDir/$logFile


	echo "Start Checking into GitHub:" `date` >> $logDir/$logFile
	./checkAllIntoGitHub.sh



