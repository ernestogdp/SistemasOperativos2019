#!/bin/sh

listfiles() {
	files=$(find $1)

	for i in $files; do
		if file $i | egrep 'ASCII text' > /dev/null; then
			bytes=$(ls -l $i | awk '{print $5}')
			if [ $bytes -ge $2 ]; then
				echo $bytes"   "$i
			fi
		fi
	done
}

if [ $# -eq 1 ]; then
	directory=$1
	limitbytes=0

elif  [ $# -eq 2 ]; then
	limitbytes=$1
	directory=$2

else
	echo "usage: findbig.sh byteslen directory or findbig.sh directory" 1>&2
	exit 1
fi

listfiles $directory $limitbytes | sort -rh

exit 0
