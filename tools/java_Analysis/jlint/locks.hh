#ifndef LOCKS_HH
#define LOCKS_HH

#ifdef __GNUC__
#if __GNUC__ > 2
using namespace std;
#endif
#endif

/******************************************************************************/
/* Class that handles the locks that each thread is holding.                  */
/******************************************************************************/

#include "types.hh"
#include "field_desc.hh"
#include "stdio.h" // for debugging output

typedef const field_desc* Lock;

// table of all locks a thread is holding
#ifdef HASH_TABLE
#include <hash_map>
#else
#include <map>
#endif
#ifdef HASH_TABLE
struct eqLock {
  bool operator()(const Lock l1, const Lock l2) const {
    assert(l1 != NULL);
    assert(l2 != NULL);
    return strcmp(l1->name.as_asciz(), l2->name.as_asciz()) == 0;
  }
};

struct hashLock {
  hash<const char*> H;
  size_t operator() (const Lock l) const {
    return H(const_cast<const char*>(l->name.as_asciz()));
  }
};

typedef hash_map<const Lock, int, hashLock, eqLock> monitor_table;
#else
struct ltLock {
  bool operator()(const Lock l1, const Lock l2) const {
    return strcmp(l1->name.as_asciz(), l2->name.as_asciz()) < 0;
  }
};

typedef map<const Lock, int, ltLock> monitor_table;
#endif

// stack of locks a thread acquired (in "chronological" order)
#ifdef SLIST
#include <slist>
#else
#include <list>
#endif
#ifdef SLIST
typedef slist<Lock> monitor_stack;
#else
typedef list<Lock> monitor_stack;
#endif

class Locks { 
private:
  monitor_table monTable;
  monitor_stack monStack;
public:
  void clear() { // clear set of locks - call before processing new method
    // (no inter-method flow analysis done yet)
    monTable.clear();
    monStack.clear();
  }

  bool acquire(Lock lock); // acquire lock Lock (add to lockset)
  bool release(Lock lock); // release lock Lock

  bool owns(Lock lock) { // return true if lock is in table
      monitor_table::iterator entry = monTable.find(lock);
      return (entry != monTable.end());
  }

  int nLocks() { // return number of locks held
    return monTable.size();
  }

  monitor_stack::const_iterator begin() {
    return monStack.begin();
  }

  monitor_stack::const_iterator end() {
    return monStack.end();
  }

  Lock getInnermost() { // get most recently acquired lock
    if (!(monStack.empty())) {
      return monStack.front();
    } else {
      return NULL;
    }
  }
};

#endif
