/** @file
 * Implementation of handling for basic pthread functions' errors.
 *
 * @author Piotr Jasinski <jasinskipiotr99@gmail.com>
 */

#include "pthread_err_supp.h"
#include "err.h"


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

/** @brief condition_wait Handles waiting on condition errors.
 * @param cond[in, out]    - pointer to condition;
 * @param mutex[in, out]   - pointer to mutex to be realeased.
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

/** @brief sigaction_create Handles sigaction creating errors.
 * @param signum    - signal's number;
 * @param act       - new signal handler;
 * @param old       - place to save previous signal handler.
 */
void sigaction_create(int signum, const sigaction *act, sigaction *old){
  int err;
  if ((err = sigaction(signum, act, old)) != 0)
    syserr(err, "sigaction error");
}

/** @brief sigqueue_sig Handles sigqueue errors.
 * @param pid     - pid of process;
 * @param sig     - signal's number;
 * @param value   - sigval value.
 */
void sigqueue_sig(pid_t pid, int sig, const sigval value){
  int err;
  if ((err = sigqueue(pid, sig, value)) != 0)
    syserr(err, "sigqueue error");
}
