#include "shared.h"
#include "multicore.h"

static volatile int Thread_Count;
static os_barrier *Barrier;

#pragma pack(push, 1)
typedef struct {
   u16 Signature;
   u32 File_Size;
   u32 Reserved;
   u32 Data_Offset;

   u32 Header_Size;
   u32 Width;
   u32 Height;
   u16 Planes;
   u16 Bits_Per_Pixel;
   u32 Compression;
   u32 Image_Size;
   u32 X_Pixels_Per_Meter;
   u32 Y_Pixels_Per_Meter;
   u32 Colors_Used;
   u32 Important_Colors;
} bmp_header;
#pragma pack(pop)

static void
Save_Bitmap_To_File(u32 *Memory, int Width, int Height, char *Filename)
{
   int Bitmap_Size = Width * Height * sizeof(*Memory);

   bmp_header Header = {0};
   Header.Signature = 0x4D42;
   Header.File_Size = Bitmap_Size + sizeof(Header);
   Header.Data_Offset = sizeof(Header);

   Header.Header_Size = 40;
   Header.Width = Width;
   Header.Height = -Height;
   Header.Planes = 1;
   Header.Bits_Per_Pixel = 32;
   Header.Image_Size = Bitmap_Size;

   FILE *File = fopen(Filename, "wb");
   if(File)
   {
      fwrite(&Header, sizeof(Header), 1, File);
      fwrite(Memory, Header.Image_Size, 1, File);

      printf("Wrote file to %s\n", Filename);
   }
   else
   {
      fprintf(stderr, "Failed to write output bmp %s\n", Filename);
   }
}

ENTRY_POINT(Thread_Main)
{
   int Bitmap_Width = 2048 + 11;
   int Bitmap_Height = 2048 + 17;
   int Tile_Dim = 32;

   int X_Tile_Count = Bitmap_Width / Tile_Dim;
   int Y_Tile_Count = Bitmap_Height / Tile_Dim;

   int X_Leftover_Count = Bitmap_Width % Tile_Dim;
   int Y_Leftover_Count = Bitmap_Height % Tile_Dim;

   static u64 Taken_Tiles = 0;
   int Tile_Count = X_Tile_Count * Y_Tile_Count;

   static u32 *Bitmap_Memory;
   if(In_Main_Thread(Thread))
   {
      Bitmap_Memory = calloc(Bitmap_Width*Bitmap_Height, sizeof(*Bitmap_Memory));
   }

   Synchronize_OS_Barrier(Barrier);

   for(;;)
   {
      int Tile_Index = Atomic_Add_U64(&Taken_Tiles, 1);
      if(Tile_Index < Tile_Count)
      {
         int Tile_Y = Tile_Index / X_Tile_Count;
         int Tile_X = Tile_Index % X_Tile_Count;

         int X_Has_Leftover = (Tile_X < X_Leftover_Count);
         int Y_Has_Leftover = (Tile_Y < Y_Leftover_Count);

         int X_Prior_Leftovers = X_Has_Leftover ? Tile_X : X_Leftover_Count;
         int Y_Prior_Leftovers = Y_Has_Leftover ? Tile_Y : Y_Leftover_Count;

         int First_X = (Tile_X * Tile_Dim) + X_Prior_Leftovers;
         int First_Y = (Tile_Y * Tile_Dim) + Y_Prior_Leftovers;

         int Past_Last_X = First_X + Tile_Dim + !!X_Has_Leftover;
         int Past_Last_Y = First_Y + Tile_Dim + !!Y_Has_Leftover;

         int Channel_Shift = (Thread->Index % 3) * 8;
         u32 Color = 0xFF000000 | (0xFF << Channel_Shift);

         for(int Y = First_Y; Y != Past_Last_Y; ++Y)
         {
            for(int X = First_X; X != Past_Last_X; ++X)
            {
               Bitmap_Memory[Y*Bitmap_Width + X] = Color;
            }
         }
      }
      else
      {
         break;
      }
   }

   Synchronize_OS_Barrier(Barrier);
   if(In_Main_Thread(Thread))
   {
      Save_Bitmap_To_File(Bitmap_Memory, Bitmap_Width, Bitmap_Height, "output.bmp");
   }
}

int main(void)
{
   Thread_Count = Count_Cpu_Cores();
   printf("Using %d threads.\n", Thread_Count);

   os_thread *Threads = calloc(Thread_Count, sizeof(*Threads));
   Barrier = Make_OS_Barrier(Thread_Count);

   for(int Index = 0; Index < Thread_Count; ++Index)
   {
      Launch_OS_Thread(Threads+Index, Thread_Main, Index, Megabytes(1));
   }
   for(int Index = 0; Index < Thread_Count; ++Index)
   {
      Join_OS_Thread(Threads+Index);
   }
   Destroy_OS_Barrier(Barrier);

   return(0);
}
