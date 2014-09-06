#ifndef METHOD_DESC_HH
#define METHOD_DESC_HH

class graph_vertex;
class overridden_method;
class access_desc;
class callee_desc;

#include "types.hh"
#include "inlines.hh"
#include "component_desc.hh"
#include "var_desc.hh"
#include "constant.hh"
#include "class_desc.hh"
#include "callee_desc.hh"
#include "field_desc.hh"
#include "local_context.hh"
#include "functions.hh"
#include "access_desc.hh"
#include "string_pool.hh"
#include "locks.hh"
class local_context;

class method_desc : public component_desc {

public:
  utf_string     desc;
  method_desc*   next;

  int attr;
  enum { 
    m_static       = 0x0008,
    m_final        = 0x0010,
    m_synchronized = 0x0020,
    m_native       = 0x0100,
    m_abstract     = 0x0400,

    m_wait         = 0x010000, // invoke wait()
    m_serialized   = 0x020000, // method is called only from methods 
    // of related classes
    m_concurrent   = 0x040000, // Method is either run of Runnable protocol
    // or synchronized or called from them.
    m_visited      = 0x080000, // Used while recursive traversal of methods
    m_deadlock_free= 0x100000, // Doesn't call any synchronized methods
    m_override     = 0x200000, // Override method of base class

    m_sync_block   = 0x400000 
    // is called from another method, within a synchronized block
  };

  int            n_vars;
  var_desc*      vars;
  int*           var_store_count;

  bool           local_variable_table_present;
  int            in_monitor;

  callee_desc*   callees;
  access_desc*   accessors;
    
  graph_vertex*  vertex;

  //
  // Chain of methods from derived classes, overriding this method
  //
  overridden_method* overridden; 

  //
  // 1 bit in position 'i' of 'null_parameter_mask' means that NULL is 
  // passed as the value of parameter 'i'
  //
  unsigned       null_parameter_mask; 
  //
  // 1 bit in position 'i' of 'unchecked_use_mask' means that formal
  // parameter 'i' is used without check for NULL
  //
  unsigned       unchecked_use_mask;    

  int            code_length;
  byte*          code;

  local_context** context;

  int            first_line; // line corresponing to first method instruction
  int            wait_line;  // line of last wait() invocation in the method
  word*          line_table;

  Locks locksAtEntry; // locks (potentially) held when method is called
  // needed to avoid double insertion of "lock -> method" into call graph

  int  demangle_method_name(char* buf);

  void calculate_attributes();

  void find_access_dependencies();

  void build_concurrent_closure();
  void add_to_concurrent_closure(callee_desc* caller, 
                                 int call_attr, int depth);

  void build_call_graph();
  bool build_call_graph(method_desc* caller, callee_desc* callee,
                        int call_attr);

  int  print_call_path_to(callee_desc* target, int loop_id, int path_id, 
                          int call_attr = 0, callee_desc* prev = NULL);

  void check_synchronization();
  void check_invocations();

  bool is_special_method() { return name.first_char() == '<'; }

  int  get_line_number(int pc);
  void message(int msg_code, int pc, ...);

  void check_variable_for_null(int pc, vbm_operand* sp); 
  void check_array_index(int pc, vbm_operand* sp); 

  void basic_blocks_analysis();

  void parse_code(constant** constant_pool, const field_desc* is_this);

  method_desc(utf_string const& mth_name, utf_string const& mth_desc, 
              class_desc* cls_desc, method_desc* chain) 
    : component_desc(mth_name, cls_desc), desc(mth_desc)
    { 
      callees = NULL;
      accessors = NULL;
      vertex = NULL;
      attr = m_serialized;
      next = chain;
      first_line = 0;
      overridden = NULL;
      local_variable_table_present = false;
      null_parameter_mask = unchecked_use_mask = 0;
      new_cnt = 0;
      // equal_descs.insert(equal_descs.begin(), NULL);
    }

private:
  int new_cnt;
  // typedef vector<field_desc*> Tequal_descs; // field_descs for "equal" field
  // Tequal_descs equal_descs; // field_descs for "equal" field
  field_desc* getNew(); // returns new field desc* and increments counter
  // returns "first.second", as a string allocated in the string pool
};

const char* compound_name(const char* first, const char* second);
extern string_pool stringPool; // declared in jlint.cc
extern field_desc* is_const;
#endif
