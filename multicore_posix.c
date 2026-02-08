#include "shared.h"
#include "multicore.h"

#include <pthread.h>
#include <unistd.h>

void *Posix_Thread_Entry_Point(void *Argument)
{
   os_thread *Thread = Argument;
   Thread->Entry_Point(Thread);

   return(0);
}

COUNT_CPU_CORES(Count_Cpu_Cores)
{
   int Result = sysconf(_SC_NPROCESSORS_CONF);
   return(Result);
}

LAUNCH_OS_THREAD(Launch_OS_Thread)
{
   Thread->Index = Index;
   Thread->Scratch = Make_Arena(Scratch_Size);
   Thread->Entry_Point = Entry_Point;

   pthread_t *Posix_Handle = (pthread_t *)&Thread->Platform_Handle;
   if(pthread_create(Posix_Handle, 0, Posix_Thread_Entry_Point, Thread) == 0)
   {
      // Success
   }
}

JOIN_OS_THREAD(Join_OS_Thread)
{
   pthread_t Posix_Handle = (pthread_t)Thread->Platform_Handle;
   void *Result;

   if(pthread_join(Posix_Handle, &Result) == 0)
   {
      Destroy_Arena(&Thread->Scratch);
   }
}

struct os_barrier
{
   pthread_barrier_t Handle;
};

MAKE_OS_BARRIER(Make_OS_Barrier)
{
   os_barrier *Result = calloc(1, sizeof(*Result));
   if(pthread_barrier_init(&Result->Handle, 0, Thread_Count) == 0)
   {
      // Success
   }
   return(Result);
}

SYNCHRONIZE_OS_BARRIER(Synchronize_OS_Barrier)
{
   pthread_barrier_wait(&Barrier->Handle);
}

DESTROY_OS_BARRIER(Destroy_OS_Barrier)
{
   if(pthread_barrier_destroy(&Barrier->Handle) == 0)
   {
      free(Barrier);
   }
}
