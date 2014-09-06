#ifdef VISUAL_CPP
#define snprintf _snprintf
#endif

#include "method_desc.hh"

void print_call_sequence(callee_desc* callee, int loop_id, int path_id)
{
  if (callee != NULL) { 
    print_call_sequence((callee_desc*)callee->backtrace, loop_id, path_id);
    callee->message(msg_loop, (void*)loop_id, (void*)path_id, 
                    callee->method);
  }
}

int method_desc::demangle_method_name(char* buf)
{
  char* dst = buf;
  const char* src = desc.as_asciz();
  assert(*src++ == '(');
  dst += sprintf(dst, "%s.%s(", cls->name.as_asciz(), name.as_asciz());
  int first_parameter = true;
  while (*src != ')') { 
    if (!first_parameter) { 
      *dst++ = ',';
      *dst++ = ' ';
    }
    first_parameter = false;
    int indirect = 0;
    while (*src == '[') { 
      indirect += 1;
      src += 1;
    }
    switch (*src++) { 
    case 'I': 
      dst += sprintf(dst, "int");
      break;
    case 'S':
      dst += sprintf(dst, "short");
      break;
    case 'D': 
      dst += sprintf(dst, "double");
      break;
    case 'J': 
      dst += sprintf(dst, "long");
      break;
    case 'F': 
      dst += sprintf(dst, "float");
      break;
    case 'B': 
      dst += sprintf(dst, "byte");
      break;
    case 'C': 
      dst += sprintf(dst, "char");
      break;
    case 'Z': 
      dst += sprintf(dst, "boolean");
      break;
    case 'L':
      while (*src != ';') { 
        if (*src == '/') *dst++ = '.';
        else *dst++ = *src;
        src += 1;
      }
      src += 1;
    }
    while (indirect != 0) { 
      *dst++ = '[';
      *dst++ = ']';
      indirect -= 1;
    }
  }
  *dst++ = ')';
  *dst = 0;
  return dst - buf;
}

void method_desc::check_invocations()
{
  for (int i = 0; i < 32; i++) { 
    if (null_parameter_mask & unchecked_use_mask & (1 << i)) {
      message_at(msg_null_param, cls->source_file, first_line, this, i);
    }
  }
}

bool method_desc::build_call_graph(method_desc* caller, callee_desc* callee, 
                                   int caller_attr)
{
  callee->backtrace = caller;
  for (overridden_method* ovr = overridden; ovr != NULL; ovr = ovr->next) { 
    ovr->method->build_call_graph(caller, callee, caller_attr);
  }
  if ((attr & m_synchronized) || (attr & m_sync_block)) { 
    if (!(caller_attr & callee_desc::i_self)) {
#ifdef DUMP_EDGES
      char buf[2][MAX_MSG_LENGTH];
      caller->demangle_method_name(buf[0]);
      demangle_method_name(buf[1]);
      printf("Call graph edge %s -> %s\n", buf[0], buf[1]);
#endif
      graph_edge* edge = new graph_edge(vertex, caller, callee);
      assert(caller->vertex != NULL);
      caller->vertex->attach(edge);
    }
    return true;
  } else if (!(attr & m_deadlock_free)) { 
    bool can_cause_deadlock = false;
    for (callee = callees; callee != NULL; callee = callee->next) {
      if (callee->backtrace != caller) { 
        can_cause_deadlock |= 
          callee->method->build_call_graph(caller, callee, 
                                           caller_attr&callee->attr);
      } 
    }
    if (!can_cause_deadlock) { 
      attr |= m_deadlock_free;
    }
    return can_cause_deadlock;
  }
  return false;
}

void method_desc::check_synchronization()
{
  if (attr & m_concurrent) {
    for (callee_desc* callee = callees; callee != NULL; 
         callee = callee->next)
      {
        if (!(callee->method->attr & (m_serialized|m_synchronized)) &&
            !(callee->method->cls->attr & class_desc::cl_system) &&
            !(callee->attr & (callee_desc::i_self
                              |callee_desc::i_synchronized)) &&
            (strstr(callee->method->name.as_asciz(), "init>") == NULL) )
          {
            callee->message(msg_concurrent_call, callee->method);
          }
      }

    for (access_desc* accessor = accessors; accessor != NULL; 
         accessor = accessor->next)
      {
        if (!(accessor->field->attr 
              & (field_desc::f_volatile|field_desc::f_final
                 |field_desc::f_serialized)) 
            && !(accessor->field->cls->attr & class_desc::cl_system) 
            && !(accessor->attr&(access_desc::a_new|access_desc::a_self)))
          {
            accessor->message(msg_concurrent_access, 
                              &accessor->field->name,accessor->field->cls);
          }
      }
  }
}

void method_desc::find_access_dependencies()
{
  for (callee_desc* callee = callees; callee != NULL; callee = callee->next){
    class_desc* caller_class = callee->method->accessor;
    if (caller_class == NULL) { 
      callee->method->accessor = cls;
    } else if (!cls->in_relationship_with(caller_class)) { 
      // Method is called from two unrelated classes, so if this
      // two methods are exeuted concurretly we will have access conflict
      callee->method->attr &= ~m_serialized;
    }
    if ((!(attr & m_static) && (callee->method->attr & m_static))
        || !(attr & m_concurrent))
      {
        //
        // If caller is instance method and callee - static method 
        // of the class, or caller is not included in concurrent closure
        // (and so it can be exeuted in main thread of control), then
        // any invocation of callee method from concurrent thread can
        // cause concurrent execution of this method. If no method
        // from concurrent closure invoke this method, then 
        // "m_serialized" attribute will not checked at all, otherwise
        // message about concurrent invocation of non-synchronized method
        // will be reported. 
        //
        callee->method->attr &= ~m_serialized;
      }
  }

  for (access_desc* accessor = accessors; accessor != NULL; 
       accessor = accessor->next)
    {
      class_desc* accessor_class = accessor->field->accessor;
      if (accessor_class == NULL) { 
        accessor->field->accessor = cls;
      } else if (!cls->in_relationship_with(accessor_class)) { 
        accessor->field->attr &= ~field_desc::f_serialized;
      }
      if ((attr & m_static) && (accessor->field->attr & field_desc::f_static)
          && cls->isa(accessor->field->cls))
        { 
          accessor->attr |= access_desc::a_self;
        }
      if ((!(attr & m_static) 
           && (accessor->field->attr & field_desc::f_static))
          || !(attr & m_concurrent))
        {
          accessor->field->attr &= ~field_desc::f_serialized;
        }
    }
}

void method_desc::build_call_graph()
{
  if (attr & m_synchronized) { 
    for (callee_desc* callee=callees; callee != NULL; callee=callee->next){
      callee->method->build_call_graph(this, callee, callee->attr);
    }
  }
}

int method_desc::print_call_path_to(callee_desc* target, 
                                    int loop_id, int path_id, int caller_attr, 
                                    callee_desc* prev)
{
  if (attr & (m_deadlock_free|m_visited)) { 
    return path_id;
  }
  attr |= m_visited;
  if (prev != NULL) { 
    for (overridden_method* ovr = overridden; 
         ovr != NULL && path_id < max_shown_paths; 
         ovr = ovr->next) 
      { 
        path_id = ovr->method->print_call_path_to(target, loop_id, path_id,
                                                  caller_attr, prev);
      }
  }
  for (callee_desc* callee = callees; 
       callee != NULL && path_id < max_shown_paths; 
       callee = callee->next)
    {
      int callee_attr = callee->attr & caller_attr;
      if (callee->method->attr & m_synchronized) { 
        if (callee == target && !(callee_attr & callee_desc::i_self)) { 
          print_call_sequence(prev, loop_id, ++path_id);
        }
      } else { 
        callee->backtrace = prev;
        path_id = callee->method->print_call_path_to(target, loop_id, 
                                                     path_id, callee_attr, 
                                                     callee);
      }
    }
  attr &= ~m_visited;
  return path_id;
}


void method_desc::add_to_concurrent_closure(callee_desc* caller, 
                                            int caller_attr, int depth)
{
  for (overridden_method* ovr=overridden; ovr != NULL; ovr=ovr->next) { 
    add_to_concurrent_closure(caller, caller_attr, depth);
  }
  if (attr & m_synchronized) { 
    if ((caller_attr & (callee_desc::i_synchronized|callee_desc::i_self)) 
        == callee_desc::i_synchronized && (attr & m_wait)) 
      {
        int callee_attr = callee_desc::i_self|callee_desc::i_wait_deadlock;
        callee_desc* up = caller; 
        while (true) { 
          callee_attr &= up->attr;
          if (up->backtrace == NULL
              || (!(callee_attr & callee_desc::i_self) &&
                  ((up->attr & callee_desc::i_synchronized)
                   || (((callee_desc*)up->backtrace)->method->attr 
                       & m_synchronized))))
            {
              break;
            }
          up = (callee_desc*)up->backtrace;
        }
        // Check if this pass was already found
        if (!(callee_attr & callee_desc::i_wait_deadlock)) { 
          message_at(msg_wait, cls->source_file, wait_line, this);   
          callee_desc* bt = caller; 
          while (true) {
            bt->message(msg_wait_path, bt->method);
            bt->attr |= callee_desc::i_wait_deadlock;
            if (bt == up) break;
            bt = (callee_desc*)bt->backtrace;
          }
        }
      }
    attr |= m_concurrent;
    if ((caller_attr & (callee_desc::i_synchronized|callee_desc::i_self))
        == callee_desc::i_synchronized && !(attr & m_visited)) 
      { 
        attr |= m_visited;
        for (callee_desc* callee = callees; 
             callee != NULL; 
             callee = callee->next)
          {
            if (callee->attr & callee_desc::i_self) {
              callee->backtrace = caller;
              callee->method->add_to_concurrent_closure
                (callee, callee_desc::i_synchronized, 0);
            }
          }
        attr &= ~m_visited;
      }
  } 
  else if (!(attr & m_visited) 
           && !((depth != 0 || (caller_attr & callee_desc::i_self)) 
                && (attr & m_concurrent))) 
    { 
      if (attr & m_concurrent) { 
        depth += 1;
      }
      attr |= m_visited|m_concurrent;
      for (callee_desc* callee=callees; callee != NULL; callee=callee->next){
        int callee_attr = caller_attr; 
        if (callee->attr & callee_desc::i_synchronized) { 
          callee_attr = callee_desc::i_synchronized;
        } else { 
          callee_attr &= callee->attr|~callee_desc::i_self;
        }
        callee->backtrace = caller;
        callee->method->add_to_concurrent_closure(callee, callee_attr, 
                                                  depth);
      }
      attr &= ~m_visited;
    }
}

void method_desc::build_concurrent_closure()
{
  //
  // Check if "run" method of class implementing Runnable interface
  // is synchronized and mark this method as concurrent.
  //
  int caller_attr = 0;
  attr |= m_visited;
  if (attr & m_synchronized) { 
    caller_attr = callee_desc::i_synchronized;
  } 
  else if (name == "run" && (cls->implements("java/lang/Runnable")
                             || cls->isa("java/lang/Thread")))
    {
      if (cls->implements("java/lang/Runnable")) {
        message_at(msg_run_nosync, cls->source_file, first_line, this);
      }
    } else { 
      for (callee_desc* callee=callees; callee != NULL; callee=callee->next){
        if (callee->attr & callee_desc::i_synchronized) { 
          callee->backtrace = NULL;
          callee->method->
            add_to_concurrent_closure(callee, 
                                      callee_desc::i_synchronized, 0);
        }
      }
      attr &= ~m_visited;
      return;
    } 
  for (callee_desc* callee=callees; callee != NULL; callee=callee->next) {
    int callee_attr = callee->attr;
    if (callee_attr & callee_desc::i_synchronized) { 
      // Synchronized construction was used. Clear "i_self" bit,
      // because two monitors are already locked
      callee_attr &= ~callee_desc::i_self; 
    }
    callee->backtrace = NULL;
    callee->method->add_to_concurrent_closure(callee, 
                                              callee_attr|caller_attr, 0);
  }
  attr &= ~m_visited;
}

void method_desc::calculate_attributes()
{
  vertex = (attr & m_static) ? cls->metaclass_vertex : cls->class_vertex; 
  //
  // Find out all "self" static invocations
  //
  for (callee_desc* callee = callees; callee != NULL; callee=callee->next) {
    if ((attr & callee->method->attr & m_static)
        && cls->isa(callee->method->cls)) 
      { 
        callee->attr |= callee_desc::i_self;
      }
  }
}

int method_desc::get_line_number(int pc)
{
  while (line_table[pc] == 0 && pc > 0) { 
    pc -= 1;
  }
  return line_table[pc];
}

void method_desc::message(int code, int pc, ...)
{
  va_list ap;
  va_start(ap, pc);
#ifdef PRINT_PC
  printf("In %s.%s pc=%d\n", cls->name.as_asciz(), name.as_asciz(), pc);
#endif
  format_message(code, cls->source_file, get_line_number(pc), ap);
  va_end(ap);
}

void method_desc::check_variable_for_null(int pc, vbm_operand* sp)
{
  if (!(sp->mask & var_desc::vs_not_null)) { 
    if (sp->index >= 0) { 
      message(msg_null_var, pc, &vars[sp->index].name);
    } else { 
      message(msg_null_ptr, pc);
    } 
  } 
  else if ((unsigned)sp->index < 32 
           && (sp->mask & var_desc::vs_not_null) != var_desc::vs_not_null)
    {
      // formal parameter was not assigned value 
      // and was not checked for NULL
      if (null_parameter_mask & (1 << sp->index)) { 
        message(msg_null_param, pc, this, sp->index);
      } else { 
        unchecked_use_mask |= 1 << sp->index;
      }
    } 
}

void method_desc::check_array_index(int pc, vbm_operand* sp)
{
  check_variable_for_null(pc, sp);
  if (sp[1].min < 0) { 
    if (sp[1].max < 0) { 
      message(msg_bad_index, pc, sp[1].min, sp[1].max);
    } else if (sp[1].min > -128) { 
      message(msg_maybe_bad_index, pc, sp[1].min, sp[1].max);
    } 
  }
  if (sp[1].max >= sp->max && sp->max != MAX_ARRAY_LENGTH) {  
    if (sp[1].min >= sp->max) { 
      message(msg_bad_index, pc, sp[1].min, sp[1].max);
    } else if (sp[1].max - sp->max < 127) { 
      message(msg_maybe_bad_index, pc, sp[1].min, sp[1].max);
    }
  }
}

void method_desc::basic_blocks_analysis()
{
  byte* pc = code;
  byte* end = pc + code_length;
  int   i;
  var_store_count = n_vars > 0 ? new int[n_vars] : NULL;

  for (i = n_vars; --i >= 0;) { 
    vars[i].type = tp_void;
    vars[i].name = utf_string("???");
    vars[i].min  = 0;
    vars[i].max  = MAX_ARRAY_LENGTH;
    var_store_count[i] = 0;
  }

  while (pc != end) { 
    int addr = pc - code;
    int offs;
    switch (*pc) { 
    case jsr:
    case ifeq:
    case ifne:
    case iflt:
    case ifge:
    case ifgt:
    case ifle:
    case if_icmpeq:
    case if_icmpne:
    case if_icmplt:
    case if_icmpge:
    case if_icmpgt:
    case if_icmple:
    case if_acmpeq:
    case if_acmpne:
    case ifnull:
    case ifnonnull:
    case goto_near:
      offs = (short)unpack2(pc+1);
      if (offs < 0) { // backward jump 
        new ctx_reset(&context[addr+offs], var_store_count, n_vars);
        new ctx_split(&context[addr], ctx_split::jmp_backward);
      } else if (offs > 0) { // forward jump
        new ctx_merge(&context[addr+offs],
                      new ctx_split(&context[addr], ctx_split::jmp_forward));
      }
      pc += 3;
      break;
    case jsr_w:
    case goto_w:
      offs = unpack4(pc+1);
      if (offs < 0) {  // backward jump 
        new ctx_reset(&context[addr+offs], var_store_count, n_vars);
      } else if (offs > 0) { // forward jump
        new ctx_merge(&context[addr+offs],
		      new ctx_split(&context[addr], ctx_split::jmp_forward));
      }
      pc += 5;
      break;
    case tableswitch:
      {
        pc += 4 - (addr & 3);
        offs = unpack4(pc); // default label
        int low = unpack4(pc+4);
        int high = unpack4(pc+8);
        int n_forward_jumps = 0;
        pc += 12;
        ctx_split* select = new ctx_split(&context[addr]);
        if (offs < 0) { 
          new ctx_reset(&context[addr+offs], var_store_count, n_vars);
        } else if (offs > 0) { 
          new ctx_merge(&context[addr+offs], select);
          n_forward_jumps += 1;
        }
        while (low <= high) { 
          offs = unpack4(pc);
          if (offs < 0) { 
            new ctx_reset(&context[addr+offs], 
                          var_store_count, n_vars);
          } else if (offs > 0) { 
            new ctx_merge(&context[addr+offs], select, low);
            n_forward_jumps += 1;
          }
          pc += 4;
          low += 1;
        }
        select->n_branches = n_forward_jumps;
      }
      break;
    case lookupswitch:
      {
        pc += 4 - (addr & 3);
        offs = unpack4(pc); // default label
        int n_pairs = unpack4(pc+4);
        int n_forward_jumps = 0;
        pc += 8;
        ctx_split* select = new ctx_split(&context[addr]);
        if (offs < 0) { 
          new ctx_reset(&context[addr+offs], var_store_count, n_vars);
        } else if (offs > 0) { 
          new ctx_merge(&context[addr+offs], select);
          n_forward_jumps += 1;
        }
        while (--n_pairs >= 0) {
          offs = unpack4(pc+4);
          if (offs < 0) { 
            new ctx_reset(&context[addr+offs], 
                          var_store_count, n_vars);
          } else if (offs > 0) { 
            new ctx_merge(&context[addr+offs], select, unpack4(pc));
            n_forward_jumps += 1;
          }
          pc += 8;
        }
        select->n_branches = n_forward_jumps;
      }
      break;
    case istore:
    case lstore:
    case astore:
      pc += 1;
      var_store_count[*pc++] += 1;
      break;
    case istore_0:
    case istore_1:
    case istore_2:
    case istore_3:
      var_store_count[*pc++ - istore_0] += 1;
      break;
    case lstore_0:
    case lstore_1:
    case lstore_2:
    case lstore_3:
      var_store_count[*pc++ - lstore_0] += 1;
      break;
    case astore_0:
    case astore_1:
    case astore_2:
    case astore_3:
      var_store_count[*pc++ - astore_0] += 1;
      break;
    case iinc:
      var_store_count[pc[1]] += 1;
      pc += 3;
      break;
    case wide:
      pc += 1;
      switch (*pc++) { 
      case istore:
      case lstore:
      case astore:
        var_store_count[unpack2(pc)] += 1;
        break;      
      case iinc:
        var_store_count[unpack2(pc)] += 1;
        pc += 2;
      }
      pc += 2;
      break;
    default:
      pc += vbm_instruction_length[*pc];
    }
  }
  for (i = n_vars; --i >= 0;) { 
    var_store_count[i] = 0;
  }
}

void method_desc::parse_code(constant** constant_pool,
                             const field_desc* is_this)
{
#ifdef DEBUG
  printf("Method %s\n", name.as_asciz());
#endif
  const int indirect = 0x100;
  byte* pc = code;
  byte* end = pc + code_length;
  const int max_stack_size = 256;
  vbm_operand stack[max_stack_size+2];
  int i;
  for (i = 0; i < max_stack_size+2; i++) {
    stack[i].equals = NULL;
  }
  vbm_operand* stack_bottom = stack+2; // avoid checks for non-empty stack
  vbm_operand* sp = stack_bottom;
  local_context* ctx;
  byte prev_cop = nop;
  bool super_finalize = false;
  
#ifdef INT8_DEFINED
  int8 left_min;
  int8 left_max;
  int8 left_mask;
  int8 right_min;
  int8 right_max;
  int8 right_mask;
#define LOAD_INT8_OPERANDS() \
    left_min = LOAD_INT8(sp-4, min);\
    left_max = LOAD_INT8(sp-4, max);\
    left_mask = LOAD_INT8(sp-4, mask);\
    right_min = LOAD_INT8(sp-2, min);\
    right_max = LOAD_INT8(sp-2, max);\
    right_mask = LOAD_INT8(sp-2, mask)
#define LOAD_INT8_OPERAND() \
    left_min = LOAD_INT8(sp-2, min);\
    left_max = LOAD_INT8(sp-2, max);\
    left_mask = LOAD_INT8(sp-2, mask)    
#endif

  in_monitor = false;
  stack[0].type = stack[1].type = tp_void;
  cls->locks.clear();
  basic_blocks_analysis();

  for (i = 0; i < 32; i++) { 
    if (null_parameter_mask & (1 << i)) { 
      assert(i < n_vars);
      vars[i].mask = 0;
      vars[i].min = 0;
      vars[i].max = MAX_ARRAY_LENGTH;
      if (vars[i].type == tp_void) { 
        vars[i].type = tp_object;
      }
    }
  }
  
  if (!(attr & m_static)) { 
    vars[0].type = tp_self;
    vars[0].mask = var_desc::vs_not_null;
    vars[0].min = 0;
    vars[0].max = MAX_ARRAY_LENGTH;
    vars[0].equals = is_this;
  }
  if (attr & m_synchronized) { // add "this" to lock set if needed
    cls->locks.acquire(is_this);
    locksAtEntry.acquire(is_this);
  }

  while (pc < end) { 
    int addr = pc - code;
    byte cop = *pc++; 
#ifdef DUMP_BYTE_CODES
    printf("%s: min=%d, max=%d, mask=0x%x\n", 
           vbm_instruction_mnemonic[cop], 
           sp[-1].min, sp[-1].max, sp[-1].mask);
#endif
    for (ctx = context[addr]; ctx != NULL; ctx = ctx->next) {
      sp = ctx->transfer(this, sp, cop, prev_cop);
    }
    switch (cop) {
    case nop:
      break;
    case aconst_null:
      sp->type = tp_object;
      sp->mask = 0;
      sp->min = sp->max = 0;
      sp->index = NO_ASSOC_VAR;
      sp += 1;
      break;
    case iconst_m1:
    case iconst_0:
    case iconst_1:
    case iconst_2:
    case iconst_3:
    case iconst_4:
    case iconst_5:
      sp->type = tp_byte;
      sp->mask = sp->min = sp->max = cop - iconst_0;
      sp->index = NO_ASSOC_VAR;
      sp += 1;
      break;
    case fconst_0:
    case fconst_1:
    case fconst_2:
      sp->type = tp_float;
      sp->index = NO_ASSOC_VAR;
      sp += 1;
      break;
    case lconst_0:
    case lconst_1:
      sp[0].type = sp[1].type = tp_long; 
      sp[0].min = sp[0].max = sp[0].mask = 0;
      sp[1].min = sp[1].max = sp[1].mask = cop - lconst_0;
      sp[0].index = sp[1].index = NO_ASSOC_VAR;
      sp += 2;
      break;
    case dconst_0:
    case dconst_1:
      sp[0].type = sp[1].type = tp_double; 
      sp[0].index = sp[1].index = NO_ASSOC_VAR;
      sp += 2;
      break;
    case bipush:
      sp->type = tp_int;
      sp->mask = sp->min = sp->max = (signed char)*pc++;
      sp->index = NO_ASSOC_VAR;
      sp += 1;
      break;
    case sipush:
      sp->type = tp_int;
      sp->mask = sp->min = sp->max = (short)unpack2(pc);
      sp->index = NO_ASSOC_VAR;
      pc += 2;
      sp += 1;
      break;
    case ldc:
      {
        constant* cp = constant_pool[*pc++];
        sp->type = cp->type();
        if (sp->type == tp_int) {
          sp->mask = sp->min = sp->max = ((const_int*)cp)->value;
        } else { 
          if (sp->type == tp_string) { 
            sp->min = sp->max = ((const_string*)cp)->length();
          } else { 
            sp->min = 0;
            sp->max = MAX_ARRAY_LENGTH;
          }
          sp->mask = var_desc::vs_not_null;
        }
        sp->index = NO_ASSOC_VAR;
        sp->equals = is_const;
        sp += 1;
      }
      break;
    case ldc_w:
      {
        constant* cp = constant_pool[unpack2(pc)];
        sp->type = cp->type();
        if (sp->type == tp_int) {
          sp->mask = sp->min = sp->max = ((const_int*)cp)->value;
        } else { 
          if (sp->type == tp_string) { 
            sp->min = sp->max = ((const_string*)cp)->length();
          } else { 
            sp->min = 0;
            sp->max = MAX_ARRAY_LENGTH;
          }
          sp->mask = var_desc::vs_not_null;
        }
        sp->index = NO_ASSOC_VAR;
        pc += 2;
        sp += 1;
      }
      break;
    case ldc2_w:
      { 
        const_long* cp = (const_long*)constant_pool[unpack2(pc)];
        sp->type = tp_long; 
        sp->index = NO_ASSOC_VAR;
        sp->mask = sp->min = sp->max = cp->value.high;
        sp += 1;
        sp->type = tp_long; 
        sp->index = NO_ASSOC_VAR;
        sp->mask = sp->min = sp->max = cp->value.low;
        sp += 1; 
        pc += 2;
      }
      break;
    case iload:
      if (vars[*pc].type == tp_void) { 
        vars[*pc].type = tp_int;
        vars[*pc].min = ranges[tp_int].min;
        vars[*pc].max = ranges[tp_int].max;
        vars[*pc].mask = ALL_BITS;
      }
      sp->type = vars[*pc].type;
      sp->min  = vars[*pc].min;
      sp->max  = vars[*pc].max;
      sp->mask = vars[*pc].mask;
      sp->index = *pc++;
      sp += 1;
      break;
    case aload:
      if (vars[*pc].type == tp_void) { 
        vars[*pc].type = tp_object;
        vars[*pc].min = 0;
        vars[*pc].max = MAX_ARRAY_LENGTH;
        vars[*pc].mask = var_desc::vs_unknown;
      }
      sp->type = vars[*pc].type;
      sp->mask = vars[*pc].mask;
      sp->min  = vars[*pc].min;
      sp->max  = vars[*pc].max;
      sp->equals = vars[*pc].equals;
#ifdef DUMP_STACK
        printf("<stack_top> := %x %s\n",
               (int)sp->equals,
               sp->equals == NULL ? 
               "<unknown>" : sp->equals->name.as_asciz()
               );
#endif
      sp->index = *pc++;
      sp += 1;
      break;
    case lload:
      { 
        int index = *pc++; 
        if (vars[index].type == tp_void) { 
          vars[index].type = tp_long;
          vars[index].min = 0x80000000;
          vars[index].max = 0x7fffffff;
          vars[index].mask = 0xffffffff;
          vars[index+1].min = 0x00000000;
          vars[index+1].max = 0xffffffff;
          vars[index+1].mask = 0xffffffff;
        }
        sp->type = tp_long;
        sp->min  = vars[index].min;
        sp->max  = vars[index].max;
        sp->mask = vars[index].mask;
        sp->index = index;
        sp += 1;
        index += 1;
        sp->type = tp_long;
        sp->min  = vars[index].min;
        sp->max  = vars[index].max;
        sp->mask = vars[index].mask;
        sp += 1;
      }
      break;
    case dload:
      sp[0].type = sp[1].type = tp_double; 
      sp[0].index = sp[1].index = NO_ASSOC_VAR;
      sp += 2; pc += 1;
      break;
    case fload:
      sp->type = tp_float; 
      sp->index = NO_ASSOC_VAR;
      sp += 1;
      pc += 1;
      break;
    case iload_0:
    case iload_1:
    case iload_2:
    case iload_3:
      { 
        var_desc* var = &vars[cop - iload_0];
        if (var->type == tp_void) { 
          var->type = tp_int;
          var->min = ranges[tp_int].min;
          var->max = ranges[tp_int].max;
          var->mask = ALL_BITS;
        }
        sp->type = var->type;
        sp->min  = var->min;
        sp->max  = var->max;
        sp->mask = var->mask;
        sp->index = cop - iload_0;
        sp += 1;
      }
      break;
    case lload_0:
    case lload_1:
    case lload_2:
    case lload_3:
      { 
        int index = cop - lload_0; 
        if (vars[index].type == tp_void) { 
          vars[index].type = tp_long;
          vars[index].min = 0x80000000;
          vars[index].max = 0x7fffffff;
          vars[index].mask = 0xffffffff;
          vars[index+1].min = 0x00000000;
          vars[index+1].max = 0xffffffff;
          vars[index+1].mask = 0xffffffff;
        }
        sp->type = tp_long;
        sp->min  = vars[index].min;
        sp->max  = vars[index].max;
        sp->mask = vars[index].mask;
        sp->index = index;
        sp += 1;
        index += 1;
        sp->type = tp_long;
        sp->min  = vars[index].min;
        sp->max  = vars[index].max;
        sp->mask = vars[index].mask;
        sp += 1;
      }
      break;
    case dload_0:
    case dload_1:
    case dload_2:
    case dload_3:
      sp[0].type = sp[1].type = tp_double; 
      sp[0].index = sp[1].index = NO_ASSOC_VAR;
      sp += 2;
      break;
    case fload_0:
    case fload_1:
    case fload_2:
    case fload_3:
      sp->type = tp_float;
      sp->index = NO_ASSOC_VAR;
      sp += 1;
      break;
    case aload_0:
    case aload_1:
    case aload_2:
    case aload_3:
      { 
        var_desc* var = &vars[cop - aload_0];
        if (var->type == tp_void) { 
          if (cop == aload_0 && !(attr & m_static)) { 
            var->type = tp_self;
            var->mask = var_desc::vs_not_null;
          } else { 
            var->type = tp_object;
            var->mask = var_desc::vs_unknown;
          }
          sp->min  = 0;
          sp->max  = MAX_ARRAY_LENGTH;
        }
        sp->type = var->type;
        sp->mask = var->mask;
        sp->min  = var->min;
        sp->max  = var->max;
        sp->index = cop - aload_0;
        sp->equals = vars[cop - aload_0].equals;
#ifdef DUMP_STACK
        printf("<stack_top> := %x %s\n",
               (int)sp->equals,
               sp->equals == NULL ? 
               "<unknown>" : sp->equals->name.as_asciz()
               );
#endif
        sp += 1;
      }
      break;
    case iaload:
    case faload:
    case aaload:
    case baload:
    case caload:
    case saload:
      sp -= 2; 
      check_array_index(addr, sp);
      if (IS_ARRAY_TYPE(sp->type)) { 
        sp->type -= indirect;
      } else { 
        sp->type = 
          (cop == aaload) ? tp_object : 
          (cop == baload) ? tp_byte : 
          (cop == caload) ? tp_char : 
          (cop == iaload) ? tp_int  : 
          (cop == saload) ? tp_short : tp_float;
      }
      if (IS_INT_TYPE(sp->type)) { 
        sp->min = ranges[sp->type].min;
        sp->max = ranges[sp->type].max;      
        sp->mask = sp->min|sp->max;
      } else { 
        sp->min  = 0;
        sp->max  = MAX_ARRAY_LENGTH;
        sp->mask = var_desc::vs_unknown;
      }
      sp->index = NO_ASSOC_VAR;
      sp += 1;
      break;
    case laload:
      check_array_index(addr, sp-2);
      sp[-1].type = sp[-2].type = tp_long;
      sp[-1].index = sp[-2].index = NO_ASSOC_VAR;
      sp[-2].min  = 0x80000000;
      sp[-2].max  = 0x7fffffff;
      sp[-2].mask = 0xffffffff;
      sp[-1].min  = 0x00000000;
      sp[-1].max  = 0xffffffff;
      sp[-1].mask = 0xffffffff;
      break; 
    case daload:
      check_array_index(addr, sp-2);
      sp[-1].type = sp[-2].type = tp_double;
      sp[-1].index = sp[-2].index = NO_ASSOC_VAR;
      break; 
    case istore:
      {
        var_desc* var = &vars[*pc];
        sp -= 1;
        var->max = sp->max;
        var->min = sp->min;
        var->mask = sp->mask;
        var_store_count[*pc] += 1;
        if (var->type == tp_void) { 
          var->type = tp_int;
        }
        pc += 1;
      }
      break;
    case astore:
      sp -= 1;
      vars[*pc].mask = sp->mask;
      vars[*pc].min = sp->min;
      vars[*pc].max = sp->max;
      var_store_count[*pc] += 1;
      if (vars[*pc].type == tp_void) { 
        vars[*pc].type = tp_object;
      }
      vars[*pc].equals = sp->equals;
#ifdef DUMP_STACK
      for (i = 0; i <= *pc; i++) {
        printf("<var_%d> := %x %s\n",
               i,
               (int)vars[i].equals,
               vars[i].equals == NULL ? 
               "<unknown>" : vars[i].equals->name.as_asciz()
               );
      }
#endif
      pc += 1;
      break;
    case fstore:
      pc += 1; sp -= 1;
      break;
    case lstore:
      {
        int index = *pc++;
        sp -= 2;
        vars[index].type = tp_long;
        vars[index].max  = sp[0].max;
        vars[index].min  = sp[0].min;
        vars[index].mask = sp[0].mask;
        vars[index+1].max  = sp[1].max;
        vars[index+1].min  = sp[1].min;
        vars[index+1].mask = sp[1].mask;
        var_store_count[index] += 1;
      }
      break;
    case dstore:
      pc += 1; sp -= 2;
      break;
    case istore_0:
    case istore_1:
    case istore_2:
    case istore_3:
      { 
        var_desc* var = &vars[cop - istore_0];
        sp -= 1;
        var->max = sp->max;
        var->min = sp->min;
        var->mask = sp->mask;
        if (var->type == tp_void) { 
          var->type = tp_int;
        }
        var_store_count[cop - istore_0] += 1;
      }
      break;

    case astore_0:
    case astore_1:
    case astore_2:
    case astore_3:
      sp -= 1;
      vars[cop - astore_0].mask = sp->mask;
      vars[cop - astore_0].min = sp->min;
      vars[cop - astore_0].max = sp->max;
      if (vars[cop - astore_0].type == tp_void) { 
        vars[cop - astore_0].type = tp_object;
      }
      vars[cop - astore_0].equals = sp->equals;
      var_store_count[cop - astore_0] += 1;
#ifdef DUMP_STACK
      for (i = 0; i <= cop-astore_0; i++) {
        printf("<var_%d> := %x %s\n",
               i,
               (int)vars[i].equals,
               vars[i].equals == NULL ? 
               "<unknown>" : vars[i].equals->name.as_asciz()
               );
      }
#endif
      break;
    case fstore_0:
    case fstore_1:
    case fstore_2:
    case fstore_3:
      sp -= 1;
      break;
    case lstore_0:
    case lstore_1:
    case lstore_2:
    case lstore_3:
      {
        int index = cop - lstore_0;
        sp -= 2;
        vars[index].type = tp_long;
        vars[index].max  = sp[0].max;
        vars[index].min  = sp[0].min;
        vars[index].mask = sp[0].mask;
        vars[index+1].max  = sp[1].max;
        vars[index+1].min  = sp[1].min;
        vars[index+1].mask = sp[1].mask;
        var_store_count[index] += 1;
      }
      break;
    case dstore_0:
    case dstore_1:
    case dstore_2:
    case dstore_3:
      sp -= 2;
      break;
    case iastore:
    case fastore:
    case aastore:
    case bastore:
    case castore:
    case sastore:
      sp -= 3;
      check_array_index(addr, sp);
      break;
    case lastore:
    case dastore:
      sp -= 4;
      check_array_index(addr, sp);
      break;
    case pop:
      sp -= 1;
      break;
    case pop2:
      sp -= 2;
      break;
    case dup_x0:      
      *sp = *(sp-1); sp += 1;
      break;
    case dup_x1:
      sp[0]=sp[-1]; sp[-1]=sp[-2]; sp[-2]=sp[0]; sp++;
      break;
    case dup_x2:
      sp[0]=sp[-1]; sp[-1]=sp[-2]; sp[-2]=sp[-3]; sp[-3]=sp[0]; sp++;
      break;
    case dup2_x0:
      sp[0]=sp[-2]; sp[1] = sp[-1]; sp += 2;
      break;
    case dup2_x1:
      sp[1]=sp[-1]; sp[0]=sp[-2]; sp[-1]=sp[-3]; sp[-2]=sp[1]; 
      sp[-3]=sp[0]; sp += 2;
      break;
    case dup2_x2:
      sp[1]=sp[-1]; sp[0]=sp[-2]; sp[-1]=sp[-3]; sp[-2] = sp[-4]; 
      sp[-3]=sp[1]; sp[-4] = sp[0]; sp += 2;
      break;
    case Jswap:
      { 
        vbm_operand tmp = sp[-1]; 
        sp[-1] = sp[-2]; 
        sp[-2] = tmp;
      }
      break;
    case iadd:
      if ((sp[-1].min == 0 && sp[-1].max == 0) ||
          (sp[-2].min == 0 && sp[-2].max == 0))
        {
          message(msg_zero_operand, addr, "+");
        }
      if (((sp[-1].max ^ sp[-2].max) < 0 
           || ((sp[-1].max + sp[-2].max) ^ sp[-1].max) >= 0)
          && ((sp[-1].min ^ sp[-2].min) < 0 
              || ((sp[-1].min + sp[-2].min) ^ sp[-1].min) >= 0))
        {
          sp[-2].max += sp[-1].max;
          sp[-2].min += sp[-1].min;
        } else { 
          sp[-2].max = ranges[tp_int].max;
          sp[-2].min = ranges[tp_int].min;
        }
      sp[-2].mask = make_mask(minimum(first_set_bit(sp[-2].mask), 
                                      first_set_bit(sp[-1].mask)));
      sp[-2].type = tp_int;
      sp[-2].index = NO_ASSOC_VAR;
      sp -= 1;
      break;
    case isub:
      if ((sp[-1].min == 0 && sp[-1].max == 0) ||
          (sp[-2].min == 0 && sp[-2].max == 0))
        {
          message(msg_zero_operand, addr, "-");
        }
      if (((sp[-2].max ^ sp[-1].min) >= 0 
           || ((sp[-2].max - sp[-1].min) ^ sp[-2].max) >= 0)
          && ((sp[-2].min ^ sp[-1].max) >= 0 
              || ((sp[-2].min - sp[-1].max) ^ sp[-2].min) >= 0))
        {
          sp[-2].max -= sp[-1].min;
          sp[-2].min -= sp[-1].max;
        } else { 
          sp[-2].max = ranges[tp_int].max;
          sp[-2].min = ranges[tp_int].min;
        }
      sp[-2].mask = make_mask(minimum(first_set_bit(sp[-2].mask), 
                                      first_set_bit(sp[-1].mask)));
      sp[-2].type = tp_int;
      sp[-2].index = NO_ASSOC_VAR;
      sp -= 1;
      break;
    case imul:
      if ((sp[-1].min == 0 && sp[-1].max == 0) ||
          (sp[-2].min == 0 && sp[-2].max == 0))
        {
          message(msg_zero_operand, addr, "*");
        }
      sp[-2].mask = make_mask(first_set_bit(sp[-2].mask) +  
                              first_set_bit(sp[-1].mask));
      if (sp[-2].mask == 0) { 
        message(msg_zero_result, addr, "*");
        sp[-2].max = sp[-2].min = 0;
      } else { 
        vbm_operand result;
        if (calculate_multiply_range(result, sp[-2], sp[-1])) { 
          assert(result.min <= result.max);
          sp[-2].max = result.max;
          sp[-2].min = result.min;
        } else { // overflow
          sp[-2].max = ranges[tp_int].max;
          sp[-2].min = ranges[tp_int].min;
        }
      }
      sp[-2].type = tp_int;
      sp[-2].index = NO_ASSOC_VAR;
      sp -= 1;
      break;
    case idiv:
      if ((sp[-1].min == 0 && sp[-1].max == 0) ||
          (sp[-2].min == 0 && sp[-2].max == 0))
        {
          message(msg_zero_operand, addr, "/");
        }
      if ((sp[-1].min > 0 
           && (sp[-2].max < sp[-1].min && sp[-2].min > -sp[-1].min))
          || (sp[-1].max < 0
              && (-sp[-2].max > sp[-1].max && sp[-2].min > sp[-1].max)))
        {
          message(msg_zero_result, addr, "/");
          sp[-2].min = sp[-2].max = 0;
        } else if (sp[-1].min > 0) { 
          if (sp[-2].max < 0) { 
            sp[-2].min = sp[-2].min/sp[-1].min;
            sp[-2].max = sp[-2].max/sp[-1].max;
          } else if (sp[-2].min < 0) { 
            sp[-2].min = sp[-2].min/sp[-1].min;
            sp[-2].max = sp[-2].max/sp[-1].min;
          } else { 
            sp[-2].min = sp[-2].min/sp[-1].max;
            sp[-2].max = sp[-2].max/sp[-1].min;
          }
        } else { 
          sp[-2].min = ranges[tp_int].min;
          sp[-2].max = ranges[tp_int].max;
        } 
      sp[-2].mask = make_mask(first_set_bit(sp[-2].mask) -  
                              first_set_bit(sp[-1].mask));
      sp[-2].type = tp_int;
      sp[-2].index = NO_ASSOC_VAR;
      sp -= 1;
      break;
    case irem:
      if ((sp[-1].min == 0 && sp[-1].max == 0) ||
          (sp[-2].min == 0 && sp[-2].max == 0))
        {
          message(msg_zero_operand, addr, "%");
        }
      if ((sp[-1].min > 0 
           && (sp[-2].max < sp[-1].min && sp[-2].min > -sp[-1].min))
          || (sp[-1].max < 0
              && (-sp[-2].max > sp[-1].max && sp[-2].min > sp[-1].max)))
        {
          message(msg_no_effect, addr);
        } else if (sp[-1].min > 0) {    
          sp[-2].min = (sp[-2].min < 0) ? 1-sp[-1].max : 0;
          sp[-2].max = sp[-1].max-1;
        } else { 
          sp[-2].min = ranges[tp_int].min;
          sp[-2].max = ranges[tp_int].max;
        }
      sp[-2].mask = ALL_BITS;
      sp[-2].type = tp_int;
      sp[-2].index = NO_ASSOC_VAR;
      sp -= 1;
      break;
    case ishl:
      if ((sp[-1].min == 0 && sp[-1].max == 0) ||
          (sp[-2].min == 0 && sp[-2].max == 0))
        {
          message(msg_zero_operand, addr, "<<");
        }
      if (sp[-1].max < 0) {
        message(msg_shift_count, addr, "<<", "<=", (void*)sp[-1].max);
      } else if(sp[-1].min >= 32) { 
        message(msg_shift_count, addr, "<<", ">=", (void*)sp[-1].min);
      } 
      else if ((unsigned(sp[-1].max - sp[-1].min) < 256) 
               && (sp[-1].max >= 32 || sp[-1].min < 0))
        { 
          //
          // Produce this message only if range of expression was 
          // restricted in comparence with domains of builtin types
          //
          message(msg_shift_range, addr, "<<",
                  (void*)sp[-1].min, (void*)sp[-1].max);
        }
      sp[-2].mask = make_lshift_mask(sp[-2].mask, 
                                     sp[-1].min, sp[-1].max);
      if (sp[-2].mask == 0) { 
        message(msg_zero_result, addr, "<<");
      }
      if ((unsigned)sp[-1].max < 32 && (unsigned)sp[-1].min < 32) {  
        if (sp[-2].max >= 0) { 
          if (sp[-2].max << sp[-1].max >> sp[-1].max == sp[-2].max){
            sp[-2].max = sp[-2].max << sp[-1].max;
          } else { 
            sp[-2].max = ranges[tp_int].max;
            sp[-2].min = ranges[tp_int].min;
          }
        } else { 
          if (sp[-2].max << sp[-1].min >> sp[-1].min == sp[-2].max){
            sp[-2].max = sp[-2].max << sp[-1].min;
          } else { 
            sp[-2].max = ranges[tp_int].max;
            sp[-2].min = ranges[tp_int].min;
          }
        }
        if (sp[-2].min >= 0) { 
          sp[-2].min <<= sp[-1].min;
        } else { 
          if (sp[-2].min << sp[-1].max >> sp[-1].max == sp[-2].min){
            sp[-2].min = sp[-2].min << sp[-1].max;
          } else { 
            sp[-2].max = ranges[tp_int].max;
            sp[-2].min = ranges[tp_int].min;
          }
        }
      } else {
        sp[-2].max = ranges[tp_int].max;
        sp[-2].min = ranges[tp_int].min;
      }
      sp[-2].type = tp_int;
      sp[-2].index = NO_ASSOC_VAR;
      sp -= 1;
      break;
    case ishr:
      if ((sp[-1].min == 0 && sp[-1].max == 0) ||
          (sp[-2].min == 0 && sp[-2].max == 0))
        {
          message(msg_zero_operand, addr, ">>");
        }
      if (sp[-1].max < 0) {
        message(msg_shift_count, addr, ">>", "<=", (void*)sp[-1].max);
      } else if(sp[-1].min >= 32) { 
        message(msg_shift_count, addr, ">>", ">=", (void*)sp[-1].min);
      } 
      else if ((unsigned(sp[-1].max - sp[-1].min) < 256) 
               && (sp[-1].max >= 32 || sp[-1].min < 0))
        { 
          //
          // Produce this message only if range of expression was 
          // restricted in comparence with domains of builtin types
          //
          message(msg_shift_range, addr, ">>",
                  (void*)sp[-1].min, (void*)sp[-1].max);
        }
      sp[-2].mask = make_rshift_mask(sp[-2].mask, 
                                     sp[-1].min, sp[-1].max);
      if (sp[-2].mask == 0) { 
        message(msg_zero_result, addr, ">>");
      }
      if ((unsigned)sp[-1].max < 32 && (unsigned)sp[-1].min < 32) { 
        sp[-2].max = (sp[-2].max >= 0) 
          ? sp[-2].max >> sp[-1].min : sp[-2].max >> sp[-1].max;
        sp[-2].min = (sp[-2].min >= 0) 
          ? sp[-2].min >> sp[-1].max : sp[-2].min >> sp[-1].min;
      } else { 
        if (sp[-2].max < 0) sp[-2].max = -1;
        if (sp[-2].min > 0) sp[-2].min = 0;
      }
      sp[-2].type = tp_int;
      sp[-2].index = NO_ASSOC_VAR;
      sp -= 1;
      break;
    case iushr:
      if ((sp[-1].min == 0 && sp[-1].max == 0) ||
          (sp[-2].min == 0 && sp[-2].max == 0))
        {
          message(msg_zero_operand, addr, ">>>");
        }
      if (sp[-1].max < 0) {
        message(msg_shift_count, addr, ">>>", "<=", (void*)sp[-1].max);
      } else if(sp[-1].min >= 32) { 
        message(msg_shift_count, addr, ">>>", ">=", (void*)sp[-1].min);
      } 
      else if ((unsigned(sp[-1].max - sp[-1].min) < 256) 
               && (sp[-1].max >= 32 || sp[-1].min < 0))
        { 
          //
          // Produce this message only if range of expression was 
          // restricted in comparence with domains of builtin types
          //
          message(msg_shift_range, addr, ">>>",
                  (void*)sp[-1].min, (void*)sp[-1].max);
        }
      sp[-2].mask = make_rushift_mask(sp[-2].mask, 
                                      sp[-1].min, sp[-1].max);
      if (sp[-2].mask == 0) { 
        message(msg_zero_result, addr, ">>>");
      }
      if ((unsigned)sp[-1].max < 32 && (unsigned)sp[-1].min < 32
          && sp[-2].min >= 0) 
        { 
          sp[-2].max >>= sp[-1].min;
          sp[-2].min >>= sp[-1].max;
        } else { 
          sp[-2].max = ranges[tp_int].max;
          sp[-2].min = ranges[tp_int].min;
        }
      sp[-2].type = tp_int;
      sp[-2].index = NO_ASSOC_VAR;
      sp -= 1;
      break;
    case iand:
      if ((sp[-1].min == 0 && sp[-1].max == 0) ||
          (sp[-2].min == 0 && sp[-2].max == 0))
        {
          message(msg_zero_operand, addr, "&");
        }
      if (*pc == ifeq || *pc == ifne) { 
        if (sp[-1].index >= 0 && 
            sp[-2].min == sp[-2].mask && sp[-2].max == sp[-2].mask) 
          {
            sp[-2].index = sp[-1].index; 
          } 
        else if (sp[-2].index < 0 || 
                 sp[-1].min != sp[-1].mask || sp[-1].max != sp[-1].mask) 
          {
            sp[-2].index = NO_ASSOC_VAR;
          }
      } else { 
        sp[-2].index = NO_ASSOC_VAR;
      }
      if ((sp[-2].mask &= sp[-1].mask) == 0) { 
        message(msg_zero_result, addr, "&");
      }
      sp[-2].min = sp[-2].mask < 0 ? ranges[tp_int].min : 0;
      sp[-2].max = (sp[-1].max & sp[-2].max) >= 0
        ? sp[-2].mask & ranges[tp_int].max : 0;
      sp[-2].type = tp_int;
      sp -= 1;
      break;
    case ior:
      if ((sp[-1].min == 0 && sp[-1].max == 0) ||
          (sp[-2].min == 0 && sp[-2].max == 0))
        {
          message(msg_zero_operand, addr, "|");
        }
      sp[-2].mask |= sp[-1].mask;
      sp[-2].min = minimum(sp[-1].min, sp[-2].min);
      sp[-2].max = sp[-2].mask & ranges[tp_int].max;
      if (sp[-2].min > sp[-2].max) { 
        sp[-2].max = sp[-2].min;
      }
      sp[-2].type = tp_int;
      sp[-2].index = NO_ASSOC_VAR;
      sp -= 1;
      break;
    case ixor:
      if ((sp[-1].min == 0 && sp[-1].max == 0) ||
          (sp[-2].min == 0 && sp[-2].max == 0))
        {
          message(msg_zero_operand, addr, "^");
        }
      sp[-2].mask |= sp[-1].mask;
      sp[-2].min = sp[-2].mask < 0 ? ranges[tp_int].min : 0;
      sp[-2].max = sp[-2].mask & ranges[tp_int].max;
      sp[-2].type = tp_int;
      sp[-2].index = NO_ASSOC_VAR;
      sp -= 1;
      break;
#ifdef INT8_DEFINED
    case ladd:
      LOAD_INT8_OPERANDS();
      if ((right_min == 0 && right_max == 0) ||
          (left_min == 0 && left_max == 0))
        {
          message(msg_zero_operand, addr, "+");
        }
      if (((right_max ^ left_max) < 0 
           || ((right_max + left_max) ^ right_max) >= 0)
          && ((right_min ^ left_min) < 0 
              || ((right_min + left_min) ^ right_min) >= 0))
        {
          left_max += right_max;
          left_min += right_min;
        } else { 
          left_min = INT8_MIN;
          left_max = INT8_MAX;
        }
      left_mask = make_int8_mask(minimum(first_set_bit(right_mask), 
                                         first_set_bit(left_mask)));
      STORE_INT8(sp-4, min, left_min);
      STORE_INT8(sp-4, max, left_max);
      STORE_INT8(sp-4, mask, left_mask);
      sp[-4].index = NO_ASSOC_VAR;
      sp -= 2;
      break;
    case lsub:
      LOAD_INT8_OPERANDS();
      if ((right_min == 0 && right_max == 0) ||
          (left_min == 0 && left_max == 0))
        {
          message(msg_zero_operand, addr, "-");
        }
      if (((left_max ^ right_min) >= 0 
           || ((left_max - right_min) ^ left_max) >= 0)
          && ((left_min ^ right_max) >= 0 
              || ((left_min - right_max) ^ left_min) >= 0))
        {
          left_max -= right_min;
          left_min -= right_max;
        } else { 
          left_max = INT8_MAX;
          left_min = INT8_MIN;
        }
      left_mask = make_int8_mask(minimum(first_set_bit(left_mask), 
                                         first_set_bit(right_mask)));
      STORE_INT8(sp-4, min, left_min);
      STORE_INT8(sp-4, max, left_max);
      STORE_INT8(sp-4, mask, left_mask);
      sp[-4].index = NO_ASSOC_VAR;
      sp -= 2;
      break;
    case lmul:
      LOAD_INT8_OPERANDS();
      if ((right_min == 0 && right_max == 0) ||
          (left_min == 0 && left_max == 0))
        {
          message(msg_zero_operand, addr, "*");
        }
      left_mask = make_int8_mask(first_set_bit(left_mask) +  
                                 first_set_bit(right_mask));
      if (left_mask == 0) { 
        message(msg_zero_result, addr, "*");
        STORE_INT8(sp-4, min, 0);
        STORE_INT8(sp-4, max, 0);
      } else { 
        if (!calculate_int8_multiply_range(sp-4, sp-2)) { 
          STORE_INT8(sp-4, min, INT8_MIN);
          STORE_INT8(sp-4, max, INT8_MAX);
        }
      }
      sp[-4].index = NO_ASSOC_VAR;
      sp -= 2;
      break;
    case lidiv:
      LOAD_INT8_OPERANDS();
      if ((right_min == 0 && right_max == 0) ||
          (left_min == 0 && left_max == 0))
        {
          message(msg_zero_operand, addr, "/");
        }
      if ((right_min > 0 
           && (left_max < right_min && left_min > -right_min))
          || (right_max < 0
              && (-left_max > right_max && left_min > right_max)))
        {
          message(msg_zero_result, addr, "/");
          left_min = left_max = 0;
        } else if (right_min > 0) { 
          if (left_max < 0) { 
            left_min = left_min/right_min;
            left_max = left_max/right_max;
          } else if (left_min < 0) { 
            left_min = left_min/right_min;
            left_max = left_max/right_min;
          } else { 
            left_min = left_min/right_max;
            left_max = left_max/right_min;
          }
        } else { 
          left_min = INT8_MIN;
          left_max = INT8_MAX;
        } 
      left_mask = make_int8_mask(first_set_bit(left_mask) -  
                                 first_set_bit(right_mask));
      STORE_INT8(sp-4, min, left_min);
      STORE_INT8(sp-4, max, left_max);
      STORE_INT8(sp-4, mask, left_mask);
      sp[-4].index = NO_ASSOC_VAR;
      sp -= 2;
      break;
    case lrem:
      LOAD_INT8_OPERANDS();
      if ((right_min == 0 && right_max == 0) ||
          (left_min == 0 && left_max == 0))
        {
          message(msg_zero_operand, addr, "%");
        }
      if ((right_min > 0 
           && (left_max < right_min && left_min > -right_min))
          || (right_max < 0
              && (-left_max > right_max && left_min > right_max)))
        {
          message(msg_no_effect, addr);
        } else if (right_min > 0) {    
          left_min = (left_min < 0) ? 1-right_max : 0;
          left_max = right_max-1;
        } else { 
          left_min = INT8_MIN;
          left_max = INT8_MAX;
        }
      STORE_INT8(sp-4, min, left_min);
      STORE_INT8(sp-4, max, left_max);
      STORE_INT8(sp-4, mask, INT8_ALL_BITS);
      sp[-4].index = NO_ASSOC_VAR;
      sp -= 2;
      break;
    case lshl:
      sp -= 1;
      LOAD_INT8_OPERAND();
      if ((sp->min == 0 && sp->max == 0) ||
          (left_min == 0 && left_max == 0))
        {
          message(msg_zero_operand, addr, "<<");
        }
      if (sp->max < 0) {
        message(msg_shift_count, addr, "<<", "<=", (void*)sp->max);
      } else if(sp->min >= 64) { 
        message(msg_shift_count, addr, "<<", ">=", (void*)sp->min);
      } 
      else if ((unsigned(sp->max - sp->min) < 256) 
               && (sp->max >= 64 || sp->min < 0))
        { 
          //
          // Produce this message only if range of expression was 
          // restricted in comparence with domains of builtin types
          //
          message(msg_shift_range, addr, "<<",
                  (void*)sp->min, (void*)sp->max);
        }
      left_mask = make_int8_lshift_mask(left_mask, 
                                        sp->min, sp->max);
      if (left_mask == 0) { 
        message(msg_zero_result, addr, "<<");
      }
      if ((unsigned)sp->max < 64 && (unsigned)sp->min < 64) {  
        if (left_max >= 0) { 
          if (left_max << sp->max >> sp->max == left_max){
            left_max = left_max << sp->max;
          } else { 
            left_max = INT8_MAX;
            left_min = INT8_MIN;
          }
        } else { 
          if (left_max << sp->min >> sp->min == left_max){
            left_max = left_max << sp->min;
          } else { 
            left_max = INT8_MAX;
            left_min = INT8_MIN;
          }
        }
        if (left_min >= 0) { 
          left_min <<= sp->min;
        } else { 
          if (left_min << sp->max >> sp->max == left_min){
            left_min = left_min << sp->max;
          } else { 
            left_max = INT8_MAX;
            left_min = INT8_MIN;
          }
        }
      } else {
        left_max = INT8_MAX;
        left_min = INT8_MIN;
      }
      STORE_INT8(sp-2, min, left_min);
      STORE_INT8(sp-2, max, left_max);
      STORE_INT8(sp-2, mask, left_mask);
      sp[-2].index = NO_ASSOC_VAR;
      break;
    case lshr:
      sp -= 1;
      LOAD_INT8_OPERAND();
      if ((sp->min == 0 && sp->max == 0) ||
          (left_min == 0 && left_max == 0))
        {
          message(msg_zero_operand, addr, ">>");
        }
      if (sp->max < 0) {
        message(msg_shift_count, addr, ">>", "<=", (void*)sp->max);
      } else if(sp->min >= 64) { 
        message(msg_shift_count, addr, ">>", ">=", (void*)sp->min);
      } 
      else if ((unsigned(sp->max - sp->min) < 256) 
               && (sp->max >= 64 || sp->min < 0))
        { 
          //
          // Produce this message only if range of expression was 
          // restricted in comparence with domains of builtin types
          //
          message(msg_shift_range, addr, ">>",
                  (void*)sp->min, (void*)sp->max);
        }
      left_mask = make_int8_rshift_mask(left_mask, 
                                        sp->min, sp->max);
      if (left_mask == 0) { 
        message(msg_zero_result, addr, ">>");
      }
      if ((unsigned)sp->max < 64 && (unsigned)sp->min < 64) { 
        left_max = (left_max >= 0) 
          ? left_max >> sp->min : left_max >> sp->max;
        left_min = (left_min >= 0) 
          ? left_min >> sp->max : left_min >> sp->min;
      } else { 
        if (left_max < 0) left_max = -1;
        if (left_min > 0) left_min = 0;
      }
      STORE_INT8(sp-2, min, left_min);
      STORE_INT8(sp-2, max, left_max);
      STORE_INT8(sp-2, mask, left_mask);
      sp[-2].index = NO_ASSOC_VAR;
      break;
    case lushr:
      sp -= 1;
      LOAD_INT8_OPERAND();
      if ((sp->min == 0 && sp->max == 0) ||
          (left_min == 0 && left_max == 0))
        {
          message(msg_zero_operand, addr, ">>>");
        }
      if (sp->max < 0) {
        message(msg_shift_count, addr, ">>>", "<=", (void*)sp->max);
      } else if(sp->min >= 64) { 
        message(msg_shift_count, addr, ">>>", ">=", (void*)sp->min);
      } 
      else if ((unsigned(sp->max - sp->min) < 256) 
               && (sp->max >= 64 || sp->min < 0))
        { 
          //
          // Produce this message only if range of expression was 
          // restricted in comparence with domains of builtin types
          //
          message(msg_shift_range, addr, ">>>",
                  (void*)sp->min, (void*)sp->max);
        }
      left_mask = make_int8_rushift_mask(left_mask, sp->min, sp->max);
      if (left_mask == 0) { 
        message(msg_zero_result, addr, ">>>");
      }
      if ((unsigned)sp->max < 64 && (unsigned)sp->min < 64 
          && left_min >= 0) 
        { 
          left_max >>= sp->min;
          left_min >>= sp->max;
        } else { 
          left_max = INT8_MAX;
          left_min = INT8_MIN;
        }
      STORE_INT8(sp-2, min, left_min);
      STORE_INT8(sp-2, max, left_max);
      STORE_INT8(sp-2, mask, left_mask);
      sp[-2].index = NO_ASSOC_VAR;
      break;
    case land:
      LOAD_INT8_OPERANDS();
      if ((right_min == 0 && right_max == 0) ||
          (left_min == 0 && left_max == 0))
        {
          message(msg_zero_operand, addr, "&");
        }
      if ((left_mask &= right_mask) == 0) { 
        message(msg_zero_result, addr, "&");
      }
      left_min = left_mask < 0 ? INT8_MIN : 0;
      left_max = (right_max & left_max) >= 0
        ? left_mask & INT8_MAX : 0;
      STORE_INT8(sp-4, min, left_min);
      STORE_INT8(sp-4, max, left_max);
      STORE_INT8(sp-4, mask, left_mask);
      sp[-4].index = NO_ASSOC_VAR;
      sp -= 2;
      break;
    case lor:
      LOAD_INT8_OPERANDS();
      if ((right_min == 0 && right_max == 0) ||
          (left_min == 0 && left_max == 0))
        {
          message(msg_zero_operand, addr, "|");
        }
      left_mask |= right_mask;
      if (right_min < left_min) { 
        left_min = right_min;
      }
      left_max = left_mask & INT8_MAX;
      if (left_max < left_min) { 
        left_max = left_min;
      }
      STORE_INT8(sp-4, min, left_min);
      STORE_INT8(sp-4, max, left_max);
      STORE_INT8(sp-4, mask, left_mask);
      sp[-4].index = NO_ASSOC_VAR;
      sp -= 2;
      break;
    case lxor:
      LOAD_INT8_OPERANDS();
      if ((right_min == 0 && right_max == 0) ||
          (left_min == 0 && left_max == 0))
        {
          message(msg_zero_operand, addr, "^");
        }
      left_mask |= right_mask;
      left_min = left_mask < 0 ? INT8_MIN : 0;
      left_max = left_mask & INT8_MAX;
      STORE_INT8(sp-4, min, left_min);
      STORE_INT8(sp-4, max, left_max);
      STORE_INT8(sp-4, mask, left_mask);
      sp[-4].index = NO_ASSOC_VAR;
      sp -= 2;
      break;
    case lneg:
      LOAD_INT8_OPERAND();
      if (left_min == left_max) {
        left_min = left_max = left_mask = -left_max;
      } else { 
        int8 min = left_min; 
        left_min = (left_max == INT8_MAX)? INT8_MIN : -left_max;
        left_max = (min == INT8_MIN) ? INT8_MAX : -min;
        left_mask = make_mask(first_set_bit(left_mask));
      }
      STORE_INT8(sp-2, min, left_min);
      STORE_INT8(sp-2, max, left_max);
      STORE_INT8(sp-2, mask, left_mask);
      sp[-2].index = NO_ASSOC_VAR;
      break;
#else
    case ladd:
    case lsub:
    case lmul:
    case lidiv:
    case lrem:
    case lor:
    case lxor:
    case land:
      sp -= 2;
      break;
    case lshl:
    case lshr:
    case lushr: 
      sp -= 1;
      break;
    case lneg:
      break;
#endif
    case fadd:
    case fsub:
    case fmul:
    case fdiv:
    case frem:
      sp -= 1;
      break;
    case dadd:
    case dsub:
    case dmul:
    case ddiv:
    case drem:
      sp -= 2;
      break;
    case ineg:
      if (sp[-1].min == sp[-1].max) {
        sp[-1].mask = sp[-1].min = sp[-1].max = -sp[-1].max; 
      } else { 
        int min = sp[-1].min; 
        sp[-1].min = (sp[-1].max == ranges[tp_int].max)
          ? ranges[tp_int].min : -sp[-1].max;
        sp[-1].max = (min == ranges[tp_int].min)
          ? ranges[tp_int].max : -min;
        sp[-1].mask = make_mask(first_set_bit(sp[-1].mask));
      }
      sp[-1].type = tp_int;
      sp[-1].index = NO_ASSOC_VAR;
      break;
    case fneg:
    case dneg:
      break;
    case iinc:
      {
        var_desc* var = &vars[*pc];
        var_store_count[*pc++] += 1;
        int add = (signed char)*pc++;
        if (((var->max^add) < 0 || ((var->max+add) ^ add) >= 0)
            && ((var->min^add) < 0  || ((var->min+add) ^ add) >= 0))
          {
            var->max += add;
            var->min += add;
          } else { 
            var->max = ranges[tp_int].max;
            var->min = ranges[tp_int].min;
          }
        var->mask = make_mask(maximum(last_set_bit(var->mask),
                                      last_set_bit(add))+1, 
                              minimum(first_set_bit(var->mask), 
                                      first_set_bit(add)));
      }
      break;
    case i2l:
      if (prev_cop == imul || prev_cop == ishl) { 
        message(msg_overflow, addr);
      }
      sp->min = sp[-1].min;
      sp->max = sp[-1].max;
      sp->mask = sp[-1].mask;
      sp->type = tp_long;
      sp[-1].max >>= 31; // extend sign
      sp[-1].min >>= 31; 
      sp[-1].mask >>= 31;
      sp[-1].type = tp_long;
      sp[-1].index = NO_ASSOC_VAR;
      sp += 1;
      break;
    case f2l:
      sp += 1;
      nobreak;
    case d2l:
      sp[-2].type = tp_long; 
      sp[-2].index= NO_ASSOC_VAR; 
      sp[-2].min  = 0x80000000;
      sp[-2].max  = 0x7fffffff;
      sp[-2].mask = 0xffffffff;
      sp[-1].type = tp_long;
      sp[-1].min  = 0x00000000;
      sp[-1].max  = 0xffffffff;
      sp[-1].mask = 0xffffffff;
      break;
    case i2d:
    case f2d:
      sp[-1].type = sp[0].type = tp_double;
      sp[-1].index = sp[0].index = NO_ASSOC_VAR; 
      sp += 1;
      break;
    case i2b:
      if (sp[-1].max < ranges[tp_byte].min || 
          sp[-1].min > ranges[tp_byte].max)
        {
          message(msg_conversion, addr, "byte");
        } 
      else if ((sp[-1].mask << 24 >> 23) != (sp[-1].mask << 1)
               && sp[-1].mask != 0xffff  // char to byte 
               && sp[-1].mask != 0xff)   // int & 0xFF to byte 
        {
          message(msg_truncation, addr, "byte");
        }
      if (sp[-1].max > ranges[tp_byte].max ||
          sp[-1].min < ranges[tp_byte].min) 
        { 
          sp[-1].max = ranges[tp_byte].max;
          sp[-1].min = ranges[tp_byte].min;
        } 
      sp[-1].mask = sp[-1].mask << 24 >> 24;
      sp[-1].type = tp_byte;
      sp[-1].index = NO_ASSOC_VAR;
      break;
    case i2f:
      sp[-1].type = tp_float;
      sp[-1].index = NO_ASSOC_VAR;
      break;
    case l2i:
#ifdef INT8_DEFINED
      LOAD_INT8_OPERAND();
      if (left_max < ranges[tp_int].min || left_min > ranges[tp_int].max)
        {
          message(msg_conversion, addr, "int");
        } else if (sp[-2].mask != 0 && sp[-2].mask != ~0) {
          message(msg_truncation, addr, "int");
        }
      if (left_min >= ranges[tp_int].min &&
          left_max <= ranges[tp_int].max)
        { 
          sp[-2].min = sp[-1].min;
          sp[-2].max = sp[-1].max;
        } else { 
          sp[-2].min = ranges[tp_int].min;
          sp[-2].max = ranges[tp_int].max;
        }
#else
      sp[-2].min = ranges[tp_int].min;
      sp[-2].max = ranges[tp_int].max;
#endif      
      sp[-2].type = tp_int;
      sp[-2].mask = sp[-1].mask;
      sp[-2].index = NO_ASSOC_VAR;
      sp -= 1;
      break;
    case d2i:
      sp -= 1;
      sp[-1].type = tp_int;
      sp[-1].min = ranges[tp_int].min;
      sp[-1].max = ranges[tp_int].max;
      sp[-1].mask = ALL_BITS;
      sp[-1].index = NO_ASSOC_VAR;
      break;
    case l2f:
    case d2f:
      sp -= 1;
      sp[-1].type = tp_float;
      sp[-1].index = NO_ASSOC_VAR;
      break;
    case l2d:
      sp[-1].type = sp[-2].type = tp_double;
      sp[-1].index = sp[-2].index = NO_ASSOC_VAR;
      break;
    case f2i:
      sp[-1].type = tp_int;
      sp[-1].min = ranges[tp_int].min;
      sp[-1].max = ranges[tp_int].max;
      sp[-1].mask = ALL_BITS;
      sp[-1].index = NO_ASSOC_VAR;
      break;
    case i2c:
      if (sp[-1].max < ranges[tp_char].min || 
          sp[-1].min > ranges[tp_char].max)
        {
          message(msg_conversion, addr, "char");
        } 
      else if ((sp[-1].mask & 0x7fff0000) != 0 &&
               (sp[-1].mask & 0x7fff0000) != 0x7fff0000)
        {
          message(msg_truncation, addr, "char");
        }
      if (sp[-1].max > ranges[tp_char].max ||
          sp[-1].min < ranges[tp_char].min) 
        { 
          sp[-1].min = ranges[tp_char].min;
          sp[-1].max = ranges[tp_char].max;
        } 
      sp[-1].mask &= 0xFFFF;
      sp[-1].type = tp_char;
      sp[-1].index = NO_ASSOC_VAR;
      break;
    case i2s:
      if (sp[-1].max < ranges[tp_short].min || 
          sp[-1].min > ranges[tp_short].max)
        {
          message(msg_conversion, addr, "short");
        } else if ((sp[-1].mask << 16 >> 15) != (sp[-1].mask << 1)) {
          message(msg_truncation, addr, "short");
        }
      if (sp[-1].max > ranges[tp_short].max ||
          sp[-1].min < ranges[tp_short].min) 
        { 
          sp[-1].max = ranges[tp_short].max;
          sp[-1].min = ranges[tp_short].min;
        } 
      sp[-1].mask = sp[-1].mask << 16 >> 16;
      sp[-1].type = tp_short;
      sp[-1].index = NO_ASSOC_VAR;
      break;
    case lcmp:
#ifdef INT8_DEFINED
      LOAD_INT8_OPERANDS();
      if (*pc == ifeq || *pc == ifne) { 
        if ((left_mask & right_mask) == 0 
            && !((left_min == 0 && left_max == 0) ||
                 (right_min == 0 && right_max == 0)))
          { 
            message(msg_disjoint_mask, addr);
          }
      }
      if (left_min > right_max || left_max < right_min
          || ((*pc == ifge || *pc == iflt) && left_min >= right_max)
          || ((*pc == ifle || *pc == ifgt) && left_max <= right_min))
        {
          message(msg_same_result, addr);
        } 
      else if (((*pc == ifgt || *pc == ifle) && left_min == right_max) ||
               ((*pc == iflt || *pc == ifge) && left_max == right_min))
        {
          message(msg_weak_cmp, addr);
        }
#endif
      sp -= 4;
      sp->type = tp_int;
      sp->min = -1;
      sp->max = 1;
      sp->mask = ALL_BITS;
      sp->index = NO_ASSOC_VAR;
      sp += 1;
      break;
    case dcmpl:
    case dcmpg:
      sp -= 2; 
      nobreak;
    case fcmpl:
    case fcmpg:
      sp -= 2;
      sp->type = tp_int;
      sp->min = -1;
      sp->max = 1;
      sp->mask = ALL_BITS;
      sp->index = NO_ASSOC_VAR;
      sp += 1;
      break;
    case ifeq:
    case ifne:
    case iflt:
    case ifge:
    case ifgt:
    case ifle:
      sp -= 1; 
      if (sp->min > 0 || sp->max < 0 || sp->min == sp->max
          || (sp->min == 0 && (cop == iflt || cop == ifge)) 
          || (sp->max == 0 && (cop == ifgt || cop == ifle))) 
        {
          message(msg_same_result, addr);
        } 
      else if ((sp->min == 0 && (cop == ifle || cop == ifgt)) || 
               (sp->max == 0 && (cop == ifge || cop == iflt)))
        {
          message(msg_weak_cmp, addr);
        }
      pc += 2; 
      break;
    case if_icmpeq:
    case if_icmpne:
      if (sp[-2].min == sp[-2].max && sp[-1].min == sp[-1].max) { 
        message(msg_same_result, addr);
      } else if ((sp[-2].mask & sp[-1].mask) == 0 
                 && sp[-2].type != tp_bool && sp[-1].type != tp_bool) 
        { 
          message(msg_disjoint_mask, addr);
        }
      nobreak;
    case if_icmplt:
    case if_icmpge:
    case if_icmpgt:
    case if_icmple:
      if (sp[-2].min > sp[-1].max || sp[-2].max < sp[-1].min
          || ((cop == if_icmpge || cop == if_icmplt) 
              && sp[-2].min >= sp[-1].max)
          || ((cop == if_icmple || cop == if_icmpgt)
              && sp[-2].max <= sp[-1].min))
        {
          message(msg_same_result, addr);
        } 
      else if (((cop == if_icmpgt || cop == if_icmple) 
                && sp[-2].min == sp[-1].max) ||
               ((cop == if_icmplt || cop == if_icmpge)
                && sp[-2].max == sp[-1].min))
        {
          message(msg_weak_cmp, addr);
        }
      if ((sp[-1].type == tp_short && sp[-2].type == tp_char) 
          || (sp[-2].type == tp_short && sp[-1].type == tp_char))
        {
          message(msg_short_char_cmp, addr);
        }
      sp -= 2;
      pc += 2;
      break;
    case if_acmpeq:
    case if_acmpne:
      if (sp[-1].type == tp_string && sp[-2].type == tp_string) { 
        message(msg_string_cmp, addr);
      }
      sp -= 2;
      pc += 2;
      break;
    case jsr:
    case goto_near:
      pc += 2;
      break;
    case ret:
      pc += 1;
      break;
    case tableswitch:
      { 
        pc += 7 - (addr & 3);
        int low = unpack4(pc);
        int high = unpack4(pc+4);
        sp -= 1;
        if (low < sp->min) { 
          message(msg_incomp_case, addr, low);
        } 
        if (high > sp->max) {
          message(msg_incomp_case, addr,  high);
        } 
        pc += (high - low + 1)*4 + 8;
      }
      break;
    case lookupswitch:
      { 
        pc += 7 - (addr & 3);
        int n_pairs = unpack4(pc);
        pc += 4;
        sp -= 1;
        while (--n_pairs >= 0) { 
          int val = unpack4(pc);
          int label = unpack4(pc+4);
          pc += 8;
          if (val < sp->min || val > sp->max || (val & ~sp->mask)) { 
            message(msg_incomp_case, addr+label, val);
          } 
        }
      } 
      break;
    case ireturn:
    case freturn:
    case areturn:
      sp -= 1;
      break;
    case dreturn:
    case lreturn:
      sp -= 2;
      break;
    case vreturn:
      break;
    case getfield:
      sp -= 1;
      nobreak;
    case getstatic:
      {
        const_ref* field_ref = (const_ref*)constant_pool[unpack2(pc)];
        const_name_and_type* nt = 
          (const_name_and_type*)constant_pool[field_ref->name_and_type];
        const_utf8* field_name= (const_utf8*)constant_pool[nt->name];
        const_utf8* desc = (const_utf8*)constant_pool[nt->desc];
        const_class*cls_info=(const_class*)constant_pool[field_ref->cls];
        const_utf8* cls_name=(const_utf8*)constant_pool[cls_info->name];
        class_desc* obj_cls = class_desc::get(*cls_name);
        field_desc* field = obj_cls->get_field(*field_name);
        field->name_and_type = nt;

        if (cop == getfield) { 
          check_variable_for_null(addr, sp);
        }
        if (cls->name == *cls_name) { 
          field->attr |= field_desc::f_used;
        }
        if (!in_monitor) { 
          accessors = new access_desc(field, cls, 
                                      get_line_number(addr), 
                                      accessors);
          if (cop == getfield) { 
            if (sp->type == tp_self) { 
              accessors->attr |= access_desc::a_self;
            } 
            if (sp->mask & var_desc::vs_new) { 
              accessors->attr |= access_desc::a_new;
            } 
          }
        }
        int type = get_type(*desc);
        sp->type = type;
        if (IS_INT_TYPE(type)) { 
          sp->min = ranges[type].min;
          sp->max = ranges[type].max;
          sp->mask = sp->min|sp->max;
        } else { 
          sp->mask = var_desc::vs_unknown;
          sp->min = 0;
          sp->max = MAX_ARRAY_LENGTH;
        }
        sp->equals = field;
        sp->index = NO_ASSOC_VAR;
        sp += 1;
        if (type == tp_long || type == tp_double) { 
          sp->type = type;
          sp->max = 0xffffffff;
          sp->min = 0;
          sp->mask = 0xffffffff;
          sp[-1].max = 0x7fffffff;
          sp[-1].min = 0x80000000;
          sp[-1].mask = 0xffffffff;
          sp += 1;
        }
        pc += 2;
#ifdef DUMP_STACK
        printf("<stack_top> := %x %s\n", 
               (int)field,
               field == NULL ? 
               "<unknown>" : field->name.as_asciz()
               );
#endif
      }
      break;
    case putfield:
    case putstatic:
      {
        const_ref* field_ref = (const_ref*)constant_pool[unpack2(pc)];
        const_name_and_type* nt = 
          (const_name_and_type*)constant_pool[field_ref->name_and_type];
        const_utf8* desc = (const_utf8*)constant_pool[nt->desc];
        const_class*cls_info=(const_class*)constant_pool[field_ref->cls];
        const_utf8* cls_name=(const_utf8*)constant_pool[cls_info->name];
        const_utf8* field_name= (const_utf8*)constant_pool[nt->name];
        class_desc* obj_cls = class_desc::get(*cls_name);
        field_desc* field = obj_cls->get_field(*field_name);

        if (cls->name == *cls_name) { 
          field->attr |= field_desc::f_used;
          if ((name != "<init>") && (name != "<clinit>") &&
              (! (cls->locks.owns(is_this)))) {
            // allow assignments to lock only inside monitor or constructor

            // should also check whether monitor field is owned (if so, error)
            if ((cls->locks.owns(field)) ||
                (locksAtEntry.owns(field))) {
              if (strchr(field->name.as_asciz(), '$') == NULL) {
                // don't print warning for variable names with $
                // because javac sometimes does weird stuff for class locks
                message_at(msg_lock_assign2, cls->source_file,
                         get_line_number(addr), field);
              }
            } else {
              if (strchr(field->name.as_asciz(), '$') == NULL) {
                // don't print warning for variable names with $
                // because javac sometimes does weird stuff for class locks
                int linenumber = get_line_number(addr);
                field->write(linenumber);
                if (field->equals) {
                  const_cast<field_desc *>(field->equals)->write(linenumber);
                  // mark field as written to (const_cast req'd here)
                }
              }
            }
          }
        }
        if (!in_monitor) { 
          accessors = new access_desc(field, cls, 
                                      get_line_number(addr), 
                                      accessors);
        }
        int type = get_type(*desc);
        sp -= (type == tp_long || type == tp_double) ? 2 : 1;
        if (sp->equals != NULL) {
          if (sp->equals->name_and_type != nt) {
            /*field_desc* aux = new field_desc(*(sp->equals));
            aux->name_and_type = nt;
            sp->equals = aux;
            equal_descs.push_back(aux);*/
            sp->equals = is_const;
          }
        }
        field->equals = sp->equals;
        if (cop == putfield) { 
          sp -= 1;
          if (!in_monitor) { 
            if (sp->type == tp_self) { 
              accessors->attr |= access_desc::a_self;
            }
            if (sp->mask & var_desc::vs_new) { 
              accessors->attr |= access_desc::a_new; 
            }
          }
          check_variable_for_null(addr, sp);
        }
#ifdef DUMP_STACK
        printf("%x %s := %x %s\n", 
               (int)field, 
               field_name->as_asciz(),
               (int)field->equals,
               field->equals == NULL ? 
               "<unknown>" : field->equals->name.as_asciz()
               );
#endif
        pc += 2;
      }
      break;
    case invokespecial:
    case invokevirtual:
    case invokestatic:
    case invokeinterface:
      {
        const_ref* method_ref = (const_ref*)constant_pool[unpack2(pc)];
        const_class* cls_info = 
          (const_class*)constant_pool[method_ref->cls];
        utf_string cls_name(*(const_utf8*)constant_pool[cls_info->name]);
        class_desc* obj_cls = class_desc::get(cls_name);
        const_name_and_type* nt = 
          (const_name_and_type*)constant_pool[method_ref->name_and_type];
        utf_string mth_name(*(const_utf8*)constant_pool[nt->name]);
        utf_string mth_desc(*(const_utf8*)constant_pool[nt->desc]);

        int n_params = get_number_of_parameters(mth_desc);
        int fp = n_params;
        if (cop != invokestatic) { 
          fp += 1;
          check_variable_for_null(addr, sp-fp);
        }

#ifdef DUMP_STACK
        printf("<stack_top> = %s\n",
               sp[-fp].equals == NULL ? 
               "<unknown>" : sp[-fp].equals->name.as_asciz()
               );
#endif
        if (cls_name == "java/lang/Object") { 
          bool hold_lock = false;
          Lock wait_on;
          if (sp[-fp].equals != NULL) { 
            wait_on = sp[-fp].equals;
          } else {
            wait_on = NULL;
          }
          if (mth_name == "notify" 
              || mth_name == "notifyAll" 
              || mth_name == "wait") { 
            // check whether lock on object which is waited on is owned
            if (wait_on != NULL) {
              if (cls->locks.owns(wait_on)) {
                hold_lock = true;
              }
            }
            if (!hold_lock) {
              message(msg_wait_nosync, addr, 
                      (wait_on != NULL ? 
                       wait_on->name.as_asciz() :
                       "<unknown>"), 
                      &mth_name);
            }
          }
          if (mth_name == "wait") { 
            wait_line = get_line_number(addr);
            attr |= m_wait;
            // check whether other locks are held
            int nLocks = cls->locks.nLocks() - (hold_lock? 1:0) > 0;
            if (nLocks > 0) {
              message(msg_wait, addr);
              // print all other locks
              char buf[MAX_MSG_LENGTH - 40]; // buffer for locks
              char* out = buf;
              int n;
              monitor_stack::const_iterator entry = cls->locks.begin();
              while (entry != cls->locks.end()) {
                if ((n = snprintf(out, 
                                  sizeof(buf)-(out-buf), 
                                  " %s,", 
                                  (*entry)->name.as_asciz())) == -1) {
                  // no space in buffer left - print "..." at end of buffer
                  sprintf(buf+sizeof(buf)-4, "...");
                  out = buf + sizeof(buf) - 1;
                  break;
                }
                out += n;
                entry++;
              }
              *(out-1) = '\0';
              message(msg_locklist, addr, cls->locks.nLocks(), buf);
              n_messages--; // avoid counting message twice
            }
          }
        } // end of wait/notify treatment

        if ((cop == invokespecial) && (mth_name == "finalize")) {
          super_finalize = true;
        } else { 
          method_desc* method = obj_cls->get_method(mth_name, mth_desc);
          int call_attr = 0;
          if (cop != invokestatic && sp[-fp].type == tp_self) {
            call_attr |= callee_desc::i_self;
          } 
          if (in_monitor) {
            call_attr |= callee_desc::i_synchronized;
          }

          callees = new callee_desc(cls, method, callees,
                                    get_line_number(addr), call_attr);

          if ((cls->locks.nLocks() > 0)
              && (method->name != "<init>")
              && (method->name != "<clinit>")
              ) {
            // if locks are currently held...
            // ... add call from innermost "pseudo method" to method
            // exception: constructors can't own locks
            method->attr |= m_concurrent;
            Lock curr = cls->locks.getInnermost();

            if (curr == is_this) { 
              call_attr |= callee_desc::i_self; 
            }
            if ((curr->cls == cls) &&
                (method->cls == cls)) { // ignore calls to other classes
              const char* curr_name =
                compound_name(cls->name.as_asciz(), curr->name.as_asciz());
              class_desc* curr_cls = class_desc::get(utf_string(curr_name));
              
              method_desc* caller_method =
                curr_cls->get_method(utf_string("<synch>"), utf_string("()"));
              // caller_method->attr |= method_desc::m_synchronized;

              if (caller_method->vertex == NULL) {
                caller_method->vertex = new graph_vertex(curr_cls);
              }

              if ((!(method->locksAtEntry.owns(curr))) &&
                  (sp[-fp].equals == is_this) && // (method->cls == cls)
                  (! (attr & m_synchronized))
                  // synch. methods are already covered by other mechanisms
                  ) {
                // context at method entry
                
                method->locksAtEntry.acquire(curr);
                // add call graph edge lock.<synch> -> method
                // only if method is of current class
                // and not added before

                // callee
                method->attr |= method_desc::m_sync_block;
                if (method->vertex == NULL) {
                  method->vertex = new graph_vertex(obj_cls);
                }
              
                method->callees =
                  new callee_desc(obj_cls, method, method->callees,
                                  get_line_number(addr), call_attr);
		
                // lock was not in list, add to call graph
		
                int caller_attr = 0;
                // caller_attr |= method_desc::m_synchronized;
                // attr |= m_synchronized; // crucial flag for later analysis?
                method->build_call_graph(caller_method, method->callees,
                                         caller_attr);
		
                // add edge for pseudo method to call graph
              }
            }
          } // end of pseudo method -> method call

          if (n_params < 32) { 
            int mask = 0;
            for (i = 0; i < n_params; i++) { 
              if ((sp[i-n_params].type == tp_object 
                   || sp[i-n_params].type == tp_string)
                  && sp[i-n_params].mask == 0) 
                { 
                  mask |= 1 << i;
                }
            }
            if (cop != invokestatic) { 
              // Reference to target object i passed as 0 parameter
              mask <<= 1;
            }
            method->null_parameter_mask |= mask;
          }
        }
        int type = get_type(mth_desc);
        sp -= fp;
        if (type != tp_void) { 
          if (IS_INT_TYPE(type)) { 
            sp->min = ranges[type].min;
            sp->max = ranges[type].max;
            sp->mask = sp->min|sp->max;
          } else { 
            sp->min = 0;
            sp->max = MAX_ARRAY_LENGTH;
            sp->mask = var_desc::vs_unknown;
            sp->equals = NULL; // we don't know what we get back!
          }
          sp->index = NO_ASSOC_VAR;
          sp->type = type;
          sp += 1;
          if (type == tp_long || type == tp_double) { 
            sp->type = type;
            sp->max = 0xffffffff;
            sp->min = 0;
            sp->mask = 0xffffffff;
            sp[-1].max = 0x7fffffff;
            sp[-1].min = 0x80000000;
            sp[-1].mask = 0xffffffff;
            sp += 1;
          }
        }
        pc += (cop == invokeinterface) ? 4 : 2; 
      }
      break;
    case anew:
      sp->type = tp_object;
      sp->mask = var_desc::vs_new|var_desc::vs_not_null;
      sp->min = 0;
      sp->max = MAX_ARRAY_LENGTH;
      sp->index = NO_ASSOC_VAR;
      sp->equals = getNew();
      sp += 1;
      pc += 2;
      break;
    case newarray:
      sp[-1].type = array_type[*pc++] + indirect;
      sp[-1].mask = var_desc::vs_new|var_desc::vs_not_null;
      sp[-1].index = NO_ASSOC_VAR;
      if (sp[-1].min < 0) {
        if (sp[-1].max < 0) { 
          message(msg_neg_len, addr, sp[-1].min, sp[-1].max);
        } else if (sp[-1].min > -128) { 
          message(msg_maybe_neg_len, addr, sp[-1].min, sp[-1].max);
        } 
      }
      if (sp[-1].min < 0) { 
        sp[-1].min = 0;
        if (sp[-1].max < 0) { 
          sp[-1].max = 0;
        }
      }
      break;
    case anewarray:
      sp[-1].type = tp_object + indirect;
      sp[-1].mask = var_desc::vs_new|var_desc::vs_not_null;
      sp[-1].index = NO_ASSOC_VAR;
      if (sp[-1].min < 0) {
        if (sp[-1].max < 0) { 
          message(msg_neg_len, addr, sp[-1].min, sp[-1].max);
        } else if (sp[-1].min > -128) { 
          message(msg_maybe_neg_len, addr, sp[-1].min, sp[-1].max);
        } 
      }
      if (sp[-1].min < 0) { 
        sp[-1].min = 0;
        if (sp[-1].max < 0) { 
          sp[-1].max = 0;
        }
      }
      pc += 2;
      break;
    case arraylength:
      sp[-1].type = tp_int;
      sp[-1].mask = make_mask(last_set_bit(sp[-1].max), 0);
      break;
    case athrow:
      sp -= 1;
      break;
    case checkcast:
      pc += 2;
      break;
    case instanceof:
      sp[-1].type = tp_bool;
      sp[-1].min = 0;
      sp[-1].max = 1;
      sp[-1].mask = 1;
      sp[-1].index = NO_ASSOC_VAR;
      pc += 2;
      break;
    case monitorenter:
      {
        in_monitor += 1;
        sp -= 1;
        if (sp->equals != NULL) {
          // mark monitor ownership; use monitor count in later version
          Lock curr = cls->locks.getInnermost();
          cls->locks.acquire(sp->equals);
          cls->usedLocks.acquire(sp->equals);
          if (sp->equals == is_this) {
            attr |= m_synchronized; // mark method as "synchronized"
          }
          if (sp->equals->name_and_type != NULL) {
            class_desc* curr_cls;
            const class_desc* curr_type;
            
            method_desc* caller_method;
            const char* curr_class_name;
            if (curr != NULL) {
              // already holding a lock
              // get class descriptor of current innermost lock (a)
              // instead of normal class descriptor, use TYPE.variable_name
              // this will allow us to distinguish between instances (imperfectly)
              /* utf_string* curr_class_type = 
                 dynamic_cast<utf_string*>(constant_pool[curr->name_and_type->desc]); */
              // if method is synchronized, use method as caller - otherwise,
              // use pseudo method OWNER.var_name.<synch>
              if (curr == is_this) {
                curr_type = curr_cls = cls;
                caller_method = this;
              } else {
                if (curr->name_and_type->name == 0) {
                  curr_cls = cls;
                } else {
                  curr_cls = curr->cls;
                }
                curr_type = curr_cls;
                // use pseudo class name "owner.name" to distinguish between
                // fields of different objects
                curr_class_name = curr_cls->name.as_asciz();
                const char* curr_name =
                  compound_name(curr_class_name, curr->name.as_asciz());
                curr_cls = class_desc::get(utf_string(curr_name));
              
                caller_method =
                  curr_cls->get_method(utf_string("<synch>"), utf_string("()"));
                // caller_method->attr |= method_desc::m_synchronized;
                // disable?
              }
            } else { // no lock acquired yet in this method
              // add call graph edge (this method) -> (lock)
              // get method descriptor of this method
              caller_method = this;
              curr_type = curr_cls = cls;
              curr_class_name = curr_cls->name.as_asciz();
            }
            if (sp->equals->cls == curr_type) {
              // only analyze locks on fields of current class

              if (caller_method->vertex == NULL) {
                caller_method->vertex = new graph_vertex(curr_cls);
              }              
              // get class descriptor of object type of new lock (b)
              const char* lock_name =
                compound_name(sp->equals->cls->name.as_asciz(), // class name
                              sp->equals->name.as_asciz()); // field name
              class_desc* obj_cls = class_desc::get(utf_string(lock_name));

              // get descriptor of dummy method "owner.b.<synch>"
              method_desc* method = 
                obj_cls->get_method(utf_string("<synch>"), 
                                    utf_string("()"));
              method->attr |= method_desc::m_synchronized;
              if (method->vertex == NULL) {
                method->vertex = new graph_vertex(obj_cls);
              }
              obj_cls->source_file = cls->source_file;
              
              // add call from a.<synch> to b.<synch>
              int call_attr = 0;
              call_attr |= callee_desc::i_synchronized;

              if (sp->equals->cls == cls) {
                call_attr |= callee_desc::i_self;
              } 

              method->callees =
                new callee_desc(obj_cls, method, method->callees,
                                get_line_number(addr), call_attr);

              if (caller_method->attr & m_sync_block) {
                method->attr |= m_sync_block;
              }

              int caller_attr= 0;
              caller_attr |= method_desc::m_synchronized;

              method->build_call_graph(caller_method, method->callees,
                                       caller_attr);
            }
#ifdef DUMP_MONITOR
          printf("%s acquires lock on %s.%s (type %s)\n", 
                 cls->name.as_asciz(),
                 sp->equals->cls->name.as_asciz(),
                 sp->equals->name.as_asciz(),
                 ((const_utf8*)constant_pool[sp->equals->name_and_type->desc])->as_asciz()
                 );
#endif
          }
        }
      }    
      break;
    case monitorexit:
      {
        if (in_monitor > 0) { 
          in_monitor -= 1;
        }
        sp -= 1;
        if (sp->equals != NULL) {
#ifdef DUMP_MONITOR
          printf("%s relinquishes lock on %s.%s\n", 
                 cls->name.as_asciz(),
                 sp->equals->cls->name.as_asciz(),
                 sp->equals->name.as_asciz()
                 );
#endif  
          cls->locks.release(sp->equals);
        }
      }
      break;
    case wide:
      switch (*pc++) { 
      case aload:
        { 
          var_desc* var = &vars[unpack2(pc)];
          if (var->type == tp_void) { 
            var->type = tp_object;
            var->min = 0;
            var->max = MAX_ARRAY_LENGTH;
            var->mask = var_desc::vs_unknown;
          }
          sp->type = var->type;
          sp->min =  var->min; 
          sp->max =  var->max; 
          sp->mask = var->mask; 
          sp->index = var - vars;
          sp->equals = var->equals;
          sp += 1;
        }
        break;
      case iload:
        { 
          var_desc* var = &vars[unpack2(pc)];
          if (var->type == tp_void) { 
            var->type = tp_int;
            var->min = ranges[tp_int].min;
            var->max = ranges[tp_int].max;
            var->mask = ALL_BITS;
          }
          sp->type = var->type;
          sp->min =  var->min; 
          sp->max =  var->max; 
          sp->mask = var->mask; 
          sp->index = var - vars;
          sp += 1;
        }
        break;
      case dload:
        sp[0].type = sp[1].type = tp_double; 
        sp[0].index = sp[1].index = NO_ASSOC_VAR; 
        sp += 2;
        break;
      case lload:
        { 
          int index = unpack2(pc);
          if (vars[index].type == tp_void) { 
            vars[index].type = tp_long;
            vars[index].min = 0x80000000;
            vars[index].max = 0x7fffffff;
            vars[index].mask = 0xffffffff;
            vars[index+1].min = 0x00000000;
            vars[index+1].max = 0xffffffff;
            vars[index+1].mask = 0xffffffff;
          }
          sp->type = tp_long;
          sp->min  = vars[index].min;
          sp->max  = vars[index].max;
          sp->mask = vars[index].mask;
          sp->index = index;
          sp += 1;
          index += 1;
          sp->type = tp_long;
          sp->min  = vars[index].min;
          sp->max  = vars[index].max;
          sp->mask = vars[index].mask;
          sp += 1;
        }
        break;
      case fload:
        sp->type = tp_float; 
        sp->index = NO_ASSOC_VAR;
        sp += 1;
        break;
      case istore:
        { 
          int index = unpack2(pc);
          var_store_count[index] += 1;
          var_desc* var = &vars[index];
          sp -= 1;  
          var->min = sp->min; 
          var->max = sp->max; 
          var->mask = sp->mask; 
          if (var->type == tp_void) { 
            var->type = tp_int;
          }
        }
        break;
      case astore:
        { 
          int index = unpack2(pc);
          var_store_count[index] += 1;
          sp -= 1;  
          vars[index].min = sp->min; 
          vars[index].max = sp->max; 
          vars[index].mask = sp->mask; 
          if (vars[index].type == tp_void) { 
            vars[index].type = tp_object;
          }
        }
        break;
      case fstore:
        sp -= 1;
        break;
      case lstore:
        {
          int index = unpack2(pc);
          sp -= 2;
          vars[index].type = tp_long;
          vars[index].max  = sp[0].max;
          vars[index].min  = sp[0].min;
          vars[index].mask = sp[0].mask;
          vars[index+1].max  = sp[1].max;
          vars[index+1].min  = sp[1].min;
          vars[index+1].mask = sp[1].mask;
          var_store_count[index] += 1;
        }
        break;
      case dstore:
        sp -= 2;
        break;      
      case ret:
        break;
      case iinc:  
        { 
          int index = unpack2(pc);
          var_store_count[index] += 1;
          var_desc* var = &vars[index];
          int add = (short)unpack2(pc+2);
          if (((var->max^add) < 0 || ((var->max+add) ^ add) >= 0)
              && ((var->min^add) < 0  || ((var->min+add) ^ add) >= 0))
            {
              var->max += add;
              var->min += add;
            } else { 
              var->max = ranges[tp_int].max;
              var->min = ranges[tp_int].min;
            }
          var->mask = make_mask(maximum(last_set_bit(var->mask),
                                        last_set_bit(add))+1, 
                                minimum(first_set_bit(var->mask), 
                                        first_set_bit(add)));
          pc += 2;  
        }
      }
      pc += 2;
      break;
    case multianewarray:
      {
        int dimensions = pc[2];
        sp -= dimensions;
        sp->type = tp_int + indirect*dimensions;
        sp->index = NO_ASSOC_VAR;
        sp->mask = var_desc::vs_new|var_desc::vs_not_null;
        for (int j = 0; j < dimensions; j++) { 
          if (sp[j].min < 0) {
            if (sp[j].max < 0) { 
              message(msg_neg_len, addr, sp[j].min, sp[j].max);
            } else if (sp[j].min > -128) { 
              message(msg_maybe_neg_len, addr, 
                      sp[j].min, sp[j].max);
            } 
          }
        }
        if (sp->min < 0) { 
          sp->min = 0;
          if (sp->max < 0) { 
            sp->max = 0;
          }
        }
        sp += 1;
        pc += 3;
        break;
      }
    case ifnull:
    case ifnonnull:
      pc += 2;
      sp -= 1;
      break;
    case jsr_w:
    case goto_w:
      pc += 4;
      break;
    default:
      assert(false/*Illegal byte code*/);
    }
    prev_cop = cop;
  }
  assert(sp == stack_bottom);

  // pop-up local variables
  for (ctx = context[code_length]; ctx != NULL; ctx = ctx->next) {
    sp = ctx->transfer(this, sp, unused, prev_cop);
  }
  // reset class fields attributes
  for (field_desc* field = cls->fields; field != NULL; field = field->next){ 
    field->attr &= ~field_desc::f_used;
  }
  if (name == "finalize" && !super_finalize) { 
    message(msg_super_finalize, 0);
  }
  
  // cleanup
  
  if (attr & m_synchronized) { // remove "this" from lock set if needed
    cls->locks.release(is_this);
  }

  /*
  for (field_desc** fd = equal_descs.begin(); fd != equal_descs.end(); fd++) {
    delete *fd;
  }
  equal_descs.clear();
  */

  for (i = code_length; i >= 0; i--) { 
    local_context* next;
    for (ctx = context[i]; ctx != NULL; ctx = next) {
      next = ctx->next;
      delete ctx;
    }
  }
  delete[] var_store_count;
  delete[] line_table;
  delete[] context;
}

field_desc* method_desc::getNew() {
  char* name = (char *)malloc((size_t)10);
  strcpy(name, "<new>");
  //  assert(equal_descs.size() < 10000); // 4 characters allowed
  assert(new_cnt < 10000); // 4 characters allowed
  snprintf(name + strlen(name), 4, "%d", new_cnt++);
  const char* fd_name = stringPool.add(name);
  free(name);
  field_desc* fd = new field_desc(utf_string(fd_name), cls, NULL);
  // equal_descs.push_back(fd);
  return fd;
}

const char* compound_name (const char* first, const char* second) {
  char* result = 
    (char *)malloc((size_t)(strlen(first) +
                            strlen(second) + 2));
  /* Add one byte for '.' and for '\0'. */
  strcpy(result, first);
  char* tmp = result + strlen(first);
  // point to \0
  *tmp = '.';
  strcpy(++tmp, second);
  const char* retval = stringPool.add(result);
  free(result);
  return retval;
}
