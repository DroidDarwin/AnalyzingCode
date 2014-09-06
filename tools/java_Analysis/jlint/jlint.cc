//-< JLINT.CC >------------------------------------------------------+--------+
// Jlint                      Version 3.1        (c) 1998  GARRET    |     ?  |
// (Java Lint)                                                       |   /\|  |
//                                                                   |  /  \  |
//   Created:                    28-Mar-98    K.A. Knizhnik          | / [] \ |
//   Version 2.X:   Last update: 05-Jun-01    Cyrille Artho          | GARRET |
//   Version 3.0:   Last update: 20-Aug-03    Raphael Ackermann      |        |
//   Version 3.1:   Last update: 13-Oct-06    Cyrille Artho          |        |
//-------------------------------------------------------------------+--------+
// Java verifier 
//-------------------------------------------------------------------+--------+

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

#include "jlint.hh"

extern "C"
{
#include <zlib.h>        // should I use <zlib.h> or "zlib.h" ??
  // #include "zlib.h"
};

bool   verbose = false;
int   max_shown_paths = 4;

int   n_messages;

char* source_file_path;
int   source_file_path_len;
bool  source_path_redefined = false;
int   reported_message_mask = cat_all;
FILE* history;
string_pool stringPool;
field_desc* is_const;

message_descriptor msg_table[] = 
  {
#define MSG(category, code, position_dependent, format) \
   {cat_##category, MSG_LOCATION_PREFIX format, #code, position_dependent, true},
#include "jlint.msg"
    {cat_all}
  };

unsigned int string_hash_function(byte* p) { 
  unsigned int h = 0, g;
  while(*p) { 
    h = (h << 4) + *p++;
    if ((g = h & 0xF0000000) != 0) { 
      h ^= g >> 24;
    }
    h &= ~g;
  }
  return h;
}

//
// Functions for extracting information from type descriptors
//

void message_at(int code, utf_string const& file, int line, ...)
{
  va_list ap;
  va_start(ap, line);
  format_message(code, file, line, ap);
  va_end(ap);
}

int get_number_of_parameters(utf_string const& str)
{
  const char* p = str.as_asciz();
  assert(*p++ =='(');
  int n_params = 0;
  while (*p != ')') { 
    switch (*p++) { 
    case '[':
      while (*p == '[') p += 1;
      if (*p++ == 'L') { 
        while (*p++ != ';');
      }
      n_params += 1;
      break;
    case 'D':
    case 'J':
      n_params += 2;
      break;
    case 'L':
      while (*p++ != ';');
      nobreak;
    default:
      n_params += 1;
    }
  }
  return n_params;
} 

int get_type(utf_string const& str) 
{ 
  const char* p = str.as_asciz();
  if (*p == '(') { 
    while (*++p != ')'); 
    p += 1;
  }
  int indirect = 0;
  while (*p == '[') { 
    p += 1;
    indirect += 1;
  }
  type_tag tag = tp_object;
  switch (*p) { 
  case 'I': tag = tp_int; break;
  case 'S': tag = tp_short; break;
  case 'D': tag = tp_double; break;
  case 'J': tag = tp_long; break;
  case 'F': tag = tp_float; break;
  case 'V': tag = tp_void; break;
  case 'B': tag = tp_byte; break;
  case 'C': tag = tp_char; break;
  case 'Z': tag = tp_bool; break;
  default:
    if (strcmp(p, "Ljava/lang/String;") == 0) tag = tp_string;
  }
  return int(tag) + (indirect << 8);
}

//
// All messages are reported by this function
// 

void format_message(int code, utf_string const& file, int line, __VALIST ap)
{
  static int loop_id;
  static message_node *first, *last;
  static char* compound_message;
  const void* parameter[MAX_MSG_PARAMETERS];
  int   n_parameters = 2;
  
  if (code == msg_loop || code == msg_sync_loop) { // extract loop identifier
    parameter[n_parameters++] = va_arg(ap, void*);
  }
  if (history != NULL) { 
    if (compound_message != NULL
        && ((loop_id != 0 
             && ((code != msg_loop && code != msg_sync_loop)
                 || (int)(long)parameter[2] != loop_id))
            || (loop_id == 0 && code != msg_wait_path)))
      {
        if (!message_node::find(compound_message)) { 
          message_node *msg = first, *next;
          do { 
            next = msg->next;
            fprintf(stdout, "%s\n", msg->text);
            delete msg;
            n_messages += 1;
          } while ((msg = next) != NULL);
          fprintf(history, "%s\n", compound_message);  
        }
        delete[] compound_message;
        compound_message = NULL;
      }
  }
  if ((reported_message_mask & msg_table[code].category) 
      && msg_table[code].enabled)
    {
      static int prev_line = 0;
  
      char  msg_buf[MAX_MSG_LENGTH];
      char  his_buf[MAX_MSG_LENGTH];
      char* hp = his_buf;
  
      if (line == 0) { 
        line = ++prev_line; // avoid several messages with 0 line number
      } else { 
        prev_line = 0;
      }
      parameter[0] = file.as_asciz();
      parameter[1] = (void*)line;

      if (history) { 
        hp += sprintf(hp, "%s", msg_table[code].name);
      }
      
      char const* src = msg_table[code].format;
      char* dst = msg_buf;

      if (code == msg_done) { 
        // do not output location prefix for termination message
        src += strlen(MSG_LOCATION_PREFIX);
        parameter[0] = (void*)n_messages;
      }
      while (*src != 0) { 
        if (*src == '%') { 
          int index;
          int pos;
          int n = sscanf(++src, "%d%n", &index, &pos); 
          assert(n == 1);
          assert(index < MAX_MSG_PARAMETERS);
          while (index >= n_parameters) { 
            parameter[n_parameters++] = va_arg(ap, void*);
          }
          src += pos;
          char* save_dst = dst;
          switch (*src++) {
          case 's': // zero terminated string
            dst += sprintf(dst, "%s", (char*)parameter[index]);  
            break;
          case 'm': // method descriptor
            dst += ((method_desc*)parameter[index])->
              demangle_method_name(dst);
            break;
          case 'u': // utf8 string
            dst += sprintf(dst, "%s", 
                           ((utf_string*)parameter[index])->as_asciz()); 
            break;
          case 'c': // class descriptor
            dst += sprintf(dst, "%s", ((class_desc*)parameter[index])->
                           name.as_asciz()); 
            break;
          case 'd': // integer
            dst += sprintf(dst, "%d", (int)(long)parameter[index]);  
            break;
          default:
            assert(false/*bad message parameter format*/);
          }
          if (history) { 
            // append parameeter to history buffer
            if ((index >= 2 || msg_table[code].position_dependent)
                && (code != msg_sync_loop || index > 2) 
                && (code != msg_loop || index > 3))
              {
                // Do not inlude loop number in history message
                hp += sprintf(hp, ":%.*s", (int)(dst - save_dst), save_dst);
              }
          }
        } else { 
          *dst++ = *src++;
        }
      }
      *dst++ = '.';
      *dst = 0;
      if (history != NULL) { 
        if (compound_message != NULL) { 
          char* new_msg = new char[strlen(compound_message)
                                  +strlen(his_buf)+2];
          sprintf(new_msg, "%s;%s", compound_message, his_buf);
          last = last->next = new message_node(msg_buf);
          delete[] compound_message;
          compound_message = new_msg;
        } else { 
          if (code == msg_loop || code == msg_sync_loop 
              || code == msg_wait) 
            { 
              compound_message = strdup(his_buf);
              first = last = new message_node(msg_buf);
              if (code != msg_wait) { 
                loop_id = (int)(long)parameter[2];
              }
            } else if (!message_node::find(his_buf)) { 
              fprintf(stdout, "%s\n", msg_buf);
              if (code != msg_done) { 
                fprintf(history, "%s\n", his_buf);
                n_messages += 1;
              }
            }
        }
      } else { 
        fprintf(stdout, "%s\n", msg_buf);
        if (code != msg_done) { 
          n_messages += 1;
        }
      }
    } 
}

graph_vertex* graph_vertex::graph;
int graph_vertex::n_vertexes;

//
// Methods of class_desc class
//

class_desc* class_desc::hash_table[class_hash_table_size];
class_desc* class_desc::chain;
int class_desc::n_classes;

void set_class_source_path(class_desc* cls)
{
  if (source_file_path_len != 0) { 
    const char* class_file_name = cls->source_file.as_asciz();
    if (!source_path_redefined) { 
      const char* dirend = strrchr(class_file_name, '/');
      if (dirend != NULL) { 
        int dirlen = dirend - class_file_name;
        if (dirlen <= source_file_path_len
            && memcmp(class_file_name, 
                      source_file_path + source_file_path_len - dirlen,
                      dirlen) == 0)
          {
            class_file_name = dirend+1;
          }
      }
    }
    char file_name_buf[MAX_MSG_LENGTH];
    int len = sprintf(file_name_buf, "%.*s%c%s", 
                      source_file_path_len, source_file_path, 
                      FILE_SEP, class_file_name);
    cls->source_file = utf_string(len, (byte*)file_name_buf);
  } 
}

bool parse_class_file(byte* fp)
{
  int i;
  unsigned magic = unpack4(fp); 
  fp += 4;
  if (magic != 0xCAFEBABE) return false;
  int minor_version = unpack2(fp);
  fp += 2;
  int major_version = unpack2(fp);
  fp += 2;
  int constant_pool_count = unpack2(fp);
  fp += 2;
  constant** constant_pool = new constant*[constant_pool_count];
  memset(constant_pool, 0, sizeof(constant*)*constant_pool_count);

  int name_index = 0;
  for (i = 1; i < constant_pool_count; i++) { 
    constant* cp = NULL;
    int n_extra_cells = 0;
    switch (*fp) { 
    case c_utf8:
      cp = new const_utf8(fp);
      if (!strcmp(((const_utf8*)cp)->as_asciz(), "this")) {
        name_index = i;
      }
      break;
    case c_integer:
      cp = new const_int(fp);
      break;
    case c_float:
      cp = new const_float(fp);
      break;
    case c_long:
      cp = new const_long(fp);
      n_extra_cells += 1;
      break;
    case c_double:
      cp = new const_double(fp);
      n_extra_cells += 1;
      break;
    case c_class:
      cp = new const_class(fp);
      break;
    case c_string:
      cp = new const_string(fp);
      break;
    case c_field_ref:
    case c_method_ref:
    case c_interface_method_ref:
      cp = new const_ref(fp);
      break;
    case c_name_and_type:
      cp = new const_name_and_type(fp);
      break;
    }
    assert(cp != NULL);
    fp += cp->length();
    constant_pool[i] = cp;
    i += n_extra_cells;
  }      
  int access_flags = unpack2(fp);
  fp += 2;
  int this_class_name = unpack2(fp);
  fp += 2;
  int super_class_name = unpack2(fp);
  fp += 2;
  int interfaces_count = unpack2(fp);
  fp += 2;
    
  class_desc* this_class = 
    class_desc::get(*(const_utf8*)constant_pool
                    [((const_class*)constant_pool[this_class_name])->name]);

  set_class_source_path(this_class);

  // init. is_this
  field_desc* is_this = new field_desc(utf_string("<this>"), this_class, NULL);
  // assign name_and_type for is_this - find name entry "this" and obj. type
  is_this->name_and_type =
    new const_name_and_type(name_index,
                            ((const_class*)constant_pool[this_class_name])->name);
  is_this->equals = is_this;
  is_this->cls = this_class;

  // init. is_const
  field_desc* is_const = new field_desc(utf_string("<const>"), NULL, NULL);

  this_class->attr = access_flags;
  if (super_class_name == 0) { // Object class
    assert(interfaces_count == 0); 
    this_class->n_bases = 0;
  } else { 
    this_class->n_bases = interfaces_count + 1;
    this_class->bases = new class_desc*[interfaces_count + 1];
    this_class->bases[0] =
      class_desc::get(*(const_utf8*)constant_pool
                      [((const_class*)constant_pool[super_class_name])->name]);
    
    for (i = 1; i <= interfaces_count; i++) { 
      int interface_name = unpack2(fp);
      fp += 2;
      this_class->bases[i] = 
        class_desc::get(*(const_utf8*)constant_pool
                        [((const_class*)constant_pool[interface_name])->name]);
    }
  }
  int fields_count = unpack2(fp);
  fp += 2;

  while (--fields_count >= 0) {
    int access_flags = unpack2(fp); fp += 2;
    int name_index = unpack2(fp); fp += 2;
    int desc_index = unpack2(fp); fp += 2;
    int attr_count = unpack2(fp); fp += 2;
    field_desc* field = 
      this_class->get_field(*(const_utf8*)constant_pool[name_index]);
    field->attr |= access_flags;
    while (--attr_count >= 0) { 
      int attr_len = unpack4(fp+2); 
      fp += 6 + attr_len;
    }
  }
   
  int methods_count = unpack2(fp);
  fp += 2;
    
  //
  // We need "SourceFile" attribute first, so remember position and 
  // skip methods definitions
  //
  byte* method_part = fp;
  for (i = 0; i < methods_count; i++) { 
    int attr_count = unpack2(fp+6); 
    fp += 8;
    while (--attr_count >= 0) { 
      int attr_len = unpack4(fp+2); 
      fp += 6 + attr_len;
    }
  }
  int attr_count = unpack2(fp); fp += 2;
  while (--attr_count >= 0) { 
    int attr_name = unpack2(fp); fp += 2; 
    int attr_len = unpack4(fp); fp += 4;
    utf_string attr(*(const_utf8*)constant_pool[attr_name]);
    if (attr == "SourceFile") { 
      int source_name = unpack2(fp); fp += 2;
      int file_name_idx = this_class->source_file.rindex(FILE_SEP);
      utf_string* file_name = (const_utf8*)constant_pool[source_name];
      if (file_name_idx >= 0) { 
        this_class->source_file.append(file_name_idx+1, *file_name);
      } else { 
        this_class->source_file = *file_name;
      }
    } else { 
      fp += attr_len;
    }
  }
  fp = method_part;

  while (--methods_count >= 0) { 
    int access_flags = unpack2(fp); fp += 2;
    int name_index = unpack2(fp); fp += 2;
    int desc_index = unpack2(fp); fp += 2;
    int attr_count = unpack2(fp); fp += 2;
  
    const_utf8* mth_name = (const_utf8*)constant_pool[name_index];
    const_utf8* mth_desc = (const_utf8*)constant_pool[desc_index];

    method_desc* method = this_class->get_method(*mth_name, *mth_desc);
    method->attr |= access_flags;
    method->vars = NULL;
  
    while (--attr_count >= 0) { 
      int attr_name = unpack2(fp); fp += 2;
      int attr_len = unpack4(fp); fp += 4;
      utf_string attr(*(const_utf8*)constant_pool[attr_name]);
      if (attr == "Code") { 
        int max_stack = unpack2(fp); fp += 2;
        int max_locals = unpack2(fp); fp += 2;
        int code_length = unpack4(fp); fp += 4;
        method->code = fp;
        method->code_length = code_length;
        fp += code_length;

        method->n_vars = max_locals;
        method->vars = max_locals > 0 ? new var_desc[max_locals] : NULL;

        method->line_table = new word[code_length];
        memset(method->line_table, 0, sizeof(word)*code_length);

        method->context = new local_context*[code_length+1];
        memset(method->context, 0, 
               sizeof(local_context*)*(code_length+1));

        int exception_table_length = unpack2(fp); fp += 2;

	/* add new entry for each distinct "byte code adress of handle".
	**
	** if an exception handler at byte code "pos" handles exception of
	** more than one byte code range, call
	** "new ctx_entry_point(&method->context[pos]);" only once! Because
	** otherwise the stack gets out of control. 
	**
	** in the following example there are two different handle adresses
	** 16 and 25. and for each of them 
	** "new ctx_entry_point(&method->context[handler_pc]);" is called 
	** exactly once. Therefore the program calls :
	** new ctx_entry_point(&method->context[16]);
	** new ctx_entry_point(&method->context[25]);
	**********************************************************************
	** Example Exception Table:                                         **
	** -------------------------------------                            **
	**                                                                  **
	**                      byte code adress                            **
	** from         to        of handle                                 **
	**  2           10           16                                     **
	** 12           14           16                                     **
	** 20           23           25                                     **
	**********************************************************************
	**
	** it is expected that the byte code adresses of the handles are 
	** ordered. If this would not be the case, a simple comparison of
	** handler_pc and old_handler_pc would not be sufficient!
	*/

	int old_handler_pc = -1;

        while (--exception_table_length >= 0) { 
          int handler_pc = unpack2(fp+4);
	  if ( handler_pc != old_handler_pc) {
	    new ctx_entry_point(&method->context[handler_pc]); 
	  }
	  fp += 8;
	  old_handler_pc = handler_pc;
        }

        int method_attr_count = unpack2(fp); fp += 2;
        bool line_table_present = false;
        bool local_var_table_present = false;
        while (--method_attr_count >= 0) { 
          int mth_attr_name = unpack2(fp); fp += 2;
          int mth_attr_len = unpack4(fp); fp += 4;
          utf_string* attr = 
            (const_utf8*)constant_pool[mth_attr_name];
          if (*attr == "LineNumberTable") {
            int table_length = unpack2(fp); fp += 2;
            while (--table_length >= 0) { 
              int start_pc = unpack2(fp); fp += 2;
              int line_no = unpack2(fp); fp += 2;
              method->line_table[start_pc] = line_no;
            }
            method->first_line = method->line_table[0];
            if (method->first_line != 0) method->first_line -= 1;
            line_table_present = true;
          } else if (*attr == "LocalVariableTable") { 
            method->local_variable_table_present = true;
            int table_length = unpack2(fp); fp += 2;
            while (--table_length >= 0) { 
              int start_pc = unpack2(fp); fp += 2;
              int length = unpack2(fp); fp += 2;
              int name  = unpack2(fp); fp += 2;
              int desc  = unpack2(fp); fp += 2;
              int index = unpack2(fp); fp += 2;
              int type = (index == 0 && 
                          !(access_flags & method_desc::m_static))
                ? tp_self 
                : get_type(*(const_utf8*)constant_pool[desc]);
              new ctx_push_var(&method->context[start_pc], 
                               (const_utf8*)constant_pool[name],
                               type, index, start_pc);
              new ctx_pop_var(&method->context[start_pc+length],
                              index);
            }
            local_var_table_present = true;
          } else {
            fp += mth_attr_len;
          }
        }
        if (verbose && !(access_flags & method_desc::m_native)) { 
          char buf[MAX_MSG_LENGTH];
          method->demangle_method_name(buf);
          if (!line_table_present) { 
            fprintf(stderr, "No line table present "
                    "for method %s\n", buf);
          }
          if (!local_var_table_present) { 
            fprintf(stderr, "No local variable table present "
                    "for method %s\n", buf);
          }
        }
        method->parse_code(constant_pool, is_this);
      } else { 
        fp += attr_len;
      }
    }
  }

  for (i = 1; i < constant_pool_count; i++) {
    if (constant_pool[i] != NULL)
      delete constant_pool[i];
  }
  
  /*
  for (method_desc* m = this_class->methods; m != NULL; m = m->next) {
    m->locksAtEntry.clear();
  }
  */
  delete[] constant_pool;
  delete is_this->name_and_type;
  //delete is_this;
  delete is_const;

  monitor_stack::const_iterator it;
  for (it = this_class->usedLocks.begin();
       it != this_class->usedLocks.end(); it++) {
    if (!((*it)->writes.empty())) {
      vector<int>::const_iterator i;
      for (i = (*it)->writes.begin(); i != (*it)->writes.end(); i++) {
        message_at(msg_lock_assign, this_class->source_file, *i, *it);
        // *i: line number; *it = field_desc*
      }
    }
  }
  
  return true;
}

//
// Determine type of file and extract Java class description from the file
//

inline int stricmp(const char* p, const char* q) 
{
  do { 
    while (*p == '_') p += 1; // ignore '_'
    while (*q == '_') q += 1; // ignore '_'
    int diff = toupper(*(byte*)p) - toupper(*(byte*)q);
    if (diff != 0) {
      return diff;
    }
    q += 1;
  } while (*p++ != 0);
  return 0;
}

void proceed_file(char* file_name, bool recursive = false)
{
#ifdef _WIN32
  HANDLE dir;
  char dir_path[MAX_PATH];
  WIN32_FIND_DATA file_data;
  if (recursive != 0) { 
    sprintf(dir_path, "%s\\*", file_name);
    if ((dir=FindFirstFile(dir_path, &file_data)) != INVALID_HANDLE_VALUE) 
      { 
        file_name = dir_path; 
      }
  } else { 
    if (strcmp(file_name, "..") == 0 || strcmp(file_name, ".") == 0) { 
      proceed_file(file_name, 1);
      return;
    }
    if ((dir=FindFirstFile(file_name, &file_data)) == INVALID_HANDLE_VALUE)
      { 
        fprintf(stderr, "Failed to open file '%s'\n", file_name);
        return;
      }
  }
  if (dir != INVALID_HANDLE_VALUE) {
    do {
      if (!recursive || *file_data.cFileName != '.') { 
        char file_path[MAX_PATH];
        char* file_dir = strrchr(file_name, '\\');
        char* file_name_with_path;
        if (file_dir != NULL) { 
          int dir_len = file_dir - file_name + 1;
          memcpy(file_path, file_name, dir_len);
          strcpy(file_path+dir_len, file_data.cFileName);
          file_name_with_path = file_path;
        } else { 
          file_name_with_path = file_data.cFileName;
        }
        proceed_file(file_name_with_path, recursive+1);
      } 
    } while (FindNextFile(dir, &file_data));
    FindClose(dir);
    return;
  }
#else
  DIR* dir = opendir(file_name);
  if (dir != NULL) { 
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) { 
      if (*entry->d_name != '.') { 
        char full_name[MAX_PATH];
        sprintf(full_name, "%s/%s", file_name, entry->d_name);
        proceed_file(full_name, 2);
      }
    } 
    closedir(dir);
    return;
  }
#endif
  if (recursive >= 2) { 
    int len = strlen(file_name); 
    if (len < 6 || stricmp(file_name + len - 6, ".class") != 0) {
      return;
    }
  }
  	
  FILE* f = fopen(file_name, "rb");
  if (f == NULL) { 
    fprintf(stderr, "Failed to open file '%s'\n", file_name);
    return;
  }
  char* suf = file_name + strlen(file_name) - 4;
  if (suf > file_name && (stricmp(suf,".zip")==0 || stricmp(suf,".jar")==0)){
    // extract files from ZIP archive
    byte hdr[ECREC_SIZE+4];
    if (fseek(f, 0, SEEK_END) != 0
        || ftell(f) < ECREC_SIZE+4 
        || fseek(f, -(ECREC_SIZE+4), SEEK_CUR) != 0
        || fread(hdr, ECREC_SIZE+4, 1, f) != 1) 
      { 
        fprintf(stderr, "Bad format of ZIP file '%s'\n", file_name);
        return;
      }
    int count = unpack2_le(&hdr[TOTAL_ENTRIES_CENTRAL_DIR]);
    int dir_size = unpack4_le(&hdr[SIZE_CENTRAL_DIRECTORY]);
    byte* directory = new byte[dir_size];
    if (fseek(f, -(dir_size+ECREC_SIZE+4), SEEK_CUR) != 0
        || fread(directory, dir_size, 1, f) != 1)
      {
        fprintf(stderr, "Bad format of ZIP file '%s'\n", file_name);
        delete[] directory;
        return;
      }
    byte* dp = directory;

    while (--count >= 0) {
      int compression_method = unpack2_le(&dp[4+C_COMPRESSION_METHOD]);
      int compressed_size = unpack4_le(&dp[4+C_COMPRESSED_SIZE]);
      int uncompressed_size = unpack4_le(&dp[4+C_UNCOMPRESSED_SIZE]);
      int filename_length = unpack2_le(&dp[4+C_FILENAME_LENGTH]);
      int cextra_length = unpack2_le(&dp[4+C_EXTRA_FIELD_LENGTH]);

      if ((dp-directory)+filename_length+CREC_SIZE+4 > dir_size) { 
        fprintf(stderr, "Bad format of ZIP file '%s'\n", file_name);
        break;
      }
  	
      byte local_rec_buffer[LREC_SIZE+4];
      int local_header_offset = 
        unpack4_le(&dp[4+C_RELATIVE_OFFSET_LOCAL_HEADER]);

      if (fseek(f, local_header_offset, SEEK_SET) != 0
          || fread(local_rec_buffer, sizeof local_rec_buffer, 1, f) != 1
          || memcmp(&local_rec_buffer[1], LOCAL_HDR_SIG, 3) != 0)
        {
          fprintf(stderr, "Bad format of ZIP file '%s'\n", file_name);
          break;
        }
  	
      int file_start = local_header_offset + (LREC_SIZE+4) +
        unpack2_le(&local_rec_buffer[4+L_FILENAME_LENGTH]) +
        unpack2_le(&local_rec_buffer[4+L_EXTRA_FIELD_LENGTH]);

      int filename_offset = CREC_SIZE+4;

      if (compression_method == C_UNCOMPRESSED)
	{
          if (uncompressed_size != 0) { 
            byte* buffer = new byte[uncompressed_size];
            if (fseek(f, file_start, SEEK_SET) != 0 
                || fread(buffer, uncompressed_size, 1, f) != 1) 
              { 
                fprintf(stderr, "Failed to read ZIP file '%s'\n", 
                        file_name);
                return;
              } else { 
                if (verbose) { 
                  fprintf(stderr, "Verify file '%.*s'\n", 
                          filename_length, dp + filename_offset); 
                }
                if (!parse_class_file(buffer)) { 
                  fprintf(stderr, "File '%.*s' isn't correct Java class "
                          "file\n", filename_length, dp+filename_offset);
                }
              }
            delete[] buffer;
          }
      }
      else if (compression_method == C_DEFLATE) {

          if ((uncompressed_size != 0) && (compressed_size != 0)) { 
            byte* uncbuffer = new byte[uncompressed_size];
            byte* cbuffer = new byte[compressed_size];
            if (fseek(f, file_start, SEEK_SET) != 0 
                || fread(cbuffer, compressed_size, 1, f) != 1) 
              { 
                fprintf(stderr, "Failed to read ZIP file '%s'\n", 
                        file_name);
                return;
              } else { 
                z_stream c_stream;

                c_stream.zalloc = (alloc_func) 0;
                c_stream.zfree = (free_func)0;
                c_stream.opaque = (voidpf)0;

                int err = inflateInit2(&c_stream, -MAX_WBITS);
                if (err != Z_OK)
                {
                    fprintf(stderr, "Error %s in inflateInit while uncompressing %.*s.\n",
                        c_stream.msg, filename_length, dp+filename_offset);
                }
                else
                {
                    c_stream.next_in = cbuffer;
                    c_stream.next_out = uncbuffer;
                    c_stream.avail_in = compressed_size;
                    c_stream.avail_out = uncompressed_size;
                    c_stream.total_in = 0;
                    c_stream.total_out = 0;

                    while ((err=inflate(&c_stream, Z_SYNC_FLUSH)) == Z_OK);
                    if ((err != Z_STREAM_END) && (err != Z_OK))
                    {
                        fprintf(stderr, "Error %s in inflate while uncompressing %.*s.\n",
                            c_stream.msg, filename_length, dp+filename_offset);
                    }
                    else
                    {
                        inflateEnd(&c_stream);

                        if (verbose) { 
                            fprintf(stderr, "Verify file '%.*s'\n", 
                            filename_length, dp + filename_offset); 
                        }
                        if (!parse_class_file(uncbuffer)) { 
                            fprintf(stderr, "File '%.*s' isn't correct Java class "
                                "file\n", filename_length, dp+filename_offset);
                        }
                    }
                }
              }
            delete[] cbuffer;
            delete[] uncbuffer;
          }
      }
      dp += filename_offset + filename_length + cextra_length; 
    }
    delete[] directory;
  } else { 
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    byte* buffer = new byte[file_size];
    if (fread(buffer, file_size, 1, f) != 1) { 
      fprintf(stderr, "Failed to read file '%s'\n", file_name);
    } else { 
      if (verbose) { 
        fprintf(stderr, "Verify file '%s'\n", file_name); 
      }
      if (!parse_class_file(buffer)) { 
        fprintf(stderr, "File '%s' isn't correct Java class file\n", 
                file_name);
      }
    }
    delete[] buffer;
  }
  fclose(f);
}
    
//
// Main function: parse command line 
//

msg_select_category_option msg_category_option[] = {
  {cat_synchronization, "synchronization",
     "detect synchronizational problems"},
  {cat_deadlock, "deadlock", 
     "possible deadlock detection"},
  {cat_race_condition, "race_condition", 
     "possible race condition detection"},
  {cat_wait_nosync, "wait_nosync", 
     "wait or notify is called from non-synchronized method"},
  {cat_inheritance, "inheritance",
     "detect problems with class/interface inheritance"},
  {cat_super_finalize, "super_finalize", 
     "super.finalize() is not called from finalize method"},
  {cat_not_overridden, "not_overridden",
     "methods with the same names and different profiles"},
  {cat_field_redefined, "field_redefined",
     "instance or static variable is redefined in derived class"},
  {cat_shadow_local, "shadow_local",
     "local variable shadows one in exterior scope"},
  {cat_data_flow, "data_flow",
     "perform data flow analysis to detect suspicious operations"},
  {cat_null_reference, "null_reference",
     "detect access to variables with possible NULL value"},
  {cat_zero_operand, "zero_operand",
     "one of the operands of a binary integer operation is zero"},
  {cat_zero_result, "zero_result",
     "result of operation is always zero"},
  {cat_redundant, "redundant",
     "operation always produces the same result"},
  {cat_overflow, "overflow", 
     "detect incorrect handling of operation overflow"},
  {cat_incomp_case, "incomp_case",
     "switch case label can't be produced by switch expression"},
  {cat_short_char_cmp, "short_char_cmp",
     "expression of char type is compared with one of short type"},
  {cat_string_cmp, "string_cmp", 
     "two strings are compared as object references"},
  {cat_weak_cmp, "weak_cmp", 
     "using of inequality comparison where equality can be checked"},
  {cat_domain, "domain",
     "operands doesn't belong to the domain of operator"},
  {cat_truncation, "truncation",
     "data can be lost as a result of type conversion"},
  {cat_bounds, "bounds",
     "array index or length is out of range"},
  {cat_done, "done", 
     "notification about verification completion"},
  {cat_all, "all",
     "produce messages of all categories"}
};

void usage()
{
  fprintf(stderr, "\
Usage: jlint {option|java_class_file}\n\
Options:\n\
  -help : print this text\n\
  -source <source_path> : path to directory with .java files\n\
  -history <file_name> : use history file to avoid repeated messages\n\
  -max_shown_paths <number> : max. shown paths between two sync. methods\n\
  (+|-)verbose : output more/less information about program activity\n\
  (+|-)message_category : select categories of messages to be reported\n\
  (+|-)message_code : enable/disable concrete message\n\
Message categories:\n");
  message_category group = msg_category_option[0].msg_cat;
  for (unsigned i = 0; i < items(msg_category_option); i++) { 
    message_category msg_cat = msg_category_option[i].msg_cat;
    if ((msg_cat & ~group) != 0) { 
      group = msg_cat;
      fprintf(stderr, "\n");
    }
    fprintf(stderr, "  %s : %s\n", 
            msg_category_option[i].cat_name, 
            msg_category_option[i].cat_desc);
  }
  if (verbose) { 
    fprintf(stderr, "\nMessages: (category:code: \"text\")\n");
    for (int i = 0; i < msg_last; i++) { 
      for (unsigned j = 0; j < items(msg_category_option); j++) { 
        if (msg_table[i].category == msg_category_option[j].msg_cat) {
          fprintf(stderr, "  %s:%s: \"%s\"\n", 
                  msg_category_option[j].cat_name,
                  msg_table[i].name, msg_table[i].format);
          break;
        }
      }
    }
  }
}

int main(int argc, char* argv[])
{
  int i, j;   

  if (argc == 1) { 
    usage();
    return EXIT_FAILURE;
  }

  for (i = 1; i < argc; i++) { 
    if (*argv[i] == '-' || *argv[i] == '+') { 
      bool enabled = *argv[i] == '+';
      char* option = &argv[i][1];
      int n_cat = items(msg_category_option);
      msg_select_category_option* msg = msg_category_option;  

      if (stricmp(option, "source") == 0 && i+1 < argc) {
        source_file_path = argv[++i];
        source_file_path_len = strlen(source_file_path);
        if (source_file_path[source_file_path_len] == FILE_SEP) {
          source_file_path_len -= 1;
        }
        source_path_redefined = true;
        continue;
      } 
      if (stricmp(option, "history") == 0 && i+1 < argc) {
        history = fopen(argv[++i], "a+");
        if (history != NULL) { 
          char his_buf[MAX_MSG_LENGTH];
          fseek(history, 0, SEEK_SET);
          while (fgets(his_buf, sizeof his_buf, history)) { 
            int len = strlen(his_buf); 
            if (len > 0) { 
              his_buf[len-1] = '\0';
              message_node::add_to_hash(his_buf);
            }
          }
          fseek(history, 0, SEEK_END);
        } else { 
          fprintf(stderr, "Failed to open history file '%s'\n",
                  argv[i]);
        }
        continue;
      }
      if (stricmp(option, "max_shown_paths") == 0 && i+1 < argc) {
        if (sscanf(argv[++i], "%d", &max_shown_paths) != 1) { 
          fprintf(stderr, "Bad parameter value for -max_shown_paths "
                  "option: '%s'\n", argv[i]);
        }
        continue;
      } 
      if (stricmp(option, "verbose") == 0) { 
        verbose = enabled;
        if (verbose) { 
          fprintf(stderr, 
                  "Jlint - program correctness verifier for Java, "
                  "version %s ("__DATE__").\n", VERSION);
        }
        continue;
      }
      if (stricmp(option, "help") == 0) { 
        usage();
        return EXIT_SUCCESS;
      }
      for (j = 0; j < n_cat; j++) { 
        if (stricmp(msg[j].cat_name, option) == 0) { 
          if (enabled) { 
            reported_message_mask |= msg[j].msg_cat;
          } else { 
            reported_message_mask &= ~msg[j].msg_cat;
          }
          break;
        }
      }
      if (j == n_cat) { 
        for (j = 0; j < msg_last; j++) { 
          if (stricmp(msg_table[j].name, option) == 0) { 
            msg_table[j].enabled = enabled;
            break;
          }
        }
        if (j == msg_last) { 
          fprintf(stderr, "Unrecognized option %s\n", argv[i]);
          usage();
        }
        return EXIT_FAILURE;
      }		 
    } else { 
      char* file_name = argv[i];
      if (!source_path_redefined) { 
        source_file_path = file_name;
        char* dirend = strrchr(source_file_path, FILE_SEP);
        source_file_path_len = (dirend != NULL)
          ? dirend - source_file_path : 0; 
      }
      proceed_file(file_name);
    }
  }
  class_desc::global_analysis();
  message_at(msg_done, utf_string(""), 0, (void*)n_messages);
  return EXIT_SUCCESS;
}  

