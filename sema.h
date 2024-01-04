#include <semaphore.h>

class CSemaphore
{
private:
 sem_t myS;

public:
  CSemaphore(int c=0)
  {
   sem_init(&myS, 0, c);
  }

  ~CSemaphore()
  {
   sem_destroy(&myS);
  }

  void notify()
  {
   sem_post(&myS);
  }

  void wait()
  {
   sem_wait(&myS);
  }
};
