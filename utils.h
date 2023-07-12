#ifdef _WIN32
  #include <windows.h>
  #include <wincrypt.h>

  #define CB_RT DWORD
  #define CB_CALL_CONV WINAPI

  #define CB_ARG LPVOID
  
  typedef struct Thread {
    HANDLE _thread;
    DWORD id;
  } Thread;

  typedef struct Mutex {
    HANDLE _mutex;
  } Mutex;
 

#else
  #include <openssl/evp.h>
  #include <pthread.h>
  #include <unistd.h>
  #include <semaphore.h>

  typedef struct Thread {
    pthread_t  _thread;
  } Thread;

  typedef struct Mutex {
    pthread_mutex_t _mutex;
  } Mutex;

#endif




#include <stdio.h>
#include <stdlib.h>

extern Mutex read_mutex;
extern Mutex write_mutex;



extern FILE *inputfile;
extern FILE *tempfile;
extern FILE *ouputfile;

void setup_thread_func(void (*callback)(void));
void start_thread(Thread* thread);
void join_thread(Thread* thread);

void create_mutex(Mutex* mutex);
void lock_mutex(Mutex* mutex);
void unlock_mutex(Mutex* mutex);

void md5(char *digest, char *input);









