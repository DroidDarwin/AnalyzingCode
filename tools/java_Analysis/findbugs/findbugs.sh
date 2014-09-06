#!/bin/bash 
### Find Clones


### Todo
# Put all of this into a log

echo "Find Bugs: " 

### Location of the findbugs functionality
bugDetectorLocation=findbugs/findbugs-2.0.3

### Location of the class files to be analyzed
#inputLocation=../../testinputclass/
inputLocation=tools/java_Analysis/javaOutput

# Basic command to implement findbugs
# java -jar lib/findbugs.jar -textui $inputLocation/location1/

#echo $total_defects
#exit

#touch test.txt
#ls $inputLocation
### Loop through all of the main files in the input location
### Findbugs will be executed on each of them
for i in $(find $inputLocation -mindepth 1 -maxdepth 1 -type d ) # mindepth ignore the top layer
do
	echo $i
 	### Find way to output this to the appropriate storage location
	# temp=$(java -jar lib/findbugs.jar -textui -output test.txt -xml -sourcepath -min $i)
	#temp="`java -jar lib/findbugs.jar -textui -output test.txt -xml -sourcepath -min $i 2>&1`"
	#echo start $temp end
#	echo dan $temp

	echo `date`; java -jar tools/java_Analysis/findbugs/findbugs-2.0.3/lib/findbugs.jar -textui -output test.txt -xml -sourcepath -min $i/; echo `date`


done

