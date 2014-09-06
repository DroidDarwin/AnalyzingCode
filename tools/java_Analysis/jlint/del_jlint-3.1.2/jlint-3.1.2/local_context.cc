#include "local_context.hh"

/*
** every subclass of local_context has a transfer method which keeps track
** of the variable states during branching. These transfer methods are 
** called in method_desc::parse_code(
**                constant** constant_pool, const field_desc* is_this)
** for every bytecode and at the end after parsing all the bytecode of the method 
** before cleanup to pop up all the local variables.
*/



vbm_operand* ctx_push_var::transfer(method_desc* method, vbm_operand* sp, 
                                    byte, byte&)
{
  var_desc* var = &method->vars[var_index];
  if (var->type == tp_void) { 
    if (IS_INT_TYPE(var_type)) { 
      var->min  = ranges[var_type].min;
      var->max  = ranges[var_type].max;
      var->mask = var->min|var->max;
    } else if (var_type == tp_long) { 
      var[0].min  = 0x80000000;
      var[0].max  = 0x7fffffff;
      var[0].mask = 0xffffffff;
      var[1].min  = 0;
      var[1].max  = 0xffffffff;
      var[1].mask = 0xffffffff;      
    } else { 
      var->mask = (var->type == tp_self) 
        ? var_desc::vs_not_null : var_desc::vs_unknown;
      var->min = 0;
      var->max = MAX_ARRAY_LENGTH;
    }
  }
  var->type = var_type;
  var->name = *var_name;
  var->start_pc = var_start_pc;
  return sp;
}

vbm_operand* ctx_pop_var::transfer(method_desc* method, vbm_operand* sp, 
                                   byte, byte&)
{
  var_desc* var = &method->vars[var_index];
  for (field_desc* field = method->cls->fields; 
       field != NULL; 
       field = field->next)
    { 
      // Do not produce message about shadowing of class camponent by local
      // variable in case when local variable is formal parameter of the
      // method and programmer explicitly refer class object component
      // by "this": this.x = x;
      if (field->name == var->name 
          && (!(field->attr & field_desc::f_used) 
              || var->start_pc != 0 /* not formal parameter*/))
        {
          method->message(msg_shadow_local, var->start_pc, 
                          &var->name, method->cls);
          break;
        }
    }
  var->type = tp_void;
  var->name = utf_string("???");
  return sp;
}


vbm_operand* ctx_split::transfer(method_desc* method, vbm_operand* sp, 
                                 byte cop, byte& prev_cop)
{
  if (n_branches > 0 && cmd == cmd_save_ctx) { 
    vars = new var_desc[method->n_vars];
    memcpy(vars, method->vars, method->n_vars*sizeof(var_desc));
  } else { 
    vars = NULL;
  }
  switch_var_index = -1;
  in_monitor = method->in_monitor;

  vbm_operand* left_op = sp-2;
  vbm_operand* right_op = sp-1;
    
  switch (cop) { 
  case jsr:
  case jsr_w:  
    sp->type = tp_object;
    sp->mask = var_desc::vs_not_null;
    sp->min = 0;
    sp->max = MAX_ARRAY_LENGTH;
    stack_pointer = sp+1;
    break;
  case tableswitch:
  case lookupswitch:
    stack_pointer = right_op;
    switch_var_index = right_op->index;
    break;
  case ifeq:
    if (vars != NULL && right_op->index >= 0) { 
      // state at the branch address
      var_desc* var = &vars[right_op->index];
      if (prev_cop == iand) { 
        // Operation of form (x & const) == 0
        if (IS_INT_TYPE(var->type)) { 
          var->mask &= ~right_op->mask;
        }
      } else { 
        var->max = var->min = 0;
        if (IS_INT_TYPE(var->type)) { 
          var->mask = 0;
        }
      } 
    }
    stack_pointer = right_op;
    break;
  case ifne:
    if (right_op->index >= 0) { // value of local var. was pushed on stack 
      // state after if
      var_desc* var = &method->vars[right_op->index];
      if (prev_cop == iand) { 
        // Operation of form (x & const) != 0
        if (IS_INT_TYPE(var->type)) { 
          var->mask &= ~right_op->mask;
        }
      } else { 
        var->max = var->min = 0;
        if (IS_INT_TYPE(var->type)) { 
          var->mask = 0;
        }
      }
    }
    stack_pointer = right_op;
    break;
  case iflt:
    if (right_op->index >= 0) { 
      // state after if
      var_desc* var = &method->vars[right_op->index];
      if (var->min < 0) var->min = 0;
      if (var->max < 0) var->max = 0;
      if (IS_INT_TYPE(var->type)) { 
        var->mask &= ~SIGN_BIT;
      }
      if (vars != NULL) { // forward branch
        // state at the branch address
        var = &vars[right_op->index];
        if (var->min >= 0) var->min = -1;
        if (var->max >= 0) var->max = -1;
      }
    }
    stack_pointer = right_op;
    break;      
  case ifge:
    if (right_op->index >= 0) { 
      // state after if
      var_desc* var = &method->vars[right_op->index];
      if (var->min >= 0) var->min = -1;
      if (var->max >= 0) var->max = -1;
      if (vars != NULL) { // forward branch
        // state at the branch address
        var = &vars[right_op->index];
        if (var->min < 0) var->min = 0;
        if (var->max < 0) var->max = 0;
        if (IS_INT_TYPE(var->type)) { 
          var->mask &= ~SIGN_BIT;
        }
      }
    }
    stack_pointer = right_op;
    break;      
  case ifgt:
    if (right_op->index >= 0) { 
      // state after if
      var_desc* var = &method->vars[right_op->index];
      if (var->min > 0) var->min = 0;
      if (var->max > 0) var->max = 0;
      if (vars != NULL) { // forward branch
        // state at the branch address
        var = &vars[right_op->index];
        if (var->min <= 0) var->min = 1;
        if (var->max <= 0) var->max = 1;
        if (IS_INT_TYPE(var->type)) { 
          var->mask &= ~SIGN_BIT;
        }
      }
    }
    stack_pointer = right_op;
    break;      
  case ifle:
    if (right_op->index >= 0) { 
      // state after if
      var_desc* var = &method->vars[right_op->index];
      if (var->min <= 0) var->min = 1;
      if (var->max <= 0) var->max = 1;
      if (IS_INT_TYPE(var->type)) { 
        var->mask &= ~SIGN_BIT;
      }
      if (vars != NULL) { // forward branch
        // state at the branch address
        var = &vars[right_op->index];
        if (var->min > 0) var->min = 0;
        if (var->max > 0) var->max = 0;
      }
    }
    stack_pointer = right_op;
    break;      
  case if_icmpeq:
    if (vars != NULL) { 
      // state at the branch address
      if (right_op->index >= 0) { 
        var_desc* var = &vars[right_op->index];
        if (var->min < left_op->min) var->min = left_op->min;
        if (var->max > left_op->max) var->max = left_op->max;
        if (var->min > var->max) var->min = var->max; // recovery
        if (IS_INT_TYPE(var->type)) { 
          var->mask &= left_op->mask;
        }
      }
      if (left_op->index >= 0) { 
        var_desc* var = &vars[left_op->index];
        if (var->min < right_op->min) var->min = right_op->min;
        if (var->max > right_op->max) var->max = right_op->max;
        if (var->min > var->max) var->min = var->max; // recovery
        if (IS_INT_TYPE(var->type)) { 
          var->mask &= right_op->mask;
        }
      }
    }
    stack_pointer = left_op;
    break;      
  case if_icmpne:
    if (right_op->index >= 0) { 
      // state after if
      var_desc* var = &method->vars[right_op->index];
      if (var->min < left_op->min) var->min = left_op->min;
      if (var->max > left_op->max) var->max = left_op->max;
      if (var->min > var->max) var->min = var->max; // recovery
      if (IS_INT_TYPE(var->type)) { 
        var->mask &= left_op->mask;
      }
    }
    if (left_op->index >= 0) { 
      var_desc* var = &method->vars[left_op->index];
      if (var->min < right_op->min) var->min = right_op->min;
      if (var->max > right_op->max) var->max = right_op->max;
      if (var->min > var->max) var->min = var->max; // recovery
      if (IS_INT_TYPE(var->type)) { 
        var->mask &= right_op->mask;
      }
    }
    stack_pointer = left_op;
    break;      
  case if_icmplt:
    if (right_op->index >= 0) {   
      // left >= right
      var_desc* var = &method->vars[right_op->index];
      if (var->max > left_op->max) { 
        var->max = left_op->max;
        if (var->min > var->max) var->min = var->max;
      }
      if (vars != NULL) { 
        // left < right
        var = &vars[right_op->index];
        if (var->min <= left_op->min) { 
          var->min = left_op->min == ranges[tp_int].max 
            ? left_op->min : left_op->min+1;
          if (var->min > var->max) var->max = var->min;
        }
      }
    }
    if (left_op->index >= 0) { 
      // left >= right
      var_desc* var = &method->vars[left_op->index];
      if (var->min < right_op->min) { 
        var->min = right_op->min;
        if (var->min > var->max) var->max = var->min;
      }
      if (vars != NULL) { 
        // left < right
        var = &vars[left_op->index];
        if (var->max >= right_op->max) { 
          var->max = right_op->max == ranges[tp_int].min 
            ? right_op->max : right_op->max-1;
          if (var->min > var->max) var->min = var->max;
        }
      }
    }
    stack_pointer = left_op;  
    break;
  case if_icmple:
    if (right_op->index >= 0) {   
      // left > right
      var_desc* var = &method->vars[right_op->index];
      if (var->max >= left_op->max) { 
        var->max = left_op->max == ranges[tp_int].min
          ? left_op->max : left_op->max-1;
        if (var->min > var->max) var->min = var->max;
      }
      if (vars != NULL) { 
        // left <= right
        var = &vars[right_op->index];
        if (var->min < left_op->min) { 
          var->min = left_op->min;
          if (var->min > var->max) var->max = var->min;
        }
      }
    }
    if (left_op->index >= 0) { 
      // left > right
      var_desc* var = &method->vars[left_op->index];
      if (var->min <= right_op->min) { 
        var->min = right_op->min == ranges[tp_int].max
          ? right_op->min : right_op->min+1;
        if (var->min > var->max) var->max = var->min;
      }
      if (vars != NULL) { 
        // left <= right
        var = &vars[left_op->index];
        if (var->max > right_op->max) { 
          var->max = right_op->max;
          if (var->min > var->max) var->min = var->max;
        }
      }
    }
    stack_pointer = left_op;  
    break;
  case if_icmpgt:
    if (right_op->index >= 0) {   
      // left <= right
      var_desc* var = &method->vars[right_op->index];
      if (var->min < left_op->min) { 
        var->min = left_op->min;
        if (var->min > var->max) var->max = var->min;
      }
      if (vars != NULL) { 
        // left > right
        var = &vars[right_op->index];
        if (var->max >= left_op->max) { 
          var->max = left_op->max == ranges[tp_int].min 
            ? left_op->max : left_op->max-1;
          if (var->min > var->max) var->min = var->max;
        }
      }
    }
    if (left_op->index >= 0) { 
      // left <= right
      var_desc* var = &method->vars[left_op->index];
      if (var->max > right_op->max) { 
        var->max = right_op->max;
        if (var->min > var->max) var->min = var->max;
      }
      if (vars != NULL) { 
        // left > right
        var = &vars[left_op->index];
        if (var->min <= right_op->min) { 
          var->min = right_op->min == ranges[tp_int].max 
            ? right_op->min : right_op->min+1;
          if (var->min > var->max) var->max = var->min;
        }
      }
    }
    stack_pointer = left_op;  
    break;
  case if_icmpge:
    if (right_op->index >= 0) {   
      // left < right
      var_desc* var = &method->vars[right_op->index];
      if (var->min <= left_op->min) { 
        var->min = left_op->min == ranges[tp_int].max
          ? left_op->min : left_op->min+1;
        if (var->min > var->max) var->max = var->min;
      }
      if (vars != NULL) { 
        // left >= right
        var = &vars[right_op->index];
        if (var->max > left_op->max) { 
          var->max = left_op->max;
          if (var->min > var->max) var->min = var->max;
        }
      }
    }
    if (left_op->index >= 0) { 
      // left < right
      var_desc* var = &method->vars[left_op->index];
      if (var->max >= right_op->max) { 
        var->max = right_op->max == ranges[tp_int].min
          ? right_op->max : right_op->max-1;
        if (var->min > var->max) var->min = var->max;
      }
      if (vars != NULL) { 
        // left >= right
        var = &vars[left_op->index];
        if (var->min < right_op->min) { 
          var->min = right_op->min;
          if (var->min > var->max) var->max = var->min;
        }
      }
    }
    stack_pointer = left_op;  
    break;
  case if_acmpeq:
    if (vars != NULL) { 
      if (right_op->index >= 0) { 
        vars[right_op->index].mask &= left_op->mask;
      } 
      if (left_op->index >= 0) { 
        vars[left_op->index].mask &= right_op->mask;
      } 
    }
    stack_pointer = left_op;
    break;
  case if_acmpne:
    if (right_op->index >= 0) { 
      method->vars[right_op->index].mask &= left_op->mask;
    } 
    if (left_op->index >= 0) { 
      method->vars[left_op->index].mask &= right_op->mask;
    } 
    stack_pointer = left_op;
    break;
  case ifnull:
    if (right_op->index >= 0) { 
      method->vars[right_op->index].mask |= var_desc::vs_not_null;
      if (vars != NULL) { 
        vars[right_op->index].mask &= ~var_desc::vs_not_null;
      }
    }
    stack_pointer = right_op;
    break;
  case ifnonnull:
    if (right_op->index >= 0) { 
      method->vars[right_op->index].mask &= ~var_desc::vs_not_null;
      if (vars != NULL) { 
        vars[right_op->index].mask |= var_desc::vs_not_null;
      }
    }
    stack_pointer = right_op;
    break;
  default:
    stack_pointer = sp;
  }
  stack_top[1] = stack_pointer[-1];
  stack_top[0] = stack_pointer[-2];
  return sp;
}

/* 
** merge states after split
*/ 

vbm_operand* ctx_merge::transfer(method_desc* method, vbm_operand* sp, 
                                 byte, byte& prev_cop)
{
  var_desc save_var;
  method->in_monitor = come_from->in_monitor;
  if (cmd == cmd_case_ctx && come_from->switch_var_index >= 0) { 
    // If branch is part of switch and switch expression is local variable,
    // then we know value of this variable if this branch takes place
    var_desc* var = &come_from->vars[come_from->switch_var_index];
    save_var = *var;
    var->max = var->min = var->mask = case_value;
  }
  var_desc* v0 = method->vars;
  var_desc* v1 = come_from->vars;
  if (prev_cop == goto_near || prev_cop == goto_w 
      || prev_cop == ret || prev_cop == athrow 
      || prev_cop == lookupswitch || prev_cop == tableswitch
      || unsigned(prev_cop - ireturn) <= unsigned(vreturn-ireturn))
    {
      // Control can be passed to this point only by branch: 
      // no need to merge states
      for (int i = method->n_vars; --i >= 0; v0++, v1++) { 
        if (v0->type == v1->type) { 
          v0->min = v1->min;
          v0->max = v1->max;
          v0->mask = v1->mask;
        } else if (v1->type == tp_void) { 
          v0->type = tp_void;
        }
      }
      sp = come_from->stack_pointer;
      sp[-1] = come_from->stack_top[1];
      sp[-2] = come_from->stack_top[0];
      // all successive ctx_merge::transfer should merge variables properties
      prev_cop = nop; 
    } else { 
      // merge states
      for (int i = method->n_vars; --i >= 0; v0++, v1++) { 
        if (v0->type == v1->type) { 
          if (IS_INT_TYPE(v0->type)) { 
            if (v0->min > v1->min) v0->min = v1->min;
            if (v0->max < v1->max) v0->max = v1->max;
            v0->mask |= v1->mask;
#ifdef INT8_DEFINED
          } else if (v0->type == tp_long) { 
            int8 min0  = LOAD_INT8(v0,min); 
            int8 max0  = LOAD_INT8(v0,max); 
            int8 mask0 = LOAD_INT8(v0,mask); 
            int8 min1  = LOAD_INT8(v1,min); 
            int8 max1  = LOAD_INT8(v1,max); 
            int8 mask1 = LOAD_INT8(v1,mask); 
            if (min0 > min1) { 
              STORE_INT8(v0, min, min1);
            } 
            if (max0 < max1) { 
              STORE_INT8(v0, max, max1); 
            }
            mask0 |= mask1;
            STORE_INT8(v0, mask, mask0);
            v0 += 1; 
            v1 += 1; 
            i -= 1; 
            assert(i >= 0);
#endif
          } else { 
            if (v0->min > v1->min) v0->min = v1->min;
            if (v0->max < v1->max) v0->max = v1->max;
            v0->mask &= v1->mask;
          }
        } else if (v0->type != tp_void && v1->type == tp_void) { 
          if (IS_INT_TYPE(v0->type)) { 
            v0->min = ranges[tp_int].min;
            v0->max = ranges[tp_int].max;
            v0->mask = ALL_BITS;
          } else if (v0->type == tp_long) { 
            v0[0].min  = 0x80000000;
            v0[0].max  = 0x7fffffff;
            v0[0].mask = 0xffffffff;
            v0[1].min  = 0x00000000;
            v0[1].max  = 0xffffffff;
            v0[1].mask = 0xffffffff;
          } else { 
            v0->min = 0;
            v0->max = MAX_ARRAY_LENGTH;
          }
        }
      }
      assert(sp == come_from->stack_pointer);
      
      if (IS_INT_TYPE(come_from->stack_top[1].type)) { 
	if (sp[-1].min > come_from->stack_top[1].min) { 
          sp[-1].min = come_from->stack_top[1].min;
        } 
        if (sp[-1].max < come_from->stack_top[1].max) { 
          sp[-1].max = come_from->stack_top[1].max;
        }
        sp[-1].mask |= come_from->stack_top[1].mask;
#ifdef INT8_DEFINED
      } else if (come_from->stack_top[1].type == tp_long) { 
        int8 min0  = LOAD_INT8(sp-2,min); 
        int8 max0  = LOAD_INT8(sp-2,max); 
        int8 mask0 = LOAD_INT8(sp-2,mask); 
        int8 min1  = LOAD_INT8(come_from->stack_top,min); 
        int8 max1  = LOAD_INT8(come_from->stack_top,max); 
        int8 mask1 = LOAD_INT8(come_from->stack_top,mask); 
        if (min0 > min1) {
          STORE_INT8(sp-2, min, min1);
        } 
        if (max0 < max1) { 
          STORE_INT8(sp-2, max, max1); 
        }
        mask0 |= mask1;
        STORE_INT8(sp-2, mask, mask0);      
#endif
      } else {
	//raphy
	sp[-1].min = 0;
	sp[-1].max = 0;
	come_from->stack_top[1].min = 0;
	come_from->stack_top[1].max = 0;
	//raphy end
        if (sp[-1].min > come_from->stack_top[1].min) { 
          sp[-1].min = come_from->stack_top[1].min;
        } 
        if (sp[-1].max < come_from->stack_top[1].max) { 
          sp[-1].max = come_from->stack_top[1].max;
        }
        sp[-1].mask &= come_from->stack_top[1].mask;
      }
    }
  if (--come_from->n_branches == 0) { 
    delete[] come_from->vars;
  } else if (cmd == cmd_case_ctx && come_from->switch_var_index >= 0) { 
    // restore state of switch expression varaible, 
    // because it can be used in other branches
    come_from->vars[come_from->switch_var_index] = save_var;
  }
  return sp;
}

vbm_operand* ctx_entry_point::transfer(method_desc* method, vbm_operand* sp, 
                                       byte, byte&)
{
#if 1
  //
  // As far as state of variable is not followed correctly in case of 
  // subroutine execution or catching exception, the obvious approach is
  // to reset state of all local variables. But in this case we will loose
  // useful information, so I decide to keep variables state, 
  // hoping that it will not cause confusing Jlint messages.
  //
  var_desc* var = method->vars;
  for (int i = method->n_vars; --i >= 0; var++) { 
    int type = var->type;
    if (IS_INT_TYPE(type)) { 
      var->min  = ranges[type].min;
      var->max  = ranges[type].max;
      var->mask = var->min | var->max;
    } else if (type == tp_long) { 
      var->min  = 0x80000000;
      var->max  = 0x7fffffff;
      var->mask = 0xffffffff;
      var += 1;
      var->min  = 0x00000000;
      var->max  = 0xffffffff;
      var->mask = 0xffffffff;
      i -= 1;
      assert(i >= 0);
    } else { 
      var->min  = 0;
      var->max  = MAX_ARRAY_LENGTH;
      var->mask = var_desc::vs_unknown;
    }
  }
#endif
  sp->type = tp_object;
  sp->mask = var_desc::vs_not_null;
  sp->min = 0;
  sp->max = MAX_ARRAY_LENGTH;
  return sp+1; // exception object is pushed on stack
}
   

vbm_operand* ctx_reset::transfer(method_desc* method, vbm_operand* sp, 
                                 byte, byte&)
{
  var_desc* var = method->vars;
  for (int n = method->n_vars, i = 0; i < n; i++, var++) { 
    //
    // Reset vaules of local variables which were modified in region of
    // code between backward jump label and backward jump intruction
    //
    if (method->var_store_count[i] != var_store_count[i]) { 
      int type = var->type;
      if (IS_INT_TYPE(type)) { 
        var->min  = ranges[type].min;
        var->max  = ranges[type].max;
        var->mask = var->min | var->max;
      } else if (type == tp_long) { 
        var->min  = 0x80000000;
        var->max  = 0x7fffffff;
        var->mask = 0xffffffff;
        var += 1;
        var->min  = 0x00000000;
        var->max  = 0xffffffff;
        var->mask = 0xffffffff;
        i += 1;
        assert(i < n);
      } else { 
        var->mask = var_desc::vs_unknown;
        var->min  = 0;
        var->max  = MAX_ARRAY_LENGTH;
      }
    }
  }
  delete[] var_store_count;
  return sp;
}
