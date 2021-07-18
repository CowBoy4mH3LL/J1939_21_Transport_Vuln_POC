if [[ $# -ne 2 ]] ; then
    echo 'usage <script> <src> <sleep>; Check script for more details.'
    exit 1
fi

# Sends RTS, recieve CTS, sends 7 data packets
# Result -- valgrind should not shout
cansend vcan0 18EC$100#10002C07FFDEADBE
sleep 1
cansend vcan0 18EB$100#01DEADBEEFCAFEBA
sleep $2
cansend vcan0 18EB$100#02DEADBEEFCAFEBA
sleep $2
cansend vcan0 18EB$100#03DEADBEEFCAFEBA
sleep $2
cansend vcan0 18EB$100#04DEADBEEFCAFEBA
sleep $2
cansend vcan0 18EB$100#05DEADBEEFCAFEBA
sleep $2
cansend vcan0 18EB$100#06DEADBEEFCAFEBA
sleep $2
cansend vcan0 18EB$100#07DEADBEEFCAFEBA

