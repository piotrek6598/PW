/** @file
 * Implementation of handling for basic pthread functions' errors.
 *
 * @author Piotr Jasinski <jasinskipiotr99@gmail.com>
 */

#include "pthread_err_supp.h"
#include "err.h"


/** @brief mutex_lock Handles mutex locking errors.
 * @param mutex
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
