# The script will ask for the password once
sudo modprobe vcan
sudo ip link add vcan0 type vcan
sudo ip link set up vcan0