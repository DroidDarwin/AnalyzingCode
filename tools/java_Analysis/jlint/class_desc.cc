#include "class_desc.hh"

class_desc::class_desc(utf_string const& str) 
  : name(str), source_file(str+".java", true) 
{ 
  fields = NULL;
  methods = NULL;
  n_bases = 0;
  attr = cl_system;
  class_vertex = new graph_vertex(this);
  metaclass_vertex = new graph_vertex(this);
  next = chain;
  chain = this;
}

class_desc* class_desc::get(utf_string const& str)
{
  unsigned h = str.hash() % class_hash_table_size;
  class_desc* cls;
  for (cls = hash_table[h]; cls != NULL; cls = cls->collision_chain) { 
    if (str == cls->name) { 
      return cls;
    }
  }
  cls = new class_desc(str);
  cls->collision_chain = hash_table[h];
  hash_table[h] = cls;
  n_classes += 1;
  return cls;
}

method_desc* class_desc::get_method(utf_string const& mth_name,
                                    utf_string const& mth_desc) 
{ 
  for (method_desc* method = methods; method != NULL; method = method->next){
    if (method->name == mth_name && method->desc == mth_desc) {
      return method;
    }
  }
  return methods = new method_desc(mth_name, mth_desc, this, methods);
}

field_desc* class_desc::get_field(utf_string const& field_name)
{ 
  for (field_desc* field = fields; field != NULL; field = field->next) { 
    if (field->name == field_name) return field;
  }
  return fields = new field_desc(field_name, this, fields);
}

bool class_desc::isa(const char* cls_name)
{
  for (class_desc* cls = this; cls->n_bases != 0; cls = *cls->bases) { 
    if (cls->name == cls_name) return true;
  }
  return false;
}

bool class_desc::isa(class_desc* base_class)
{
  for (class_desc* cls = this; cls->n_bases != 0; cls = *cls->bases) { 
    if (cls == base_class) return true;
  }
  return false;
}

bool class_desc::implements(const char* interface_name)
{
  if (name == interface_name) { 
    return true;
  }
  for (int i = n_bases; --i >= 0;) { 
    if (bases[i]->implements(interface_name)) {  
      return true;
    }
  }
  return false;    
}

bool class_desc::in_relationship_with(class_desc* cls)
{
  return isa(cls) || cls->isa(this);
}

void class_desc::check_inheritance(class_desc* derived)
{
  for (field_desc* bf = fields; bf != NULL; bf = bf->next) {
    for (field_desc* df = derived->fields; df != NULL; df = df->next) {
      if (bf->name == df->name) { 
        message_at(msg_field_redefined, derived->source_file, 0, 
                   &bf->name, derived, this);
        break;
      }
    }
  }
  for (method_desc* bm = methods; bm != NULL; bm = bm->next) { 
    if (bm->is_special_method() || (bm->attr & method_desc::m_static)) { 
      continue;
    }
    for (method_desc* dm=derived->methods; dm != NULL; dm=dm->next) {
      if ( (bm->name == dm->name) && (bm->desc == dm->desc)) { 
        if (!(dm->attr & method_desc::m_override)) { 
          bm->overridden = 
            new overridden_method(dm, bm->overridden);
          if ((bm->attr & method_desc::m_synchronized) &&
              !(dm->attr & method_desc::m_synchronized))
          {
            message_at(msg_nosync, derived->source_file, 
                       dm->first_line, bm, derived);
          }
          if (!(attr & cl_interface)) { 
            dm->attr |= method_desc::m_override;
          }
        }
        break;
      }
    }
  }

  /* find possible override mistakes */
  for (method_desc* bm = methods; bm != NULL; bm = bm->next) { 
    if (bm->is_special_method() || (bm->attr & method_desc::m_static)) { 
      continue;
    }
    bool overridden = false;
    method_desc* match = NULL;
    for (method_desc* dm=derived->methods; dm != NULL; dm=dm->next) {
      if (bm->name == dm->name) { 

        if (!(dm->attr & method_desc::m_override))
          match = dm;

        if (bm->desc == dm->desc) {
          overridden = true;
          break;
        }
      }
    }
    if (match != NULL && !overridden) { 
      message_at(msg_not_overridden, derived->source_file, 
                 match->first_line, bm, derived);
    }
  }
  
  for (int n = 0; n < n_bases; n++) { 
    bases[n]->check_inheritance(derived);
  }
}

void class_desc::calculate_attributes()
{
  for (method_desc* method = methods; method != NULL; method = method->next){
    method->calculate_attributes();
  }
}

void class_desc::build_concurrent_closure()
{
  for (method_desc* method = methods; method != NULL; method = method->next){
    method->build_concurrent_closure();
  }
}

void class_desc::verify()
{
  for (method_desc* method = methods; method != NULL; method=method->next) {
    method->check_synchronization();
    method->check_invocations();
  }
}

void class_desc::build_class_info()
{
  for (int i = 0; i < n_bases; i++) { 
    bases[i]->check_inheritance(this);
  }
  for (method_desc* method = methods; method != NULL; method = method->next){
    method->find_access_dependencies();
  }
  bool overriden_equals = false;
  bool overriden_hashcode = false;
  method_desc* equals_hashcode_match = NULL;
  for (method_desc* bm = methods; bm != NULL; bm = bm->next) { 
    if (bm->name == "equals") {
      overriden_equals = true;
      equals_hashcode_match = bm;
    } else if (bm->name == "hashCode") {
      overriden_hashcode = true;
      equals_hashcode_match = bm;
    }
  }
  if (overriden_equals != overriden_hashcode) {
    if (overriden_equals) {
      message_at(msg_hashcode_not_overridden, source_file, 
                 equals_hashcode_match->first_line);
    } else {
      message_at(msg_equals_not_overridden, source_file, 
                 equals_hashcode_match->first_line);
    }
  }
}

void class_desc::build_call_graph()
{
  for (method_desc* method = methods; method != NULL; method = method->next){
    method->build_call_graph();
  }
}

void class_desc::global_analysis()
{
  class_desc* cls;

  //
  // Set attributes which depends on inheritance hierarchy. 
  //
  for (cls = chain; cls != NULL; cls = cls->next) { 
    cls->calculate_attributes();
  } 

  //
  // Set m_concurrent attribute for all synchronized methods and methods
  // called from them. Detect invocation of wait() method with multiple
  // monitors locked.
  //
  for (cls = chain; cls != NULL; cls = cls->next) { 
    cls->build_concurrent_closure();
  } 
    
  for (cls = chain; cls != NULL; cls = cls->next) { 
    cls->build_class_info();
  }
    
  for (cls = chain; cls != NULL; cls = cls->next) { 
    cls->build_call_graph();
  }
    
  //
  // Check non-synchrnized access to variables by concurrent methods
  // (race condition) and detect unchecked accessed to formal parameters
  // of method which can be passed "null" value in some invocation of 
  // this method
  //
  for (cls = chain; cls != NULL; cls = cls->next) { 
    cls->verify();
  }
    
  //
  // Explore class dependency grpah to detect possible sources of 
  // deadlocks
  //
  graph_vertex::verify();
}
