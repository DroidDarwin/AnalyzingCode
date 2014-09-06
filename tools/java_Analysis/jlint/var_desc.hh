#ifndef VAR_DESC_HH
#define VAR_DESC_HH

#include "utf_string.hh"

class var_desc { 
public:
  utf_string     name;
  int            type;
  int            start_pc;
  
  int4           min;
  int4           max;
  int4           mask;

  enum object_var_state {
    vs_unknown  = 0x01, // state of variable is unknown
    vs_not_null = 0x03, // variable was checked for null
    vs_new      = 0x04  // variable points to object created by new
  };
  
  const field_desc* equals;

  var_desc() {
    equals = NULL;
  }
};

#endif
