// extern function declarations

#ifndef FUNCTIONS_HH
#define FUNCTIONS_HH

#include <stdio.h>
#include "types.hh"
#include <stdarg.h>
class utf_string;

/* The following preprocessor directives are historically grown, if anyone
   wanted to clean up, I'd be very grateful :) */
#ifndef __VALIST
#ifdef va_list
#define __VALIST va_list
#endif
// FreeBSD uses typedef in stdarg.h
#ifdef __FreeBSD__
#define __VALIST va_list
#endif
// so does OpenBSD
#ifdef __OpenBSD__
#define __VALIST va_list
#endif
// so does LinuxPPC
#ifdef __PPC__
#define __VALIST va_list
#endif
// so does MacOSX
#ifdef __APPLE_CC__
#define __VALIST va_list
#endif
// so does MSVC++
#ifdef VISUAL_CPP
#define __VALIST va_list
#endif
// so does alpha
#ifdef __alpha__
#define __VALIST va_list
#endif

// so does GNUC
#ifdef __GNUC__
#define __VALIST va_list
#endif

/* replaced by version above found in jlint for debian distro
// Fix for g++3.2, not needed for g++3.0 or lower. g++3.1?? 
#ifdef __GNUC__
#if __GNUC__ > 2
#ifdef __GNUC_MINOR__
#if __GNUC_MINOR__ > 0
#define __VALIST va_list
#endif
#endif
#endif
#endif
*/

// Fix for cygwin (and possible others), if va_list typedef'd or undefined
#ifndef __VALIST
#define __VALIST void*
#endif
#endif

extern void format_message(int code, utf_string const& file, int line, __VALIST ap);
extern void message_at(int code, utf_string const& file, int line, ...);
extern int get_type(utf_string const& str);
extern int get_number_of_parameters(utf_string const& str);
extern unsigned string_hash_function(byte* p);

#endif
