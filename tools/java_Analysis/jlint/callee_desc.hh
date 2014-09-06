#ifndef CALLEE_DESC_HH
#define CALLEE_DESC_HH

#include <stddef.h>
#include <stdarg.h>

class method_desc;
class field_desc;
class constant;
#include "class_desc.hh"
#include "functions.hh"

class callee_desc {
 public:
  class_desc*    self_class;
  method_desc*   method;
  callee_desc*   next;

  void*          backtrace;
  int            line;
  int            attr;
  enum { 
    i_self          = 0x01, // invoke method of self class
    i_synchronized  = 0x02, // method is invoked from synchronized(){} body
    i_wait_deadlock = 0x04  // invocation can cause deadlock in wait()
  };

  void message(int msg_code, ...); 
  
  callee_desc(class_desc* cls, method_desc* mth, callee_desc* chain, 
              int lineno, int call_attr) 
    { 
      self_class = cls;  
      method = mth;
      next = chain;
      line = lineno;
      attr = call_attr;
      backtrace = NULL;
    }
};

#endif
