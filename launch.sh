#!/bin/sh
PROGRAM_NAME="gatewayTest"
HOSTNAME="192.168.0.10"
#HOSTNAME="cm4"
if make
then
#echo "Setting up run environment..."
#ssh root@$HOSTNAME "killall $PROGRAM_NAME ; rm $PROGRAM_NAME"
echo "Copying program..."
scp $PROGRAM_NAME root@$HOSTNAME:~/$PROGRAM_NAME
scp nodes.txt root@$HOSTNAME:~/nodes.txt
echo "Program output:"
ssh -tt root@$HOSTNAME "killall $PROGRAM_NAME ; ./$PROGRAM_NAME"
fi
