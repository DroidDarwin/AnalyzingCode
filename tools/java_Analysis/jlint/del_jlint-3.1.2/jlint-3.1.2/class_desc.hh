#ifndef CLASS_DESC_HH
#define CLASS_DESC_HH

#include "types.hh"
#include "locks.hh"
#include "utf_string.hh"
#include "field_desc.hh"
#include "graph.hh"
#include "method_desc.hh"
#include "overridden_method.hh"

class class_desc { 
public:
  Locks locks; // locks held by current thread
  Locks usedLocks; // locks (other than "this") ever used by current class

  utf_string     name;
  utf_string     source_file;
  class_desc*    next;
  class_desc*    collision_chain; 

  method_desc*   methods;

  int            attr; 
  enum class_attrs { 
    cl_interface = 0x00200,
    cl_system    = 0x10000      
  };

  int            n_bases;
  class_desc**   bases;

  field_desc*    fields;

  graph_vertex*  class_vertex;
  graph_vertex*  metaclass_vertex;

  static class_desc* get(utf_string const& str);

  method_desc* get_method(utf_string const& mth_name, 
                          utf_string const& mth_desc);

  field_desc* get_field(utf_string const& field_name);

  static class_desc* hash_table[];
  static int         n_classes;
  static class_desc* chain;

  bool isa(const char* cls_name);
  bool isa(class_desc* cls);
  bool implements(const char* interface_name);
  bool in_relationship_with(class_desc* cls);

  void verify();
  void calculate_attributes();
  void build_class_info();
  void build_call_graph();
  void build_concurrent_closure();
  void check_inheritance(class_desc* derived);

  static void global_analysis();

  class_desc(utf_string const& str);
};

#endif
