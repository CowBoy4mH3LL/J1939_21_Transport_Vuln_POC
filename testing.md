# Regular test

Before beginning to test the vulnerabilities, one can run test scripts to 
# Vuln 1

## Test regular
1. Run the binary with Valgrind in one terminal as ``` build/main vcan 0 ``` ; replace "vcan" with CAN or whatever. The second argument is the index of the interface.
2. Run test script as ``` ./test_recv_conn.sh <src as in conf.h> <sleep between every sent message>```
3. Quit with ctrl+c
4. Binary will return data

## Test exploit

### Ulimit crash test
1. Run the binary with Valgrind in one terminal as ``` build/main vcan 0 ``` ; replace "vcan" with CAN or whatever. The second argument is the index of the interface.
2. In another terminal run ``` prlimit -d100 -p <pid> ``` ; This will restrict the process's data size segment's size to 100 bytes. One can do a prlimit at 256 kbytes to replicate an embedded processor. The processes PID can be found by running ``` ps aux| grep "build/main"```
3. Now run ``` ./exploit_vuln1.sh <src as in conf.h> <sleep between every sent message> ``` ; I typically use a sleep of 0.1 to eat heap up before they are unallocated; 
4. Program will crash after a while
   
### Valgrind check
1. Run the binary as ``` valgrind --tool=massif  build/main vcan 0 ```; Replace VCAN and 0 as before
2. Now run ``` ./exploit_vuln1.sh <src as in conf.h> <sleep between every sent message> ``` ; I typically use a sleep of 0.1 to eat heap up before they are unallocated; 
3. Kill the program with cntrl+c
4. massif output is produced in the same directory
5. read is using ``` ms_print massif.out<some number>```. It will show a steady increase in heap usage

# Vuln 2

## Test regular

1. Run the binary with Valgrind in one terminal as ``` valgrind --leak-check=yes  build/main vcan 0 ``` ; replace "vcan" with CAN or whatever. The second argument is the index of the interface.
2. Run test script as ``` ./test_send_conn.sh <src as in conf.h> <sleep between every sent message>```
3. Quit with ctrl+c
4. Valgrind will keep quite; Report no leaks.

## Test exploit
1. Run the binary with Valgrind in one terminal as ``` valgrind --leak-check=yes  build/main vcan 0 ``` ; replace "vcan" with CAN or whatever. The second argument is the index of the interface.
2. Run test script as ``` ./exploit_vuln2.sh <src as in conf.h> <sleep between every sent message>```
3. Watch Valgrind shout on the first terminal
4. Quit with ctrl+c if required.
   
# Vuln 3