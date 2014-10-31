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
#ifndef _SERIAL_H_

#define _SERIAL_H_


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define MAX_BUFSIZE	1000

struct termios tio;
int fd;

/****************************************************************
 *	@name:		serial_Open
 *	@param:		serialPortName: serial port file's path
 *	@retval:	SUCCESS:	0
 *			FAIL:		otherwise
 ***************************************************************
 * */
int serial_Open(char *serialPortName);


/************************************************************************
 *	@name:          readData
 *      @param:         result: a char* string read from serial port
 *      @retval:        SUCCESS:        0
 *                      FAIL:           otherwise
 ************************************************************************
 **/
int readData(char *result);


/************************************************************************
 *	@name:		writeData
 *	@param:		sendData:	a char* string will be sent to
 *					serial port
 *	@retval:	SUCCESS:	0
 *			FAIL:		otherwise
 ************************************************************************
 * */
int writeData(char *sendData);


#endif




/*	END OF FILE	*/
