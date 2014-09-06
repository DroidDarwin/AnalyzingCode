#!/bin/bash 

#### Description: 
###		Removes the top line of all files
### 	Only for temporary purposes

directory=javaOutput

FILES=$(find $directory -type f -name '*.java')
	for f in $FILES
	do

#echo $f	
	sed -i 1d $f



		
		
		
#		string=$f

#		if [[ $string != *'$'*  ]]
#		then
#			temp=$(basename $f)
#			echo "Converted " $(basename $f) " to " ${temp//.class/".java"}
		
			### A faster decompiler should be used
#			java -jar $apk_Conv_dir/jd-cmd/jd-cli/target/jd-cli.jar $f > ${f//.class/".java"}
#				#java -jar $apk_Conv_dir/cfr_0_78.jar $f > ${f//.class/".java"}
			
			### Remove the top line from each file which is just dummy output
#			sed -i 1d $JavaOutputDir/${temp//.class/".java"}
	
#		fi
	done
