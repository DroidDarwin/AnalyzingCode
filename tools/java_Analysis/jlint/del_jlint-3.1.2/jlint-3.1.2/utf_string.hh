#ifndef UTF_STRING_HH
#define UTF_STRING_HH

#include <string>
#include <stdio.h>
#include <assert.h>
#include "functions.hh"
#include "types.hh"
#include "message_node.hh"

class method_desc;
class class_desc;

extern unsigned int string_hash_function(byte* p);

class utf_string {
 protected:
  int   len; // string length without trailing \0
  byte* data;

 public:
  bool operator == (utf_string const& str) const { 
    return len == str.len && memcmp(data, str.data, len) == 0; 
  }
  bool operator != (utf_string const& str) const { 
    return len != str.len || memcmp(data, str.data, len) != 0; 
  }
  bool operator == (const char* str) const { 
    return strcmp((char*)data, str) == 0; 
  }
  bool operator != (const char* str) const { 
    return strcmp((char*)data, str) != 0; 
  }
  unsigned hash() const { 
    return string_hash_function(data);
  }
  int  first_char() const { return data[0]; }

  void operator = (utf_string const& str) { 
    len = str.len;
    data = str.data;
  }

  utf_string operator + (const char* suffix) const { 
    utf_string str;
    str.len = len + strlen(suffix);
    str.data = new byte[str.len+1];
    memcpy(str.data, data, len);
    memcpy(str.data+len, suffix, str.len - len);
    str.data[str.len] = 0; // zero terminated string
    return str;
  }

  void append(int offs, utf_string const& suffix) { 
    assert(offs <= len);
    len = offs + suffix.len;
    byte* new_data = new byte[len+1];
    memcpy(new_data, data, offs);
    memcpy(new_data+offs, suffix.data, suffix.len);
    new_data[len] = 0; // zero terminated string
    delete[] data;
    data = new_data;
  }

  int rindex(byte ch) const { 
    byte* p = (byte*)strrchr((char*)data, ch);
    return p ? p - data : -1;
  }

  void set_size(int size) { len = size; }

  const char* as_asciz() const { return (const char*)data; }

  utf_string(int length, byte* str) { 
    len = length;
    data = new byte[length+1];
    memcpy(data, str, length);
    data[length] = 0;
  }
  utf_string(const char* str) { 
    len = strlen(str);
    data = (byte*)str;
  }

  utf_string(utf_string const& str) { 
    len = str.len;
    data = str.data;
  }

  utf_string(utf_string const& str, bool foo) {
    len = str.len;
    data = str.data;
    
    if (FILE_SEP != '/') { 
      // Produce valid operationg system dependent file name
      for (char* p = (char *) data; *p != '\0'; p++) { 
        if (*p == '/') { 
          *p = FILE_SEP;
        } 
      }
    }
  }

  utf_string() { len = 0; data = NULL; }
};

#endif
