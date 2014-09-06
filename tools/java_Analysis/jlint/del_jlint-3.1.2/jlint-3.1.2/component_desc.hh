#ifndef COMPONENT_DESC_HH
#define COMPONENT_DESC_HH

class class_desc;
#include "utf_string.hh"

class component_desc { 
public:
  utf_string     name;
  class_desc*    cls;
  class_desc*    accessor;
    
  component_desc(utf_string const& component_name, class_desc* component_cls)
    : name(component_name), cls(component_cls), accessor(NULL) {}
};

#endif
