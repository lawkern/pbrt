#include "shared.h"
#include "multicore.h"

static volatile int Thread_Count;
static os_barrier *Barrier;

static int Bitmap_Width = 1024;
static int Bitmap_Height = 1024;
static u32 *Bitmap;

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
   Header.Height = Height;
   Header.Planes = 1;
   Header.Bits_Per_Pixel = 32;
   Header.Image_Size = Bitmap_Size;

   FILE *File = fopen(Filename, "wb");
   if(File)
   {
      fwrite(&Header, sizeof(Header), 1, File);
      fwrite(Memory, Header.Image_Size, 1, File);
   }
   else
   {
      fprintf(stderr, "Failed to write output bmp %s\n", Filename);
   }
}

ENTRY_POINT(Thread_Main)
{
   if(In_Main_Thread(Thread))
   {
      Bitmap = calloc(Bitmap_Width*Bitmap_Height, sizeof(*Bitmap));
   }
   Synchronize_OS_Barrier(Barrier);

   int Tile_Width = Bitmap_Width;
   int Tile_Height = Bitmap_Height / Thread_Count;

   int Start_X = 0;
   int Start_Y = Thread->Index * Tile_Height;

   u32 Color = (Thread->Index & 1) ? 0x0000FF00 : 0x000000FF;

   for(int Y = Start_Y; Y < Start_Y+Tile_Height; ++Y)
   {
      for(int X = Start_X; X < Start_X+Tile_Width; ++X)
      {
         Bitmap[Y*Bitmap_Width + X] = Color;
      }
   }

   Synchronize_OS_Barrier(Barrier);
   if(In_Main_Thread(Thread))
   {
      Save_Bitmap_To_File(Bitmap, Bitmap_Width, Bitmap_Height, "output.bmp");
   }
}

int main(void)
{
   Thread_Count = Count_Cpu_Cores();
   printf("Detected %d CPU Cores.\n", Thread_Count);

   os_thread *Threads = calloc(Thread_Count, sizeof(*Threads));
   Barrier = Initialize_OS_Barrier(Thread_Count);

   for(int Thread_Index = 1; Thread_Index < Thread_Count; ++Thread_Index)
   {
      Launch_OS_Thread(Threads+Thread_Index, Thread_Main, Thread_Index, Megabytes(1));
   }
   Thread_Main(Threads+0);

   for(int Thread_Index = 1; Thread_Index < Thread_Count; ++Thread_Index)
   {
      Join_OS_Thread(Threads+Thread_Index);
   }
   Destroy_OS_Barrier(Barrier);

   return(0);
}
