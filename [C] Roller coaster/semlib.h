#ifndef _SEMLIB_H_
#define _SEMLIB_H_

typedef pthread_mutex_t bsem;

extern int mybsem_init(pthread_mutex_t *mutex, int value);
extern int down(pthread_mutex_t *mutex);
extern int up(pthread_mutex_t *mutex);
extern int mybsem_destroy(pthread_mutex_t *mutex);

#endif