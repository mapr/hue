/* */

#ifndef MAPR_THRDLOCAL
#define MAPR_THRDLOCAL

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>
#include <stdlib.h>

#include <common/linkedList2.h>

namespace mapr {
namespace fs {

/* Thread Local Storage (TLS)
 * This class is for use in reserving and filling a slot in the thread
 * local storage array
 *
 * Because there is a maximum number of slots in the the thread local
 * storage array, this class should be created sparingly.  The recommended
 * usage is for individual components to define a structure of pointers
 * to memory/structures/classes that encompasses all data that is
 * required to be accessible via thread local means and create this
 * class only once per component
 *
 * Note that while the local store is accessed individually in
 * different threads, the memory is globally available memory.
 */
typedef void (*thrdLclDestructor)(void *);
typedef struct thrdMemPrefix thrdMemPrefix;

class ThreadLocalStore
{
public:
  /* ThreadLocalStore initializer
   * Parameters:
   *    destr  - The destructor for the thread local storage area
   *             This destructor is called indirectly and is
   *             responsible only for cleaning up the memory of the
   *             TLS, e.g. deleting any memory pointed to by the TLS,
   *             deleting the TLS itself, etc.  It is not required to
   *             manipulate the thread key itself, nor should it try to
   */
  ThreadLocalStore(thrdLclDestructor destr);

  ~ThreadLocalStore();

  /* Create a chunk of global memory that will be accessed from a
   * thread
   */
  void *createLocalStore(int storeSize);
  
  /* Retrieve a chunk of memory stored in thread local storage */
  void *getLocalStore();

  /* Perform cleanup operations on the memory stored locally
   * This is not the same as the destructor function, which is called
   * involuntarily on thread termination.  This is called willfully by
   * the client to perform cleanup of the thread local storage
   */
  void deleteLocalStore();

  /* This function is called by the Windows DllMain function for thread
   * local storage cleanup for all ThreadLocalStore objects.
   */
  static int deleteAllLocalStoreForThread();

  /* This is only called by Linux thread cleanup routines to clean up
   * for a specific piece of thread local storage
   */
  static void destroyLocalStore(void *localStore);
  
private:
  pthread_key_t      thrdKey;
  thrdLclDestructor  thrdDestr;
  
  linkedList2        thrdQueue;

  /* The following is only really needed for Windows because we need to
     be able to find all objects created to destroy the thread local of
     all objects on thread termination
  */
  static bool        gblInitCompleted;
  static linkedList2 globalQueue;
  linkedListEntry2   globalQueueEntry;
  
  /* This function will be called on thread termination to clean up
   * the thread local memory, but will not be called directly
   */
  void deleteLocalStore(thrdMemPrefix *localStore);  
};
} // namespace fs
} // namespace mapr
#endif // ifndef MAPR_THRDLOCAL
