#include "message_node.hh"

message_node* message_node::hash_table[1023];

bool message_node::find(char* msg_text) 
{
    unsigned h = string_hash_function((byte*)msg_text) % items(hash_table);
    for (message_node* msg = hash_table[h]; msg != NULL; msg = msg->next) { 
      if (strcmp(msg->text, msg_text) == 0) { 
        return true;
      }
    }
    return false;
}

void message_node::add_to_hash(char* msg_text)
{
    unsigned h = string_hash_function((byte*)msg_text) % items(hash_table);
    message_node* msg = new message_node(msg_text);
    msg->next = hash_table[h];
    hash_table[h] = msg;
} 

