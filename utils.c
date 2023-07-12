#include "utils.h"
#include <string.h>

void (*_worker)(void);
void md5(char *digest, char *input);

void setup_thread_func(void (*callback)(void)) { _worker = callback; }

#ifdef _WIN32

    void md5(char *digest, char *input) {
      HCRYPTPROV hProv;
      HCRYPTHASH hHash;
      DWORD cbHash = 16;
      BYTE rgbHash[16];
      DWORD i;

      if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL,
                              CRYPT_VERIFYCONTEXT)) {
        return;
      }

      if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        return;
      }

      if (!CryptHashData(hHash, (BYTE *)input, (DWORD)strlen(input), 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return;
      }

      if (!CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return;
      }

      for (i = 0; i < cbHash; i++) {
        sprintf(&digest[i * 2], "%02x", rgbHash[i]);
      }
      digest[cbHash * 2] = '\0';
      CryptDestroyHash(hHash);
      CryptReleaseContext(hProv, 0);
      return;
    }

    DWORD WINAPI thread_func(LPVOID lpParam) {
      _worker();
      return 0;
    }

    void start_thread(Thread *thread) {
      HANDLE hThread;
      DWORD threadId;
      hThread = CreateThread(NULL, 0, thread_func, NULL, 0, &threadId);
      if (hThread == NULL) {
        printf("Failed to create thread\n");
        exit(EXIT_FAILURE);
      }

      thread->id = threadId;
      thread->_thread = hThread;
    }

    void join_thread(Thread *thread) {
      WaitForSingleObject(thread->_thread, INFINITE);
      CloseHandle(thread->_thread);
    }

    void create_mutex(Mutex *mutex) {
      mutex->_mutex = CreateMutex(NULL, FALSE, NULL);
    }
    void lock_mutex(Mutex *mutex) { WaitForSingleObject(mutex->_mutex, INFINITE); }
    void unlock_mutex(Mutex *mutex) { ReleaseMutex(mutex->_mutex); }
#else
  void md5(char *digest, char *input) {
    unsigned char md5_result[EVP_MAX_MD_SIZE];
    unsigned int md5_length;
    EVP_MD_CTX *mdctx;
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit(mdctx, EVP_md5());
    EVP_DigestUpdate(mdctx, input, strlen(input));
    EVP_DigestFinal(mdctx, md5_result, &md5_length);
    EVP_MD_CTX_free(mdctx);
    char result_str[md5_length * 2 + 1];
    for (int i = 0; i < md5_length; i++)
    {
      sprintf(&result_str[i * 2], "%02x", md5_result[i]);
    }

    strncpy(digest, result_str, 33);
  }

  void* thread_func(void* args) {
    _worker();
    return 0;
  }

  void start_thread(Thread *thread) {
    pthread_create(&(thread->_thread), NULL, thread_func, NULL);
  }

  void join_thread(Thread *thread) {
    pthread_join(&(thread->_thread), NULL);
  }

  void create_mutex(Mutex *mutex) {
    pthread_mutex_init(mutex->_mutex,NUll);
  }
  void lock_mutex(Mutex *mutex) { pthread_mutex_lock(mutex->_mutex); }
  void unlock_mutex(Mutex *mutex) { pthread_mutex_unlock(mutex->_mutex); }
#endif
