/////////////////////////////////////////////////////////////////////////////
//
// @file logging.h @brief Digital Devices Common Interface plugin for VDR.
//
// Copyright (c) 2013 - 2014 by Jasmin Jessich.  All Rights Reserved.
//
// Contributor(s):
//
// License: GPLv2
//
// This file is part of vdr_plugin_ddci2.
//
// vdr_plugin_ddci2 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// vdr_plugin_ddci2 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with vdr_plugin_ddci2.  If not, see <http://www.gnu.org/licenses/>.
//
// $Id:  $
/////////////////////////////////////////////////////////////////////////////

#ifndef __LOGGING_H
#define __LOGGING_H

// configuration defaults
#ifndef CNF_LOG_FUNCTIONS
#define CNF_LOG_FUNCTIONS 0
#endif

/// global loglevel variable
extern int LogLevel;

// Default loglevel is Info
static const int LL_DEFAULT = 2;

#define M_START  do {
#define M_END    } while(0)
#define M_EMPTY  M_START M_END

#define LOG_X(ll, sev, txt, a...) \
	M_START if (LogLevel >= ll) { syslog_with_tid(sev, txt a); } M_END

#define L_ALL(a...)  LOG_X( 0, LOG_ERR,   "DDCI: ",     a )
#define L_ERR(a...)  LOG_X( 1, LOG_ERR,   "DDCI-Err: ", a )
#define L_INF(a...)  LOG_X( 2, LOG_INFO,  "DDCI-Inf: ", a )
#define L_DBG(a...)  LOG_X( 3, LOG_DEBUG, "DDCI-Dbg: ", a )

// Print file/line and strerror(errno)
#define L_ERROR_X(a...)  LOG_X( 1, LOG_ERR, "", a )
#define L_ERROR()        L_ERROR_X( "DDCI-Err (%s,%d): %m", __FILE__, __LINE__ )
#define L_ERROR_STR(s)   L_ERROR_X( "DDCI-Err (%s,%d): %s: %m", __FILE__, __LINE__, s )

#define L_FUNC_X(a...)          LOG_X( 3, LOG_DEBUG, "", a )
#define L_FUNC()                L_ERROR_X( "DDCI-Dbg (%s)", __FUNCTION__ )
#define L_FUNC_STR(s)           L_ERROR_X( "DDCI-Dbg (%s) %s", __FUNCTION__, s )
#define L_FUNC_PRINTF(f, a...)  L_ERROR_X( "DDCI-Dbg (%s)" f, __FUNCTION__, ##a )

#if CNF_LOG_FUNCTIONS
# define LOG_FUNCTION_ENTER            L_FUNC_STR( "enter" )
# define LOG_FUNCTION_INFO(s)          L_FUNC_STR( s )
# define LOG_FUNCTION_PRINTF(f, a...)  L_FUNC_PRINTF( f, ##a )
# define LOG_FUNCTION_EXIT             L_FUNC_STR( "leave" )
#else
# define LOG_FUNCTION_ENTER            M_EMPTY
# define LOG_FUNCTION_INFO(s)          M_EMPTY
# define LOG_FUNCTION_PRINTF(f, a...)  M_EMPTY
# define LOG_FUNCTION_EXIT             M_EMPTY
#endif

#endif // __LOGGING_H
