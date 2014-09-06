#ifndef STRING_POOL_HH
#define STRING_POOL_HH

#include <cstdlib>

#ifdef __GNUC__
#if __GNUC__ > 2
using namespace std;
#endif
#endif


/******************************************************************************/
/* Class that allocates a const char* if not yet present in string pool       */
/******************************************************************************/

// table of all locks a thread is holding
#ifdef HASH_TABLE
#include <hash_set>
#else
#include <set>
#endif
#ifdef HASH_TABLE
struct eqstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};
typedef hash_set<const char*, hash<const char*>, eqstr> string_table;
#else
struct ltstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};
typedef set<const char*, ltstr> string_table;
#endif

class string_pool {
public:
  string_table pool;

  const char* add(const char* el) {
    char* entry;
    string_table::iterator it;
    if ((it = pool.find(el)) == pool.end()) {
      // allocate element!
      entry = strdup(el);
      pool.insert(entry);
      return const_cast<const char*>(entry);
    } else {
      return *it;
    }
  }

  void remove(const char* el) {
    string_table::iterator entry;
    if ((entry = pool.find(el)) != pool.end()) {
      // free space for element
      free(const_cast<char*>(*entry));
      pool.erase(entry);
    }
  }

  void clear() {
    for (string_table::iterator it = pool.begin(); it != pool.end(); it++) {
      free(const_cast<char*>(*it));
      pool.erase(it);
    }
  }
};
#endif
