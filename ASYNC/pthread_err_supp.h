/** @file
 * Interface providing handling for basic pthread library functions' errors.
 *
 * @author Piotr Jasinski <jasinskipiotr99@gmail.com>
 */

#ifndef PTHREAD_ERR_SUPP_H
#define PTHREAD_ERR_SUPP_H

#include <pthread.h>

void mutex_lock(pthread_mutex_t *mutex);
void mutex_unlock(pthread_mutex_t *mutex);
void condition_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
void condition_signal(pthread_cond_t *cond);

#endif // PTHREAD_ERR_SUPP_H
