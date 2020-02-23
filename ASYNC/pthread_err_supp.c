/** @file
 * Implementation of handling for basic pthread functions' errors.
 *
 * @author Piotr Jasinski <jasinskipiotr99@gmail.com>
 */

#include "pthread_err_supp.h"
#include "err.h"
#include <signal.h>


/** @brief mutex_lock Handles mutex locking errors.
 * @param mutex[in, out]   - pointer to mutex.
 */
void mutex_lock(pthread_mutex_t *mutex){
  int err;
  if ((err = pthread_mutex_lock(mutex)) != 0)
    syserr(err, "mutex lock error");
}

/** @brief mutex_unlock Handles mutex unlocking errors.
 * @param mutex[in, out]   - pointer to mutex.
 */
void mutex_unlock(pthread_mutex_t *mutex){
  int err;
  if ((err = pthread_mutex_unlock(mutex)) != 0)
    syserr(err, "mutex unlock error");
}

/** @brief mutex_destroy Handles mutex destroying errors.
 * @param mutex[in]   - pointer to mutex.
 */
void mutex_destroy(pthread_mutex_t *mutex){
  int err;
  if ((err = pthread_mutex_destroy(mutex)) != 0)
    syserr(err, "mutex destroy error");
}

/** @brief condition_wait Handles waiting on condition errors.
 * @param cond[in, out]    - pointer to condition;
 * @param mutex[in, out]   - pointer to mutex to be released.
 */
void condition_wait(pthread_cond_t *cond, pthread_mutex_t *mutex){
  int err;
  if ((err = pthread_cond_wait(cond, mutex)) != 0)
    syserr(err, "condition wait error");
}

/** @brief condition_signal Handles signal on condition errors.
 * @param cond[in, out]    - pointer to condition.
 */
void condition_signal(pthread_cond_t *cond){
  int err;
  if ((err = pthread_cond_signal(cond)) != 0)
    syserr(err, "condition signal error");
}

/** @brief condition_broadcast Handles broadcast on condition errors.
 * @param cond[in, out]   - pointer to condition.
 */
void condition_broadcast(pthread_cond_t *cond){
  int err;
  if ((err = pthread_cond_broadcast(cond)) != 0)
    syserr(err, "condition broadcast error");
}

/** @brief condition_destroy Handles condition destroying errors.
 * @param cond[in]   - pointer to condition.
 */
void condition_destroy(pthread_cond_t *cond){
  int err;
  if ((err = pthread_cond_destroy(cond)) != 0)
    syserr(err, "condition destroy error");
}

/** @brief sigaction_create Handles sigaction creating errors.
 * @param signum    - signal's number;
 * @param act       - new signal handler;
 * @param old       - place to save previous signal handler.
 */
void sigaction_create(int signum, const struct sigaction *act,
                      struct sigaction *old){
  int err;
  if ((err = sigaction(signum, act, old)) != 0)
    syserr(err, "sigaction error");
}

/** @brief sigqueue_sig Handles sigqueue errors.
 * @param pid     - pid of process;
 * @param sig     - signal's number;
 * @param value   - sigval value.
 */
void sigqueue_sig(pid_t pid, int sig, const union sigval value){
  int err;
  if ((err = sigqueue(pid, sig, value)) != 0)
    syserr(err, "sigqueue error");
}

/** @brief thread_create Handles thread creation errors.
 * In case of error, function destroy @param attr attribute.
 * @param thread[in, out]   - pointer to thread's descriptor;
 * @param attr[in, out]     - thread's attributes ;
 * @param start_routine     - pointer to thread's function;
 * @param arg[in, out]      - arguments of thread's function.
 */
void thread_create(pthread_t *thread, pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg){
  int err;
  if ((err = pthread_create(thread, attr, start_routine, arg)) != 0){
      pthread_attr_destroy(attr);
      syserr(err, "thread create error");
    }
}

/** @brief thread_join Handles thread join errors.
 * @param thread[in]        - thread to be destroyed;
 * @param retval[in, out]   - place to store thread's exit status,
 */
void thread_join(pthread_t thread, void **retval){
  int err;
  if ((err = pthread_join(thread, retval)) != 0)
    syserr(err, "thread join error");
}
