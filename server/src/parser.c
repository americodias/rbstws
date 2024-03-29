/*   This file is prepared for Doxygen automatic documentation generation     */
/*! \file **********************************************************************
 *
 * \brief
 *      Command parser
 * 
 * \author
 *      Américo Dias <americo.dias@fe.up.pt>
 *
 * $Revision$
 * $HeadURL$
 * $Date$
 * $Author$
 * $Id$
 *
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parser.h"
#include "defs.h"
#include "interpreter.h"


int send_ack(int *socket) {
    return write(*socket, CMD_ACKNOWLEDGE, strlen(CMD_ACKNOWLEDGE));
}

int send_nack(int *socket) {
	return write(*socket, CMD_NOT_ACKNOWLEDGE, strlen(CMD_NOT_ACKNOWLEDGE));
}

int command_test(char *string, char *command) {
    if(strlen(string) < strlen(command))
        return 0;

    return (strncmp(string, command, strlen(command)) == 0) ? 1 : 0;
}

int command_parser(char *string, int *socket) 
{
    static int program_status = 0;
    static int breakpoint_status = 0;
    
	FILE *fp;
	int n;
	
	/*
	 * Command exit:
	 * Close the connection!
	 */
    if(command_test(string, CMD_EXIT)) {
		if((n=send_ack(socket)) < 0) return n;
		sleep(1);
        return 1;
    }
    /*
     * Command program:
     * Receive the hexadecimal file from the TCP/IP connection and sent it
     * to the FILE_NAME in the system temporary directory.
     * If the file already exist, it is rewritten.
     */
	else if(command_test(string, CMD_PROGRAM)) {
		fp = fopen(PROG_FILE_NAME, "wt");
		
		if(fp == NULL)
			return -1;
		
		fclose(fp);
		
		program_status = 1;
		
		if((n=send_ack(socket)) < 0) return n;
	}
    /*
     * Command end program:
     * Receive the hexadecimal file from the TCP/IP connection and sent it
     * to the FILE_NAME in the system temporary directory.
     * If the file already exist, it is rewritten.
     */
	else if(command_test(string, CMD_END_PROGRAM)) {				
		program_status = 0;
		code_parser();
		if((n=send_ack(socket)) < 0) return n;
	}
	/*
	 * Command run program:
	 */
    else if(command_test(string, CMD_RUN_PROGRAM)) {
        interpreter_start_run();
        if((n=send_ack(socket)) < 0) return n;
    }
	/*
	 * Command abort program:
	 */
    else if(command_test(string, CMD_ABORT_PROGRAM)) {
        interpreter_abort_run();
        if((n=send_ack(socket)) < 0) return n;
    }
	/*
	 * Command step program:
	 */
    else if(command_test(string, CMD_STEP_PROGRAM)) {
        interpreter_step();
        if((n=send_ack(socket)) < 0) return n;
    }
	/*
	 * Command breakpoint start:
	 */
    else if(command_test(string, CMD_BREAKPOINT_START)) {
        interpreter_clear_breakpoints();
        breakpoint_status = 1;
        if((n=send_ack(socket)) < 0) return n;
    }
	/*
	 * Command breakpoint end:
	 */
    else if(command_test(string, CMD_BREAKPOINT_END)) {
        breakpoint_status = 0;
        if((n=send_ack(socket)) < 0) return n;
    }
	/*
	 * Command ping:
	 * Just for testing... answer with a "PONG".
	 */
    else if(command_test(string, CMD_PING)) {
        n = write(*socket, CMD_PONG, strlen(CMD_PONG));
        if (n < 0)
			return n;
    }
    /*
     * Program mode:
     * If the program mode is active, send the data to the program file
     */ 
    else if(program_status == 1) {
		
		fp = fopen(PROG_FILE_NAME, "at");
		
		if(fp == NULL)
			return -1;
			
		n = fwrite(string, sizeof(char), strlen(string), fp);
		fputc('\n', fp);
		
		fflush(fp);
		fclose(fp);
		
		if(n < 0)
			return n;
			
		if((n=send_ack(socket)) < 0) return n;
			
    }
    /*
     * Breakpoint mode:
     */ 
    else if(breakpoint_status == 1) {
        interpreter_add_breakpoint(atoi(string));
        if((n=send_ack(socket)) < 0) return n;
    }
    /*
     * Else:
     * Command or data not understood. Send a NOT ACKOWLEDGE.
     */ 
	else {
		if((n=send_nack(socket)) < 0) return n;
	}

	return 0;
}

