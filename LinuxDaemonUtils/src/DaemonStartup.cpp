/*
 * DaemonStartup.cpp
 *
 *  Created on: Jul 14, 2010
 *      Author: jsuriano
 */
#include <sys/types.h>  // uses pid_t
#include <sys/stat.h>   // uses the stat function
#include <stdlib.h>     // calls exit and uses EXIT_FAILURE and EXIT_SUCCESS
#include <fcntl.h>      // calls umask
#include <unistd.h>     // uses usleep
#include <stdio.h>      // uses the standard FILE items
#include <csignal>            // catches signals sent in
#include <ptypes/pstreams.h>        //

#include "CommandLineStruct.h"
#include "M427_Logging.h"

extern CmdLineStruct gCmdlineItems;
extern pt::logfile* gLogFile;
extern pthread_mutex_t gLockWriting;
extern int gReopenLogFile;
//extern std::vector< FpgaCtrl* > gFpgaCtrl;

void Open_Log_File( void );

/*! \brief This function is used to handle incoming signals to the daemon. It will capture the SIGHUP and SIGTERM signals
 * to allow the system to shutdown gracefully by closing all connections and deleting any dynamic memory prior to exiting.
 *
 * \param sig The signal that is currently being received and handled by the function.
 */
void Term_Signal_Handler( int sig )
{

    switch( sig )
    {
    case SIGTERM:
        if( gSystem != NULL )
        {
            delete gSystem;
        }   // END if gM427 device exists

        Write_Message_To_LogFile( "Shutting Down", eInfoLog );

        try
        {
            gLogFile->close();
            delete gLogFile;
        }
        catch( pt::estream* es )
        {
            delete es;
        }

        // is there other memory to free here?
        exit( EXIT_SUCCESS );
        break;
    case SIGHUP:
        // --- all we want to do is reopen the log file, so signal as such
        gReopenLogFile = 1;
        break;
    }
}   // END Term_Signal_Handler

/*
 * This function is used to start the process as a daemon. In order to do this the process
 * is forked from it's original PID and a Child process is started. THe original process will
 * the exit and the child will continue (if forked properly). The new process then opens
 * the log file and closes stdout, stderr and continues.
 *
 * In order to have this run as a single process without forking, do not define DAEMON in the
 * makefile
 */
void Start_Daemon()
{
    // We want to start a child process for our daemon
    pid_t childProcess;
    pid_t sessionId;
#ifdef DAEMON
    // --- First thing to do is fork from the parent and shutdown the parent
    childProcess = fork();

    // --- Check if we forked properly
    if( childProcess < 0 )
    {
        exit( EXIT_FAILURE );
    }   // END if error forking

    // --- If we did fork properly then shutdown the parent( childProcess will equal 0 for the child so it doesn't exit)
    if( childProcess > 0 )
    {
        exit( EXIT_SUCCESS );
    }   // END child process good, exit parent

    // --- Get our session ID so we can store that in the pid file for system use ---
    sessionId = setsid();
    if( sessionId < 0 )
    {
        exit( EXIT_FAILURE );
    }   // END error getting session id

    // Change the file mode mask so to ensure that logs can be written and read properly
    umask( 0 );
#else
    sessionId = getpid();
#endif
    Open_Log_File();

    pthread_mutex_init( &gLockWriting, 0 );

    // --- Setup signal handling to properly shutdown the daemon -----
    // -- Make sure we catch TERM signals when they come in to exit gracefully
    void (*prevFcn)(int);
    prevFcn = signal(SIGTERM, Term_Signal_Handler); // register a SIGTERM handler
    if( SIG_ERR == prevFcn )
    {
        Write_Message_To_LogFile( "Error Setting SIGTERM Handler", eWarnLog );
    }   // END if SIG_ERR Returned
    else if( SIG_IGN == prevFcn )
    {
        signal( SIGTERM, SIG_IGN );
    }   // END else SIG_IGN returned

    prevFcn = signal( SIGHUP, Term_Signal_Handler);
    if( SIG_ERR == prevFcn )
    {
        Write_Message_To_LogFile( "Error Setting SIGHUP Handler", eWarnLog );
    }   // END if SIG_ERR returned
    else if( SIG_IGN == prevFcn )
    {
        signal( SIGHUP, SIG_IGN );
    }   // END if SIG_IGN returned

    // --- Create the pid file for system use ---
    FILE* fp = fopen( gCmdlineItems.mPidFile.c_str(), "w" );
    if( fp != NULL )
    {
        // -- write out the process ID and close the file ---
        fprintf( fp, "%d\n", sessionId );
        fclose( fp );
    }
    else
    {
        // -- Log if we can't write the PID file ---
        Write_Message_To_LogFile( "Error opening and writing pid file", eErrorLog );
    }

#ifdef DAEMON
    // change to root working directory
    if( ( chdir( "/" ) )  < 0 )
    {
        exit( EXIT_FAILURE );
    }   // END if change dir

    // --- Close all standard IO as we won't use that ----
    close( STDIN_FILENO );
    close( STDOUT_FILENO );
    close( STDERR_FILENO );
#endif
    Write_Message_To_LogFile( "Starting...", eInfoLog );

}   // END Start_Daemon
