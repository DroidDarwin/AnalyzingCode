#ifndef LOCAL_CONTEXT_HH
#define LOCAL_CONTEXT_HH

#include "types.hh"
#include "var_desc.hh"
#include "method_desc.hh"

class local_context { 
public:
  enum context_cmd { 
    cmd_pop_var,    // start of local variable scope
    cmd_push_var,   // end of local variable scope
    cmd_merge_ctx,  // forward jump label
    cmd_reset_ctx,  // backward jump label
    cmd_enter_ctx,  // entry point
    cmd_case_ctx,   // switch case label
    cmd_update_ctx, // backward jump
    cmd_save_ctx    // forward jump
  } cmd;

  local_context* next;

  virtual vbm_operand* transfer(method_desc* method, vbm_operand* sp, 
                                byte cop, byte& prev_cop) = 0;

  local_context(context_cmd ctx_cmd, local_context** chain) { 
    cmd = ctx_cmd;
    while (*chain != NULL && (*chain)->cmd < ctx_cmd) { 
      chain = &(*chain)->next;
    }
    next = *chain;
    *chain = this;
  }
};

class ctx_entry_point : public local_context { 
public:
  ctx_entry_point(local_context** chain)
    : local_context(cmd_enter_ctx, chain) {}
    
  virtual vbm_operand* transfer(method_desc* method, 
                                vbm_operand* sp, byte cop, byte& prev_cop);
};

class ctx_split : public local_context { 
public: 
  var_desc*    vars; 
  vbm_operand* stack_pointer;
  vbm_operand  stack_top[2];
  int          switch_var_index;
  int          n_branches;
  int          in_monitor;
    
  enum jmp_type { jmp_forward, jmp_backward };

  ctx_split(local_context** chain, jmp_type type = jmp_forward) 
    : local_context(type == jmp_forward ? cmd_save_ctx : cmd_update_ctx, chain)
  {
    n_branches = 1;
  }
    
  virtual vbm_operand* transfer(method_desc* method, vbm_operand* sp, 
                                byte cop, byte& prev_cop);
};

class ctx_merge : public local_context { 
public:
  ctx_split* come_from;
  int        case_value;
    
  ctx_merge(local_context** chain, ctx_split* come_from_ctx) 
    : local_context(cmd_merge_ctx, chain) 
  { 
    come_from = come_from_ctx;
  }

  ctx_merge(local_context** chain, ctx_split* come_from_ctx, int value) 
    : local_context(cmd_case_ctx, chain) 
  {
    come_from = come_from_ctx;
    case_value = value;
  }
  
  virtual vbm_operand* transfer(method_desc* method, vbm_operand* sp, 
                                byte cop, byte& prev_cop);
};

class ctx_pop_var : public local_context { 
public:
  int var_index;

  ctx_pop_var(local_context** chain, int index) 
    : local_context(cmd_pop_var, chain), var_index(index) {}

  virtual vbm_operand* transfer(method_desc* method, vbm_operand* sp, 
                                byte cop, byte& prev_cop);
};

class ctx_push_var : public local_context { 
public:
  utf_string* var_name;
  int         var_type;
  int         var_index;
  int         var_start_pc;
    
  ctx_push_var(local_context** chain,
               utf_string* name, int type, int index, int start_pc)
    : local_context(cmd_push_var, chain) {
    var_name = name; 
    var_type = type;
    var_index = index;
    var_start_pc = start_pc;
  }
    

  virtual vbm_operand* transfer(method_desc* method, vbm_operand* sp, 
                                byte cop, byte& prev_cop);
};

class ctx_reset : public local_context { 
public:
  int* var_store_count;
    
  ctx_reset(local_context** chain, int* counts, int n_vars) 
    : local_context(cmd_reset_ctx, chain) 
  { 
    var_store_count = new int[n_vars];
    memcpy(var_store_count, counts, n_vars*sizeof(int));
  }
    
  virtual vbm_operand* transfer(method_desc* method, vbm_operand* sp, 
                                byte cop, byte& prev_cop);
};

#endif
