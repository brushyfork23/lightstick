###
# program_lights_osx.bash
# 
# Program the light array from an OSX terminal
# 
# Usage:
#    ./program_lights_osx.bash
#
# Requires:
#    pyserial (`pip install pyserial`)
# 
# Questions: Danne Stayskal <danne@stayskal.com>
###

### Find the HEX file
export HEX_FILE=`ls /var/folders/6*/*/T/arduino_build_*/LightStick_Control.ino.hex`
if [ -e "$HEX_FILE" ]
then
  echo "Found HEX file at $HEX_FILE"
else
	echo "Cannot find HEX file. Please compile LightStick_Control in the Arduino IDE and re=run this script. Exiting."
	exit 1
fi

### Find the COM port
export COM_PORT=`ls /dev/cu.usbserial*`
if [ -e "$COM_PORT" ]
then
	echo "Found COM port at $COM_PORT"
else
	echo "Cannot find COM port. Exiting."
	exit 1
fi

### Clean up after ourselves
if [ -f LightStick_Control.hex ]
then
  rm LightStick_Control.hex
fi
if [ -f program_lights_output.log ]
then
	rm program_lights_output.log
fi
if [ -f program_lights_error.log ]
then
	rm program_lights_error.log
fi

### Push the build
cp $HEX_FILE LightStick_Control.hex
for BOARD_NUMBER in 27
do
  echo "Pushing build to Moteuino $BOARD_NUMBER"
  python WirelessProgramming.py -f LightStick_Control.hex -s $COM_PORT -t $BOARD_NUMBER 2> program_lights_error.log > program_lights_output.log
done

if [ `grep -e . -c program_lights_error.log` -gt 0 ]
then
	echo "Build errors:"
	tail -10 program_lights_error.log
	echo ""
	echo "Full error log in program_lights_error.log"
else
	echo "Build pushed with no errors"
fi

echo "Full programmer output is contained in program_lights_output.log"
