/******************************************************************************
*                                                                             *
*   Module:     versions.h                                                    *
*                                                                             *
*   Date:       03/03/01                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This header file deals with differences in kernel versions.
        All differences should be abstracted here, mostly in the form of
        preprocessor macros.

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 03/03/01   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _VERSIONS_H_
#define _VERSIONS_H_

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

#include <linux/autoconf.h>

//-----------------------------------------------------------------------------
// Deal with CONFIG_MODVERSIONS

#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
#define MODVERSIONS
#endif

#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif

#ifdef CONFIG_SMP
#define __SMP__ 1
#endif

#include <linux/kernel.h>
#define __NO_VERSION__
#include <linux/module.h>  

//-----------------------------------------------------------------------------
// In 2.2.3 /usr/include/linux/version.h includes a macro for this, 
//  but 2.0.35 doesn't - so add it here if necessary

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) ((a)*65536+(b)*256+(c))
#endif

//-----------------------------------------------------------------------------
// Deal with the different function prototype for different kernel versions
//-----------------------------------------------------------------------------
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)

/* This function is called when a process closes the 
 * device file. It doesn't have a return value in 
 * version 2.0.x because it can't fail (you must ALWAYS
 * be able to close a device). In version 2.2.x it is 
 * allowed to fail - but we won't let it. 
 */

#define DEV_CLOSE_RET           int
#define DEV_CLOSE_RET_VAL       (0)

/* flush call is added in the kernel version 2.2.0 */

#define FLUSH_FOPS(function)    function,


#else // LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)

#define DEV_CLOSE_RET           void
#define DEV_CLOSE_RET_VAL

#define FLUSH_OPS(function)

#endif // LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)


#endif //  _VERSIONS_H_
