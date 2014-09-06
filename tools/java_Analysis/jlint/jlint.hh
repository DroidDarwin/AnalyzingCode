//-< JLINT.H >-------------------------------------------------------+--------+
// Jlint                      Version 3.1        (c) 1998  GARRET    |     ?  |
// (Java Lint)                                                       |   /\|  |
//                                                                   |  /  \  |
//   Created:                    28-Mar-98    K.A. Knizhnik          | / [] \ |
//   Version 2.X:   Last update: 08-Aug-01    Cyrille Artho          | GARRET |
//   Version 3.0:   Last update: 20-Jul-03    Raphael Ackermann      |        |
//   Version 3.1.X: Last update: 15-Jan-13    Cyrille Artho          |        |
//-------------------------------------------------------------------+--------+
// Java verifier 
//-------------------------------------------------------------------+--------+

#ifndef __JLINT_HH__
#define __JLINT_HH__

#define VERSION "3.1.3"

#include "types.hh"
#include "message_node.hh"
#include "utf_string.hh"
#include "method_desc.hh"
#include "field_desc.hh"
#include "class_desc.hh"
#include "constant.hh"
#include "callee_desc.hh"
#include "access_desc.hh"
#include "graph.hh"
#include "component_desc.hh"
#include "var_desc.hh"
#include "local_context.hh"
#include "overridden_method.hh"
#include "string_pool.hh"

enum const_types { 
    c_none,
    c_utf8,
    c_reserver,
    c_integer,
    c_float,
    c_long,
    c_double, 
    c_class,
    c_string,
    c_field_ref,
    c_method_ref,
    c_interface_method_ref,
    c_name_and_type
};

//
// Constants for extracting zip file
//

#define LOCAL_HDR_SIG     "\113\003\004"   /*  bytes, sans "P" (so unzip */

//--- ZIP_local_file_header layout ---------------------------------------------
#      define LREC_SIZE                         26 /* lengths of local file headers */
#      define L_VERSION_NEEDED_TO_EXTRACT_0     0
#      define L_VERSION_NEEDED_TO_EXTRACT_1     1
#      define L_GENERAL_PURPOSE_BIT_FLAG        2
#      define L_COMPRESSION_METHOD              4
#      define L_LAST_MOD_FILE_TIME              6
#      define L_LAST_MOD_FILE_DATE              8
#      define L_CRC32                           10
#      define L_COMPRESSED_SIZE                 14
#      define L_UNCOMPRESSED_SIZE               18
#      define L_FILENAME_LENGTH                 22  //used
#      define L_EXTRA_FIELD_LENGTH              24  //used
 
 //--- ZIP_central_directory_file_header layout ---------------------------------
#      define CREC_SIZE  42  /* length of directory headers */
#      define C_VERSION_MADE_BY_0               0
#      define C_VERSION_MADE_BY_1               1
#      define C_VERSION_NEEDED_TO_EXTRACT_0     2
#      define C_VERSION_NEEDED_TO_EXTRACT_1     3
#      define C_GENERAL_PURPOSE_BIT_FLAG        4
#      define C_COMPRESSION_METHOD              6 //used new
#      define C_LAST_MOD_FILE_TIME              8
#      define C_LAST_MOD_FILE_DATE              10
#      define C_CRC32                           12
#      define C_COMPRESSED_SIZE                 16 //used
#      define C_UNCOMPRESSED_SIZE               20 // used new
#      define C_FILENAME_LENGTH                 24 //used
#      define C_EXTRA_FIELD_LENGTH              26 //used
#      define C_FILE_COMMENT_LENGTH             28
#      define C_DISK_NUMBER_START               30
#      define C_INTERNAL_FILE_ATTRIBUTES        32
#      define C_EXTERNAL_FILE_ATTRIBUTES        34
#      define C_RELATIVE_OFFSET_LOCAL_HEADER    38 //used

//--- ZIP_end_central_dir_record layout ----------------------------------------
#      define ECREC_SIZE                        18  /* length of central dir record */
/*      define SIGNATURE                        0   space-holder only */
#      define NUMBER_THIS_DISK                  4
#      define NUM_DISK_WITH_START_CENTRAL_DIR   6
#      define NUM_ENTRIES_CENTRL_DIR_THS_DISK   8
#      define TOTAL_ENTRIES_CENTRAL_DIR         10 //used
#      define SIZE_CENTRAL_DIRECTORY            12 //used
#      define OFFSET_START_CENTRAL_DIRECTORY    16
#      define ZIPFILE_COMMENT_LENGTH            20


// -- possible values at place C_COMPRESSION_METHOD
#      define C_DEFLATE                         8
#	define C_UNCOMPRESSED			0

#endif
