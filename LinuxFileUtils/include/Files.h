/*
 * Files.h
 *
 *  Created on: Jun 17, 2010
 *      Author: jsuriano
 */

#ifndef FILES_H_
#define FILES_H_
#include <string>
#include <vector>

int Get_SharedLib_List( const std::string& dir, const std::string extension, std::vector<std::string> &files );

#endif /* FILES_H_ */
