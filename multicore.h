typedef struct os_thread os_thread;
typedef struct os_barrier os_barrier;

#define ENTRY_POINT(Name) void Name(os_thread *Thread)
typedef ENTRY_POINT(entry_point);

struct os_thread
{
   int Index;
   arena Scratch;
   entry_point *Entry_Point;

   void *Platform_Handle;
};

static inline
int In_Main_Thread(os_thread *Thread)
{
   int Result = (Thread->Index == 0);
   return(Result);
}

typedef struct {
   volatile u64 Serving;
   volatile u64 Count;
} ticket_mutex;

static inline
void Begin_Ticket_Mutex(ticket_mutex *Mutex)
{
   u64 Ticket = Atomic_Add_U64(&Mutex->Count, 1);
   while(Ticket != Mutex->Serving);
}

static inline
void End_Ticket_Mutex(ticket_mutex *Mutex)
{
   Atomic_Add_U64(&Mutex->Serving, 1);
}

// NOTE: OS-provided API.
#define COUNT_CPU_CORES(Name) int Name(void)
COUNT_CPU_CORES(Count_Cpu_Cores);

#define LAUNCH_OS_THREAD(Name) void Name(os_thread *Thread, entry_point *Entry_Point, s64 Index, offset Scratch_Size)
LAUNCH_OS_THREAD(Launch_OS_Thread);

#define JOIN_OS_THREAD(Name) void Name(os_thread *Thread)
JOIN_OS_THREAD(Join_OS_Thread);

#define MAKE_OS_BARRIER(Name) os_barrier *Name(int Thread_Count)
MAKE_OS_BARRIER(Make_OS_Barrier);

#define SYNCHRONIZE_OS_BARRIER(Name) void Name(os_barrier *Barrier)
SYNCHRONIZE_OS_BARRIER(Synchronize_OS_Barrier);

#define DESTROY_OS_BARRIER(Name) void Name(os_barrier *Barrier)
DESTROY_OS_BARRIER(Destroy_OS_Barrier);
