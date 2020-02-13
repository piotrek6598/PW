/** @file
 * Interface providing handling for basic pthread library functions' errors.
 *
 * @author Piotr Jasinski <jasinskipiotr99@gmail.com>
 */

#ifndef PTHREAD_ERR_SUPP_H
#define PTHREAD_ERR_SUPP_H

#include <pthread.h>

/** @brief mutex_lock Handles mutex locking errors.
 * @param mutex[in, out]   - pointer to mutex.
 */
void mutex_lock(pthread_mutex_t *mutex);

/** @brief mutex_unlock Handles mutex unlocking errors.
 * @param mutex[in, out]   - pointer to mutex.
 */
void mutex_unlock(pthread_mutex_t *mutex);

/** @brief condition_wait Handles waiting on condition errors.
 * @param cond[in, out]    - pointer to condition;
 * @param mutex[in, out]   - pointer to mutex to be realeased.
 */
void condition_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

/** @brief condition_signal Handles signal on condition errors.
 * @param cond[in, out]    - pointer to condition.
 */
void condition_signal(pthread_cond_t *cond);

/** @brief condition_broadcast Handles broadcast on condition errors.
 * @param cond[in, out]   - pointer to condition.
 */
void condition_broadcast(pthread_cond_t *cond);

/** @brief sigaction_create Handles sigaction creating errors.
 * @param signum    - signal's number;
 * @param act       - new signal handler;
 * @param old       - place to save previous signal handler.
 */
void sigaction_create(int signum, const struct sigaction *act, struct sigaction *old);

/** @brief sigqueue_sig Handles sigqueue errors.
 * @param pid     - pid of process;
 * @param sig     - signal's number;
 * @param value   - sigval value.
 */
void sigqueue_sig(pid_t pid, int sig, const union sigval value);

#endif // PTHREAD_ERR_SUPP_H
