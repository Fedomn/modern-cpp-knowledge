// https://github.com/huangjiahua/haz_ptr
// https://www.wikiwand.com/en/articles/Hazard_pointer

// Implementation Principles:
// Hazard Pointer Per Thread:
// Each participating thread maintains a set of "hazard pointers." These are pointers that a thread sets to indicate which
// objects it is currently accessing.

// Protection:
// When a thread accesses an object that might be concurrently deallocated, it first places the object's address into one of
// its hazard pointers. This signals to other threads that this object is "in use" and should not be deallocated.

// Retire List:
// When an object is logically removed from a data structure (e.g., a node is removed from a lock-free list), it is not
// immediately deallocated. Instead, its address is placed on a "retire list" or "quarantine list."

// Safe Reclamation:
// Periodically, a thread (or a dedicated reclamation thread) scans the retire list. For each object on the retire list, it
// checks if its address is present in any thread's hazard pointer. If an object's address is not found in any active hazard
// pointer, it means no thread is currently accessing it, and it can be safely deallocated.
