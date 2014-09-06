#!/bin/bash 


#temp=${temp//Found /""}
#temp=${temp//clone/""}
#temp=${temp//pairs/""}
#echo ResultStart $temp ResultEnd  #### This is a dirty hack


#temp="Verification completed: 0 reported messages. "
#set -- $temp
#stringchecker=$1

#if [[ $stringchecker =~ "Verification"* ]]
#then
#	echo yes
#else
#	echo no
#fi


output="Verification completed: 99 reported messages."
output2="javaOutput/Gmail%apk/com/google/common/collect/MapMaker$StrategyImpl.java:1: Method '<unknown>.notifyAll' is called without synchronizing on '<unknown>'. javaOutput/Gmail%apk/com/google/common/collect/MapMaker$StrategyImpl.java:2: Method '<unknown>.wait' is called without synchronizing on '<unknown>'. Verification completed: 2 reported messages."
output3="javaOutput/Gmail%apk/com/google/common/io/protocol/ProtoBuf.java:1: Data can be lost as a result of truncation to byte. javaOutput/Gmail%apk/com/google/common/io/protocol/ProtoBuf.java:2: Data can be lost as a result of truncation to byte. javaOutput/Gmail%apk/com/google/common/io/protocol/ProtoBuf.java:3: Data can be lost as a result of truncation to byte. javaOutput/Gmail%apk/com/google/common/io/protocol/ProtoBuf.java:4: Data can be lost as a result of truncation to byte. javaOutput/Gmail%apk/com/google/common/io/protocol/ProtoBuf.java:5: Data can be lost as a result of truncation to byte. javaOutput/Gmail%apk/com/google/common/io/protocol/ProtoBuf.java:6: Data can be lost as a result of truncation to byte. javaOutput/Gmail%apk/com/google/common/io/protocol/ProtoBuf.java:7: Maybe type cast is not correctly applied. javaOutput/Gmail%apk/com/google/common/io/protocol/ProtoBuf.java:8: Data can be lost as a result of truncation to int. Verification completed: 8 reported messages."



#sed -n "/START-WORD-HERE/,/END-WORD-HERE/p" $output


#sed "/START-WORD-HERE/,/END-WORD-HERE/p" "dan"

#echo "Test to find string between words FOO-/*+_and_+*/- BAR" | sed -e "s/.*FOO//;s/BAR.*//"

#echo $output | sed -e "s/.*Verification completed: //;s/ reported messages.*//"
#echo $output2 | sed -e "s/.*Verification completed: //;s/ reported messages.*//"
#echo $output3 | sed -e "s/.*Verification completed: //;s/ reported messages.*//"

verify() {
if [[ $1 =~ "Verification Completed"* ]]
		then
			echo yes
		else
			echo no
		fi



}



str1="dan is cool"
str2="asdfas Verification Completed"
str3="asdfas Verification Completed adfasfasd "
str4="Verification Completed adfasfasd "
str5="adfasfasdasdffadsf asd "

temp=`verify "$str1"`
echo 111 $temp

temp=`verify "$str2"`
echo 222 $temp

temp=`verify "$str3"`
echo 333 $temp

temp=`verify "$str4"`
echo 444 $temp

temp=`verify "$str5"`
echo 555 $temp


exit


#### Get the output count from the output generated from jlint
getOutputCount (){

	outputCount=0 ## Set the initial count to be zero in case no values are going to be returned
	
#	### This is all done to set up the validation of the first word in the String
#	input=$1
#	temp=$1
#	set -- $temp
#	stringchecker=$1



#sed -n "/<PRE>/,/<\/PRE>/p" input.html

sed -n "/START-WORD-HERE/,/END-WORD-HERE/p" $1


exit

	if [[ $stringchecker =~ "Verification"* ]]
		then
			### Strip out the un needed information from each message
			###		Would be cleaner to do in one line, but I am not sure how to do this.
			temp=$input
			temp=${temp//Verification /""}
			temp=${temp//completed:/""}
			temp=${temp//reported/""}
			temp=${temp//messages./""}
			outputCount=$temp
		fi
	echo $outputCount
}


### Function should return message count. 0 if no messages were recorded







### check to make sure it contains verification complete


output="Verification completed: 99 reported messages."
output2="java/lang/String.java:1: equals() was overridden but not hashCode()."
output3="javaOutput/Gmail%apk/com/google/common/io/protocol/ProtoBuf.java:1: Data can be lost as a result of truncation to byte. javaOutput/Gmail%apk/com/google/common/io/protocol/ProtoBuf.java:2: Data can be lost as a result of truncation to byte. javaOutput/Gmail%apk/com/google/common/io/protocol/ProtoBuf.java:3: Data can be lost as a result of truncation to byte. javaOutput/Gmail%apk/com/google/common/io/protocol/ProtoBuf.java:4: Data can be lost as a result of truncation to byte. javaOutput/Gmail%apk/com/google/common/io/protocol/ProtoBuf.java:5: Data can be lost as a result of truncation to byte. javaOutput/Gmail%apk/com/google/common/io/protocol/ProtoBuf.java:6: Data can be lost as a result of truncation to byte. javaOutput/Gmail%apk/com/google/common/io/protocol/ProtoBuf.java:7: Maybe type cast is not correctly applied. javaOutput/Gmail%apk/com/google/common/io/protocol/ProtoBuf.java:8: Data can be lost as a result of truncation to int. Verification completed: 8 reported messages."



temp=`getOutputCount "$output"`
#echo 111: $temp

temp=`getOutputCount "$output2"`
#echo 222: $temp

temp=`getOutputCount "$output3"`
echo 333: $temp