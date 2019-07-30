/*
 * Files.cpp
 *
 *  Created on: Jun 17, 2010
 *      Author: jsuriano
 */
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>

#include "Files.h"

void Print_Dir_Error( const std::string& dir, int errorNum )
{
    switch( errorNum )
    {
    case EACCES:
        printf( "DirListing: Error: Permission denied to directory %s\n", dir.c_str() );
        break;
    case EMFILE:
        printf( "DirListing: Error: Too many file descriptors in use by process (%s)\n", dir.c_str() );
        break;
    case ENFILE:
        printf( "DirListing: Error: Too many files are currently open in the system (%s)\n", dir.c_str() );
        break;
    case ENOENT:
        printf( "DirListing: Error: Directory does not exist, or name is an empty string (%s)\n", dir.c_str() );
        break;
    case ENOMEM:
        printf( "DirListing: Error: Insufficient memory to complete the operation (%s)\n", dir.c_str() );
        break;
    case ENOTDIR:
        printf( "DirListing: Error: %s is not a directory\n", dir.c_str() );
        break;
    default:
        printf( "DirListing: Error: Unknown error (%d) when opening directory %s\n", errorNum, dir.c_str() );
        break;
    }   // END switch
}   // END Print_Error

int Get_SharedLib_List( const std::string& dir, const std::string extension, std::vector<std::string> &files )
{
    DIR *dp;
    dirent *dirp;
    int extLength = extension.length();

    if( ( dp  = opendir( dir.c_str() ) ) == NULL )
    {
        int errorNum = errno;
        Print_Dir_Error( dir, errorNum );
        return errno;
    }

    while( ( dirp = readdir( dp ) ) != NULL )
    {
        //printf("Looking at: %s\n", dirp->d_name );
        std::string tmp = dirp->d_name;
        if( ( tmp.compare( "." ) != 0 ) && ( tmp.compare( ".." ) != 0 ) )
        {
            // --- make sure this is a file. If not then it's something else and we will handle separately
            if( ( DT_UNKNOWN == dirp->d_type ) || ( DT_REG == dirp->d_type ) )
            {
                /*
                 * We are only interested in shared libraries so assure that the file
                 * has a .so extension, otherwise we will ignore it.
                 */
                std::string tmp = dirp->d_name;
                if( tmp.size() >= 3 )
                {
                    if( tmp.compare( tmp.size()-extLength, extLength, extension ) == 0 )
                    {
                        std::string currFile = dir + "/";
                        currFile += dirp->d_name;
                        files.push_back( currFile );
                    }   // END if compare extension
                }   // END tmp.size >3
            }   // END if type == UNKNOWN or REG
            else if ( DT_DIR == dirp->d_type )
            {
                std::string subdir = dir + "/";
                subdir += dirp->d_name;
                //printf(" Got a directory %s, %d\n", subdir.c_str(), dirp->d_type);

                Get_SharedLib_List( subdir, extension, files );
            }   // END else directory
        }   // END if name != . or ..
    }   // END while loop
    closedir(dp);
    return 0;
}
