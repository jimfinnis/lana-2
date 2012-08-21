#ifndef __FLAGS_H
#define __FLAGS_H

/**
 * @file
 * assorted operation mode flags, which can be set in Lana or
 * the API with setFlags().
 */

/// strip comment and blankline instructions on parsing
#define LOP_STRIPCOMMENTS 1
/// do not attempt to execute the code - just parse it
#define LOP_NORUN 2

#endif /* __FLAGS_H */
