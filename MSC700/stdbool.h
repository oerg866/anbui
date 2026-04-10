/* STDBOOL.H Implementation For Microsoft C 7.00 */

#ifndef _STDBOOL_H_
#define _STDBOOL_H_

#if !defined(bool)
typedef enum { false, true } _myBool;
#define bool _myBool
#endif

#endif
