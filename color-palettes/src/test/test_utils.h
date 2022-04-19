#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <pse_platform.h>

#include <stdlib.h>
#include <stdio.h>

/******************************************************************************
 * 
 * PSE TEST MACRO to simplify the code
 *
 ******************************************************************************/

#define CHECK(a,b)   if((a)!=(b)) { assert(false); exit(1); }
#define NCHECK(a,b)  if((a)==(b)) { assert(false); exit(1); }

#endif /* TEST_UTILS_H */
