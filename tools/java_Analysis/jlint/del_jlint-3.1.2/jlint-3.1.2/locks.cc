#include "locks.hh"
#include "class_desc.hh" // for debugging

bool Locks::acquire(Lock lock) { // acquire lock Lock (add to lockset)
  // return true if lock is new
  monitor_table::iterator entry = monTable.find(lock);
  if (entry == monTable.end()) { // new lock, insert
    monTable.insert(monitor_table::value_type(lock, 1));
    monStack.push_front(lock);
    return true;
  } else {
    // lock exists, do nothing (for now); later version: incr. count
#ifdef DUMP_MONITOR
    printf("Lock %s already acquired.\n", lock->name.as_asciz());
#endif
  }
  return false;
}
   
bool Locks::release(Lock lock) { // release lock Lock
  // return true on success
  if (getInnermost() == lock) {
    monStack.pop_front();
    monitor_table::iterator entry = monTable.find(lock);
    if (entry != monTable.end()) { // ignore counter value for now
      monTable.erase(entry);
    }
    return true;
  } else {
#ifdef DUMP_MONITOR
    Lock curr = getInnermost();
    printf("Lock %s != %s could not be released.\n", 
           lock->name.as_asciz(), curr ? curr->name.as_asciz() : "<none>");
#endif
    return false; // could not delete lock; was not innermost lock
  }
}
