#include <windows.h>
#include <stdio.h>




DWORD WINAPI threadFunc(LPVOID lpParam)
{
  printf("Thread started\n");
  return 0;
}

int main()
{
  HANDLE hThread;
  DWORD threadId;
  hThread = CreateThread(NULL, 0, threadFunc, NULL, 0, &threadId);
  if (hThread == NULL)
  {
    printf("Failed to create thread\n");
    return 1;
  }
  printf("Thread created with ID %d\n", threadId);
  WaitForSingleObject(hThread, INFINITE);
  printf("Thread finished\n");
  CloseHandle(hThread);
  return 0;
}