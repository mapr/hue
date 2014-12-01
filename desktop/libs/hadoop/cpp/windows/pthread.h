#ifndef MAPR_WIN_PTHREAD_H__
#define MAPR_WIN_PTHREAD_H__

#include "common/nonlinuxsupport.h"

#ifdef __WINDOWS__

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#define MUTEX_NEEDS_INIT 20

typedef INIT_ONCE pthread_once_t;
#define PTHREAD_ONCE_INIT INIT_ONCE_STATIC_INIT

struct pthread_mutex_t_ {
  long initBit;
  INIT_ONCE once;
  CRITICAL_SECTION cs;
};

struct pthread_attr_t {
  int     __detachstate;
};

enum {
  PTHREAD_CREATE_JOINABLE = 0,
  PTHREAD_CREATE_DETACHED
};
 
typedef HANDLE pthread_t;
typedef DWORD pthread_key_t;
typedef struct pthread_mutex_t_ pthread_mutex_t;
typedef CONDITION_VARIABLE pthread_cond_t;
typedef CRITICAL_SECTION pthread_spinlock_t;

int pthread_attr_init(pthread_attr_t *attr);

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);

int pthread_create(pthread_t *thread, pthread_attr_t *attr, 
      void *(*start_routine)(void *), void *arg);

int pthread_join(pthread_t thread, void **value_ptr);

pthread_t pthread_self(void);

int pthread_mutex_init(pthread_mutex_t *mutex, void *attr);

int pthread_mutex_destroy(pthread_mutex_t *mutex);

int pthread_mutex_lock(pthread_mutex_t *mutex);

int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_cond_init(pthread_cond_t *cv, void *attr);

int pthread_cond_destroy(pthread_cond_t *cv);

int pthread_cond_wait(pthread_cond_t *cv, pthread_mutex_t *mutex);

int pthread_cond_timedwait(pthread_cond_t *cv, pthread_mutex_t *mutex,
  struct timespec *abstime);

int pthread_cond_signal(pthread_cond_t *cv);

int pthread_cond_broadcast(pthread_cond_t *cv);

int pthread_spin_init(pthread_spinlock_t *lock, int pshared);

int pthread_spin_destroy(pthread_spinlock_t *lock);

int pthread_spin_lock(pthread_spinlock_t *lock);

int pthread_spin_unlock(pthread_spinlock_t *lock);

void pthread_yield(void);

bool pthread_equal(pthread_t t1, pthread_t t2);

#define PTHREAD_MUTEX_INITIALIZER { MUTEX_NEEDS_INIT }

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));

int pthread_key_delete(pthread_key_t key);

void* pthread_getspecific(pthread_key_t key);

int pthread_setspecific(pthread_key_t key, const void *value);

int pthread_once(pthread_once_t *once_control, void (*init_routine) (void));

#endif  //__WINDOWS__
#endif  //MAPR_WIN_PTHREAD_H__
