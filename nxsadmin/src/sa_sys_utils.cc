/***************************************************************************
 *   Copyright (C) 2007 by Maxim Stjazhkin                                 *
 *   maxt_t@drohobych.com.ua                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "sa_sys_utils.h"
//----------------------------------------------------------------------------
#define TIME_STRING_BUF 50
//----------------------------------------------------------------------------
Glib::ustring System::timeString(time_t t)
{
    struct tm * local;
    char timeBuf[TIME_STRING_BUF];
    
    local = localtime(&t);
    strftime(timeBuf, TIME_STRING_BUF, "%c", local);
    
    Glib::ustring retVal(timeBuf);
    
    return retVal;
}
//----------------------------------------------------------------------------
void System::getTimeStr(char * aTimeString)
{
    time_t currt;
    tm * t;
    
    char timeString[40];
    
    currt = time(0);
    t = localtime(&currt);
    
    strftime(timeString, sizeof(timeString), "%d-%m-%Y %H:%M:%S", t);
    
    strcpy(aTimeString, timeString);
}
//----------------------------------------------------------------------------
gint System::spawn(const char * aProgram, char * const aArgList[])
{
    // Make copy of current process
    pid_t childPid = fork();
    
    if (childPid != 0)
    {
        // This is parent process
        return childPid;
    }
    else
    {
        if (execvp(aProgram, aArgList) < 0)
        {
            if (errno)
            {
                perror("execvp");
                return -1;
            }
        }
    }
    return 0;
}
