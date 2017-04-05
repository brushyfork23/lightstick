HEX_FILE=`ls /tmp/build*/LightStick_Control.ino.hex`
echo "found $HEX_FILE"
rm LightStick_Control.hex
cp $HEX_FILE LightStick_Control.hex

python WirelessProgramming.py -f LightStick_Control.hex -s /dev/ttyUSB0 -t 20
python WirelessProgramming.py -f LightStick_Control.hex -s /dev/ttyUSB0 -t 21
python WirelessProgramming.py -f LightStick_Control.hex -s /dev/ttyUSB0 -t 22
