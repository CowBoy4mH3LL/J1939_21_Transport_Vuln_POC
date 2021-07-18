if [[ $# -ne 2 ]] ; then
    echo 'usage <script> <src> <sleep>; Check script for more details.'
    exit 1
fi

# Sends RTS, recieve CTS, sends 7 data packets
# Result -- valgrind should not shout
cangen vcan0 -e -g $1 -I 18EA$100 -D DEADBE

