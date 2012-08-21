#ifndef __DEBUG_H
#define __DEBUG_H

/**
 * @file
 * A list of debugging flags, set with Lana and the API's setDebug() call.
 */

/// trace execution - i.e. dump instructions as they run
#define LDEBUG_TRACE	1 
/// recreate each line
#define LDEBUG_RECREATE	2
/// show each line as it is fed
#define LDEBUG_SHOW	4
/// show instruction emission
#define LDEBUG_EMIT	8
/// show instruction dump after parse
#define LDEBUG_DUMP	16
/// store source filename (if set) and line number (if given)
/// using OP_SRCLINE and OP_SRCFILE
#define LDEBUG_SRCDATA 32


#endif /* __DEBUG_H */
