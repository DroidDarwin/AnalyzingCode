#ifndef MESSAGE_NODE_HH
#define MESSAGE_NODE_HH
//#include <string>
//#include "types.hh" // included by functions.hh
#include "functions.hh"
#include <cstring>

class message_node {
  static message_node* hash_table[];
public:
  message_node* next;
  char*         text;

  static bool find(char* msg);
  static void add_to_hash(char* msg);

  message_node(char* msg) { 
    text = strdup(msg);
    next = NULL;
  }
  ~message_node() { delete[] text; }
};

#endif
