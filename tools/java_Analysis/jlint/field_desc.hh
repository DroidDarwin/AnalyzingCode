#ifndef FIELD_DESC_HH
#define FIELD_DESC_HH

#ifdef VISUAL_CPP
using namespace std;
#pragma warning (disable : 4786)
#endif

#include "component_desc.hh"
#include "utf_string.hh"
class const_name_and_type;
class class_desc;

class field_desc : public component_desc {
public:
  field_desc*    next;
  int            attr;
  enum { 
    f_static     = 0x0008, 
    f_final      = 0x0010,
    f_volatile   = 0x0040, 

    f_used       = 0x10000,
    f_serialized = 0x20000 // field is accessed only from methods
    // of related classes
  };

  const field_desc* equals; // value that has been assigned, or NULL == unknown
  const_name_and_type* name_and_type; // NULL == unknown
  vector<int> writes;

  void write(int linenumber) {
    // mark field as written to
    writes.push_back(linenumber);
  }

  field_desc(utf_string const& field_name, class_desc* owner, 
             field_desc* chain) 
    : component_desc(field_name, owner)
  {
    equals = NULL;
    name_and_type = NULL;
    next = chain; 
    attr = f_serialized;
  }
};

#endif
