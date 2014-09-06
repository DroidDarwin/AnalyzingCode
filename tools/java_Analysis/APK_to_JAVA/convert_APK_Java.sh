#!/bin/bash 

##Usage
## ./convert_APK_Java

## Description: Converts all apk files in a given directory to .java x
### Make sure that open-jdk is installed 
### 	sudo apt-get install openjdk-7-jdk

## Location of directory holding all java conversion functionality
apk_Conv_dir=tools/java_Analysis/APK_to_JAVA

###Input location with all of the .apk files to be analyzed
APKInputDir=$1 

logLocation=logs/convert_apk.log

### Remove the log if it is there
rm -f $logLocation
## Create the log
touch $logLocation


date1=$(date +"%s") # Start Run Time

### convert individual file
### Refactor this out to a different file
convertAPK (){
	
	echo Working DIR: `pwd`
	echo "Begin Analyzing:" `date` " " $1 >> $logLocation
	inputFileName=$(basename $1 .apk)

	### Application Name without .apk
	appName=${1//.apk/""}

	## Replace all the periods in the apk to allow the better creation of the output foldername
	## Replace . with %
	## 		This should probably be fixed to just strip out the apk entirely.
	##		May need to be refactored 
	outputFolderName=${1//[.]/%}

	### Main directory all of the generated java output
	javaOutputLocation=tools/java_Analysis/javaOutput
	mkdir -p $javaOutputLocation

	JavaOutputDir=$javaOutputLocation/$outputFolderName
	dirAndAppName=$JavaOutputDir/$1

	mkdir -p $JavaOutputDir

	cp $APKInputDir/$1 $JavaOutputDir

	## Start analyzing the apk files
	java -jar $apk_Conv_dir/apktool1.5.2/apktool.jar d -f $dirAndAppName
	
	### Not sure why it creates an output file here, but delete it
	### This is a messy fix
	rm -rf $inputFileName

	## Create the dex file
	jar xvf $JavaOutputDir/$1 classes.dex
	mv classes.dex $JavaOutputDir/

	./$apk_Conv_dir/dex2jar-0.0.9.15/dex2jar.sh $JavaOutputDir/classes.dex 

	## Switching locations was the only way to have everything output in the appropriate location.
	cd $JavaOutputDir
	jar xvf classes_dex2jar.jar 
	cd ../../ 

	### An ugly hack
	cd ../../

	### Get number of classes to be analyzed
	### This will provide a rough estimate of the classes to be analyzed
	classCompareCount=`find $JavaOutputDir -type f -name '*.class' | wc -l`
	echo "Classes to convert:" $classCompareCount  `date` >> $logLocation

	count=0 ### Clear the counter

	## Now convert all of the .class files to .java
	java -jar $apk_Conv_dir/jd-cmd/jd-cli/target/jd-cli.jar $JavaOutputDir -od $JavaOutputDir

		
 	## Log the results
 	classFileCount=`find $JavaOutputDir -type f -name '*.class' | wc -l`
 	javaFileCount=`find $JavaOutputDir -type f -name '*.java' | wc -l`

	echo "	*****Output Dir: " `echo $JavaOutputDir` "Classfiles " `echo $classFileCount` >> $logLocation
	echo "	" `echo $appName` " Class Files Created: " $classFileCount

	echo "	*****Output Dir: " `echo $JavaOutputDir` "Javafiles " `echo $javaFileCount` >> $logLocation
	echo "	" `echo $appName` " Java Files Created: " $javaFileCount


	##### INSERT THE NUMBER OF CLASS FILES CREATED INTO THE SQLITE DATABASE
	#####  # Of class files: $classFileCount
	#####  File name: $1
	#cd ../../  #moving to the directory with the database


	apkID=${1//.apk/""} ### Remove the apk exension from the apkID
	rowid=`sqlite3 EvolutionOfAndroidApplications.sqlite  "SELECT rowid FROM ApkInformation WHERE ApkId='$apkID';"`
	
	### If the rowID is not found, then insert the value into the database. This should never happen, but this is a failsafe.
	permRowCount=0
	permRowCount=`sqlite3 EvolutionOfAndroidApplications.sqlite  "SELECT count(*) FROM toolresults WHERE apkid='$rowid';"`

	if [ $permRowCount -eq 0 ]
	then
		sqlite3 EvolutionOfAndroidApplications.sqlite  "INSERT INTO toolresults (ApkId) VALUES ('$rowid');"
	fi

	sqlite3 EvolutionOfAndroidApplications.sqlite  "UPDATE ToolResults SET ClassFiles=$classFileCount WHERE ApkId=$rowid;"	

	##### INSERT THE NUMBER OF Java FILES CREATED INTO THE SQLITE DATABASE
	#####  # Of Java files: $javaFileCount
	#####  File name: $JavaOutputDir
	sqlite3 EvolutionOfAndroidApplications.sqlite  "UPDATE ToolResults SET JavaFiles=$javaFileCount WHERE ApkId=$rowid;"	
	#cd tools/java_Analysis #going back to where you were before



	### Get the LOC (Lines of Code) in the application and add that to the DB
	loc=0 ### 
	loc=`(find $JavaOutputDir/* -name '*.java' -print0 | xargs -0 cat ) | wc -l`
	echo "Total LOC: $loc" `date` >> $logLocation
	echo LOC: $loc

	sqlite3 EvolutionOfAndroidApplications.sqlite  "UPDATE ToolResults SET loc=$loc WHERE ApkId=$rowid;"


	## Output the results to the user
	#	echo "	*****Output Dir: " `echo $JavaOutputDir` >> $logLocation
	#	echo "	" `echo $appName` " Files Created: " `find $JavaOutputDir -type f -name '*.java' | wc -l` 

}

## Remove spaces in the filenames. This will cause probems for the rest of the application.
find $APKInputDir -name '* *' | while read file;
do
	target=`echo "$file" | sed 's/ /_/g'`;
	echo "Renaming '$file' to '$target'";
	mv "$file" "$target";
done;

echo "Start Date:" `date` >> $logLocation

## Loop through all the contents of the main APK directory
FILES=$(find $APKInputDir -type f -name '*.apk')
for f in $FILES
do
	#echo $f
	convertAPK $(basename $f)
	#echo $(basename $f)
done

	#cd ../../

### Log end date/time
date2=$(date +"%s")
diff=$(($date2-$date1))

echo "convert_APK End:" `date` >> $logLocation
echo "Total Time $(($diff / 60)) minutes and $(($diff % 60)) seconds."  >> $logLocation





### Todo: 
# Find a faster decompilation method

