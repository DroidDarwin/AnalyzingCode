#include "graph.hh"

void graph_edge::message(int loop_id)
{
    caller->print_call_path_to(invocation, loop_id, 0);
    class_desc* abstract_class = invocation->method->cls; 
    invocation->method->cls = vertex->cls;
    if (!strcmp(invocation->method->name.as_asciz(), "<synch>")) {
      // special error message about synchronized blocks
      if (invocation->method->attr & method_desc::m_sync_block) {
        const char* lock1 = invocation->self_class->name.as_asciz();
        // pretty print lock set
        string lockset = "";
        monitor_stack::const_iterator entry = caller->locksAtEntry.begin();
        while (entry != caller->locksAtEntry.end()) {
          if ((*entry)->cls == NULL)
            break;
          const char* full_name =
            compound_name((*entry)->cls->name.as_asciz(),
                          (*entry)->name.as_asciz());
          if (strcmp(full_name, lock1)) {
            lockset += string(", ");
            if (strcmp(full_name, "")) {
              lockset += full_name;
            } else {
              lockset += "<unknown>";
            }
          }
          entry++;
        }
        if (caller->locksAtEntry.nLocks() > 2) {
          lockset += "}";
          lockset.replace(0, 2, "set {");
        } else {
          lockset.replace(0, 2, "");
        }
        if (lockset == "") {
          lockset = "<unknown>";
        }
        invocation->message(msg_lock, lock1, lockset.c_str());
      } else {
        invocation->message(msg_lock, invocation->self_class->name.as_asciz(), 
                            caller->cls->name.as_asciz());
      }
    } else {
      if (!(invocation->method->attr & method_desc::m_sync_block)) {
        invocation->message(msg_sync_loop, (void*)loop_id, invocation->method);
      }
    }
    invocation->method->cls = abstract_class;
    mask |= (1 << (loop_id & 31));
}

void graph_vertex::verify()
{
  graph_edge** stack = new graph_edge*[n_vertexes];
  int marker = 0;
  int n_loops = 0;
    
  for (graph_vertex* root = graph; root != NULL; root = root->next) { 
    if (root->marker <= marker || root->visited >= max_shown_paths) { 
      continue;
    }
    int sp = 0;
    int i;
  
    graph_edge* edge = root->edges; 
    root->visited |= flag_vertex_on_path;
    root->marker = ++marker;

    while (edge != NULL) {
      while (edge->vertex->marker >= marker || 
             edge->vertex->visited < max_shown_paths) 
        {
          graph_vertex* vertex = edge->vertex;
          stack[sp++] = edge;

          if (vertex->visited & flag_vertex_on_path) { 
            // we found loop in the graph
            unsigned mask = edge->mask;
            for (i = sp-1; --i >= 0 && stack[i]->vertex != vertex;) { 
              mask &= stack[i]->mask;
            }
            if (mask == 0) { 
              n_loops += 1;
              while (++i < sp) { 
                stack[i]->message(n_loops);
              }
            }
          } else { 
            // Pass through the graph in depth first order
            if (vertex->edges != NULL) { 
              vertex->visited = 
                (vertex->visited + 1) | flag_vertex_on_path;
              vertex->marker = marker;
              vertex->n_loops = n_loops;
              edge = vertex->edges;
              continue;
            }
          }
          sp -= 1;
          break;
        } // end of depth-first loop

      while (edge->next == NULL) { 
        if (--sp < 0) break;
        edge = stack[sp];
        edge->vertex->visited &= ~flag_vertex_on_path;
        if (edge->vertex->n_loops == n_loops) { 
          // This path doesn't lead to deadlock: avoid future
          // visits of this vertex
          edge->vertex->marker = 0;
        }
      }
      edge = edge->next;
    }
    root->visited &= ~flag_vertex_on_path;
  }
}

