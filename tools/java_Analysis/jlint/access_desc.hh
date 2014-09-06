#ifndef ACCESS_DESC_HH
#define ACCESS_DESC_HH

#include <stdarg.h>

class field_desc;

#include "class_desc.hh"
#include "functions.hh"

class access_desc { 
 public: 
  access_desc*   next;
  class_desc*    self_class;
  field_desc*    field;
  int            line;
  int            attr;
  enum { 
    a_self = 0x01, // access to the component of the same class
    a_new  = 0x02  // field of newly created object is accessed
  };

  void message(int msg_code, ...); 

  access_desc(field_desc* desc, class_desc* cls, 
              int lineno, access_desc* chain)
    { 
      field = desc;
      next = chain;
      self_class = cls;
      line = lineno;
      attr = 0;
    }
};

#endif
