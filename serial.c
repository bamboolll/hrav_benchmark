/****************************************************************
 *	Ho Chi Minh city University of Technology
 *	Faculty of Computer Science and Engineering
 *	
 *	@filename:		serial.h
 *	@date:			Dec 11
 *
 *
 *
 ***************************************************************
 * */

#include "serial.h"

/****************************************************************
 *	@name:		serial_Open
 *	@param:		serialPortName: serial port file's path
 *	@retval:	SUCCESS:	0
 *			FAIL:		otherwise
 ***************************************************************
 * */

int serial_Open(char *serialPortName) {
	// open serial port
	fd = open(serialPortName, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		printf("Can not open serial port at %s\n", serialPortName);
		return -1;
	}
	
	// get the current settings of the serial port
	tcgetattr(fd, &tio);

	// set baud rate to 9600
	cfsetispeed(&tio, B9600);
	cfsetospeed(&tio, B9600);

	// set no parity
	tio.c_cflag &= ~PARENB;
	
	// only one stop bit
	tio.c_cflag &= ~CSTOPB;
	
	// clear out the current word size
	tio.c_cflag &= ~CSIZE;

	// Eight-bits per word
	tio.c_cflag |= CS8;

	// set timeout
	tio.c_cc[VMIN] = 0;
	tio.c_cc[VTIME] = 1;

	// don't allow the port control to be changed
	// and enable the receiver
	tio.c_cflag |= (CLOCAL | CREAD);

	// apply the settings to the serial port
	if (tcsetattr(fd, TCSANOW, &tio) != 0) {
		printf("can not apply settings to serial port at %s\n", serialPortName);
		return -1;
	}
	return 0;

}




/************************************************************************
 *	@name:          serial_Open
 *      @param:         serialPortName: serial port file's path
 *      @retval:        SUCCESS:        0
 *                      FAIL:           otherwise
 ************************************************************************
 **/
int readData(char *result) {
	int resultLength = read(fd, result, MAX_BUFSIZE);
	if (resultLength < 0) {
		return -1;
	}else{
		result[resultLength] = '\0';
		return resultLength;
	}
}

int writeData(char *sendData) {
	return write(fd, sendData, strlen(sendData));
}

