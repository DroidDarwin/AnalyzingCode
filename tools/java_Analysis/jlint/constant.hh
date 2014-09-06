#ifndef CONSTANT_HH
#define CONSTANT_HH

#include "types.hh"
#include "utf_string.hh"

class constant { 
 public:
  byte tag;
  virtual int length() = 0;
  virtual type_tag type() { return tp_object; }
  constant(byte* p) { tag = *p; }
  constant() { tag = 0; } // for is_this
};

class const_class : public constant {
 public:   
  int name;
  const_class(byte* p) : constant(p) {
    name = unpack2(p+1);
  }
  int length() { return 3; }
};

class const_double : public constant {
 public:
  const_double(byte* p) : constant(p) {}
  int length() { return 9; }
  type_tag type() { return tp_double; }
};

class const_float : public constant {
 public:
  const_float(byte* p) : constant(p) {}
  int length() { return 5; }
  type_tag type() { return tp_float; }
};

class const_int : public constant {
 public:
  int value;
  const_int(byte* p) : constant(p) {
    value = unpack4(p+1);
  }
  int length() { return 5; }
  type_tag type() { return tp_int; }
};

class const_long : public constant {
 public:
  struct { 
    int4 high;
    int4 low;
  } value;
  const_long(byte* p) : constant(p) {
    value.high = unpack4(p+1);
    value.low  = unpack4(p+5);
  }
  int length() { return 9; }
  type_tag type() { return tp_long; }
};

class const_name_and_type : public constant {
 public:   
  int name;
  int desc;
  const_name_and_type(byte* p) : constant(p) {
    name = unpack2(p+1);
    desc = unpack2(p+3);
  }
  const_name_and_type(int n, int t) { // used for is_this
    name = n;
    desc = t;
  }
  int length() { return 5; }
};

class const_ref : public constant {
 public:   
  int cls;
  int name_and_type;
  const_ref(byte* p) : constant(p) {
    cls = unpack2(p+1);
    name_and_type = unpack2(p+3);
  }
  int length() { return 5; }
};

class const_string : public constant {
 public:   
  int str;
  const_string(byte* p) : constant(p) {
    str = unpack2(p+1);
  }
  int length() { return 3; }
  type_tag type() { return tp_string; }
};

class const_utf8 : public constant, public utf_string { 
 public:
  const_utf8(byte* p) : constant(p), utf_string(unpack2(p+1), p+3) {}
  int length() { return 3 + len; }
};

#endif
