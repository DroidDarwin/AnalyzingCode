#ifndef TYPES_HH
#define TYPES_HH

#ifdef VISUAL_CPP
#include <assert.h>
#pragma warning( disable : 4786)
#endif

#include <string>
#include <vector>
#include <stddef.h>

typedef int      int4;
typedef unsigned nat4;

#if defined(__GNUC__)
#define INT8_DEFINED 1
typedef long long          int8;
typedef unsigned long long nat8;
#else
#if defined(_WIN32)
#define INT8_DEFINED 1
typedef __int64 int8;
typedef unsigned __int64 nat8;
#else
#if defined(__osf__ )
#define INT8_DEFINED 1
typedef   signed long int8;
typedef unsigned long nat8;
#endif
#endif
#endif

#define nobreak 

typedef unsigned char  byte;
typedef unsigned short word;

enum vbm_instruction_code { 
#define JAVA_INSN(code, mnem, len) mnem,
#include "jlint.d"
  last_insn
};

#define   items(array) (sizeof(array)/sizeof*(array))

enum message_category { 
  cat_deadlock        = 0x00000001, 
  cat_race_condition  = 0x00000002,
  cat_wait_nosync     = 0x00000004,
  cat_synchronization = 0x0000000F,

  cat_super_finalize  = 0x00000010,
  cat_not_overridden  = 0x00000020,
  cat_field_redefined = 0x00000040,
  cat_shadow_local    = 0x00000080,
  cat_inheritance     = 0x000000F0,

  cat_zero_operand    = 0x00000100,
  cat_zero_result     = 0x00000200, 
  cat_redundant       = 0x00000400,
  cat_overflow        = 0x00000800, 
  cat_incomp_case     = 0x00001000,
  cat_short_char_cmp  = 0x00002000,
  cat_string_cmp      = 0x00004000,
  cat_weak_cmp        = 0x00008000,
  cat_domain          = 0x00010000,
  cat_null_reference  = 0x00020000,
  cat_truncation      = 0x00040000,
  cat_bounds          = 0x00080000,
  cat_data_flow       = 0x000FFF00,

  cat_done            = 0x10000000,

  cat_all             = 0xFFFFFFFF
};

enum type_tag { 
  tp_bool,
  tp_byte,
  tp_char,
  tp_short,
  tp_int,
  tp_long,
  tp_float,
  tp_double,
  tp_void, 
  tp_self,
  tp_string,
  tp_object
};

struct int_type_range { 
  int4 min;
  int4 max;
};

struct message_descriptor {
  message_category category;
  const char*      format;
  const char*      name;
  bool             position_dependent;
  bool             enabled;
};


enum message_code { 
#define MSG(category, code, pos, text) msg_##code,
#include "jlint.msg"
  msg_last
};

#define MSG_LOCATION_PREFIX "%0s:%1d: " // Emacs style: "file:line_number: "

#define MAX_MSG_LENGTH      1024
#define MAX_MSG_PARAMETERS  16

const unsigned class_hash_table_size = 1987;

struct msg_select_category_option { 
  message_category msg_cat;
  const char*      cat_name;
  const char*      cat_desc;
};

class field_desc;

struct vbm_operand { 
  int  type;  // type of expression/variable before it was pushed in stack
  int4 max;   // maximal value of operand
  int4 min;   // minimal value of operand
  int4 mask;  // mask of possible set bits and zero value indicator for 
  // object types
  int  index; // index of local veriable, which value was loaded in stack
  const field_desc* equals;
};

#define IS_INT_TYPE(tp) (tp <= tp_int)
#define IS_ARRAY_TYPE(tp) ((tp & ~0xFF) != 0)

int_type_range const ranges[] = { 
  //   min         max
  {0x00000000, 0x00000001}, // tp_bool
    {0xffffff80, 0x0000007f}, // tp_byte 
      {0x00000000, 0x0000ffff}, // tp_char
        {0xffff8000, 0x00007fff}, // tp_short
          {0x80000000, 0x7fffffff}  // tp_int
};

int const array_type[] = { 
  0,
    0,
    0,
    0,
    tp_bool,
    tp_char,
    tp_float,
    tp_double,
    tp_byte,
    tp_short,
    tp_int,
    tp_long
    };

int const vbm_instruction_length[] = {
#define JAVA_INSN(code, mnem, len) len,
#include "jlint.d"
  0
    };

char const* const vbm_instruction_mnemonic[] = {
#define JAVA_INSN(code, mnem, len) #mnem,
#include "jlint.d"
  NULL
    };

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

#ifdef INT8_DEFINED
#define TO_INT8(high, low)        ((nat8(high) << 32) | unsigned(low))
#define LOW_PART(x)               int4(x)
#define HIGH_PART(x)              int4(nat8(x) >> 32)

#define LOAD_INT8(src,field)      TO_INT8((src)[0].field, (src)[1].field)
#define STORE_INT8(dst,field,src) (dst)[0].field = HIGH_PART(src),\
          (dst)[1].field = LOW_PART(src) 

#ifndef INT8_MAX
#define INT8_MAX      ((int8)((nat8)-1 >> 1))
#endif
#ifndef INT8_MIN
#define INT8_MIN      ((int8)(((nat8)-1 >> 1) + 1))
#endif
#define INT8_ZERO     ((int8)0)
#define INT8_ALL_BITS ((int8)-1)
#endif

#define MAX_ARRAY_LENGTH 0x7fffffff

#define ALL_BITS 0xffffffff
#define SIGN_BIT 0x80000000

#define NO_ASSOC_VAR -1

#ifdef _WIN32
#define FILE_SEP '\\'
#else
#define FILE_SEP '/'
#endif

// declared in jlint.cc:
extern int max_shown_paths;
extern char* source_file_path;
extern int   source_file_path_len;
extern bool  source_path_redefined;
extern int n_messages;

#endif
