#include <stdint.h>
typedef   int8_t s8;
typedef  uint8_t u8;
typedef  int16_t s16;
typedef uint16_t u16;
typedef  int32_t s32;
typedef uint32_t u32;
typedef  int64_t s64;
typedef uint64_t u64;

#include <stddef.h>
typedef ptrdiff_t offset;

#define Array_Count(Array) (sizeof(Array) / sizeof((Array)[0]))
#define Kilobytes(N) (1024L * (N))
#define Megabytes(N) (1024L * Kilobytes(N))
#define Gigabytes(N) (1024L * Megabytes(N))

typedef struct {
   u8 *Base;
   offset Size;
   offset Used;
} arena;

#include <stdio.h>
#include <stdlib.h>

static inline
arena Make_Arena(offset Size)
{
   arena Result = {0};
   Result.Size = Size;
   Result.Base = calloc(1, Size);

   return(Result);
}

static inline
void Destroy_Arena(arena *Arena)
{
   if(Arena->Base)
   {
      free(Arena->Base);
      Arena->Base = 0;
      Arena->Size = 0;
      Arena->Used = 0;
   }
}

static inline
u64 Atomic_Add_U64(volatile u64 *Pointer, u64 Value)
{
   // NOTE: Return the initial value, not the result.
#ifdef __GNUC__
   u64 Result = __sync_fetch_and_add(Pointer, Value);
#else
#  error Unsupported compiler.
#endif

   return(Result);
}
