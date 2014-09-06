#ifndef GRAPH_HH
#define GRAPH_HH

#include "types.hh"
#include "method_desc.hh"

class callee_desc;
class graph_vertex;

class graph_edge { 
public:
  graph_edge*   next;
  callee_desc*  invocation;
  method_desc*  caller;
  graph_vertex* vertex;
  int           mask;

  void message(int loop_id);

  graph_edge(graph_vertex* node, method_desc* method, callee_desc* call) { 
    invocation = call;
    caller = method;
    vertex = node;
    mask = 0;
  }
};

class graph_vertex { 
public:
  graph_edge*   edges;
  graph_vertex* next;
  class_desc*   cls;
  int           visited;
  int           marker;
  int           n_loops;
  enum { 
    flag_vertex_on_path    = 0x80000000,
    flag_vertex_not_marked = 0x7fffffff
  };

  static int n_vertexes;

  static graph_vertex* graph;
    
  static void verify();

  void attach(graph_edge* edge) { 
    edge->next = edges;
    edges = edge;
  }

  graph_vertex(class_desc* vertex_class) { 
    cls = vertex_class;
    visited = 0;
    marker = flag_vertex_not_marked;
    next = graph;
    graph = this;
    edges = NULL;
    n_vertexes += 1;
  }
};

#endif
