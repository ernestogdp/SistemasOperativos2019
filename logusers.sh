#!/bin/sh

if [ $# -ne 1 ]
then
    echo "usage : logusers directory_name" 1>&2
    exit 1
fi

if ! test -d $1; then
	if ! test -f $1; then
		mkdir $1
	else
		echo "ERR: The dir name does already exist as a regular file" 1>&2
		exit 1
	fi
else
	echo "Err: The directory name does already exist as a directory" 1>&2
	exit 1
fi

cd $1
usuarios=$(who | awk '{print $1}' | uniq)

for usuario in $usuarios; do
	ps aux | awk '$1 ~ /'$usuario'/ {print $2}' | grep -v PID > $usuario.log
done

exit 0
