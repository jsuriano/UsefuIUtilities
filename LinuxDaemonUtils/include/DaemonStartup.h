/*
 * DaemonStartup.h
 *
 *  Created on: Jul 14, 2010
 *      Author: jsuriano
 */

#ifndef DAEMONSTARTUP_H_
#define DAEMONSTARTUP_H_

/*! \brief This function is used to startup the Daemon process properly. It spawns a child process that is used to
 * run all the functionality of the daemon and then exits the original process. This leaves the daemon process running
 * without being tied to the original terminal and will not exit prematurely then.
 */
void Start_Daemon();

void Term_Signal_Handler( int sig );

#endif /* DAEMONSTARTUP_H_ */
