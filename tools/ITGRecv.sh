#!/bin/sh

# Tiny script that re-lunches ITGRecv in case it dies...
cd /opt/ditg/bin
while [ 1 ]
do
	date=`date`

	echo "-------------------------------"
	echo "[$date] Starting ITGRecv"
	./ITGRecv -l /dev/null -Sp 3000

	# If you want to get an email alert when this happens, uncomment the following line:
	# echo "ITGRecv has crashed on: $date" | mail -s "ITGRecv has crashed" info@broadbandforall.net

	sleep 10
done
