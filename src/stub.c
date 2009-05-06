/*
  Single Executable Bundle Stub

  This stub reads itself for embedded instructions to create directory
  and files in a temporary directory, launching a program.
*/

#include <windows.h>
#include <shellapi.h>
#include <stdio.h>

const BYTE Signature[] = { 0x41, 0xb6, 0xba, 0x4e };

#define OP_END 0
#define OP_CREATE_DIRECTORY 1
#define OP_CREATE_FILE 2
#define OP_CREATE_PROCESS 3
#define OP_DECOMPRESS_LZMA 4
#define OP_SETENV 5
#define OP_MAX 6

void ProcessImage(LPVOID p, DWORD size);
void ProcessOpcodes(LPVOID* p);

BOOL OpEnd(LPVOID *p);
BOOL OpCreateFile(LPVOID *p);
BOOL OpCreateDirectory(LPVOID *p);
BOOL OpCreateProcess(LPVOID *p);
BOOL OpDecompressLzma(LPVOID *p);
BOOL OpSetEnv(LPVOID *p);
#include <LzmaDec.h>

typedef BOOL (*POpcodeHandler)(LPVOID*);

DWORD ExitStatus = 0;

POpcodeHandler OpcodeHandlers[OP_MAX] = {
   &OpEnd,
   &OpCreateDirectory,
   &OpCreateFile,
   &OpCreateProcess,
   &OpDecompressLzma,
   &OpSetEnv
};

CHAR InstDir[MAX_PATH];

/** Decoder: Zero-terminated string */
LPTSTR GetString(LPVOID* p)
{
   LPTSTR str = *p;
   *p += strlen(str) + 1;
   return str;
}

/** Decoder: 32 bit unsigned integer */
DWORD GetInteger(LPVOID* p)
{
   DWORD dw = *(DWORD*)*p;
   *p += 4;
   return dw;
}

int main(int argc, char** argv)
{
   CHAR TempPath[MAX_PATH];
   GetTempPath(MAX_PATH, TempPath);
   GetTempFileName(TempPath, "seb", 0, InstDir);
#ifdef _DEBUG
   printf("Temporary directory: %s\n", InstDir);
#endif

   /* Attempt to delete the temp file created by GetTempFileName.
      Ignore errors, i.e. if it doesn't exist. */
   (void)DeleteFile(InstDir);

   /* Create the temporary directory that will hold the extracted files */
   if (!CreateDirectory(InstDir, NULL)){
      printf("Failed to create temporary directory.\n");
      return -1;
   }

   /* Find name of image */
   TCHAR ImageFileName[MAX_PATH];
   if (!GetModuleFileName(NULL, ImageFileName, MAX_PATH)) {
      fprintf(stderr, "Failed to get executable name (error %lu).\n", GetLastError());
      return -1;
   }

   /* Open the image (executable) */
   HANDLE hImage = CreateFile(ImageFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
   if (hImage == INVALID_HANDLE_VALUE) {
      fprintf(stderr, "Failed to open executable (%s)\n", ImageFileName);
      return -1;
   }      

   /* Create a file mapping */
   DWORD FileSize = GetFileSize(hImage, NULL);
   HANDLE hMem = CreateFileMapping(hImage, NULL, PAGE_READONLY, 0, FileSize, NULL);
   if (hMem == INVALID_HANDLE_VALUE) {
      fprintf(stderr, "Failed to create file mapping (error %lu)\n", GetLastError());
      CloseHandle(hImage);
      return -1;
   }

   /* Map the image into memory */
   LPVOID lpv = MapViewOfFile(hMem, FILE_MAP_READ, 0, 0, 0);
   if (lpv == NULL)
   {
      fprintf(stderr, "Failed to map view of executable into memory (error %lu).\n", GetLastError());
   }
   else
   {
      ProcessImage(lpv, FileSize);
      if (!UnmapViewOfFile(lpv))
         fprintf(stderr, "Failed to unmap view of executable.\n");
   }

   if (!CloseHandle(hMem))
      fprintf(stderr, "Failed to close file mapping.\n");

   if (!CloseHandle(hImage))
      fprintf(stderr, "Failed to close executable.\n");

   /* Remove the temporary directory */
   SHFILEOPSTRUCT shop;
   shop.hwnd = NULL;
   shop.wFunc = FO_DELETE;
   InstDir[strlen(InstDir)+1] = 0;
   shop.pFrom = InstDir;
   shop.pTo = NULL;
   shop.fFlags = FOF_NOCONFIRMATION;
   SHFileOperation(&shop);

   ExitProcess(ExitStatus);

   /* Never gets here */
   return 0;
}

/**
   Process the image by checking the signature and locating the first
   opcode.
*/
void ProcessImage(LPVOID ptr, DWORD size)
{
   LPVOID pSig = ptr + size - 4;
   if (memcmp(pSig, Signature, 4) == 0)
   {
#ifdef _DEBUG
      printf("Good signature found.\n");
#endif
      DWORD OpcodeOffset = *(DWORD*)(pSig - 4);
      LPVOID pSeg = ptr + OpcodeOffset;
      ProcessOpcodes(&pSeg);
   }
   else
   {
      fprintf(stderr, "Bad signature in executable.\n");
   }
}

/**
   Process the opcodes in memory.
*/
void ProcessOpcodes(LPVOID* p)
{
   BOOL Stop = FALSE;
   while(!Stop)
   {
      DWORD opcode = GetInteger(p);
      if (opcode >= OP_MAX || !OpcodeHandlers[opcode](p))
         break;
   }
}

/**
   Expands a specially formatted string, replacing \xFF with the
   temporary installation directory.
*/
void ExpandPath(LPTSTR out, LPTSTR str)
{
   char* a;
   while ((a = strchr(str, '\xFF')))
   {
      int l = a - str;
      if (l > 0) {
         memcpy(out, str, l);
         out += l;
         str += l;
      }
      str += 1;
      strcpy(out, InstDir);
      out += strlen(out);
   }
   strcpy(out, str);
}

/**
   Finds the start of the first argument after the current one. Handles quoted arguments.
*/
LPTSTR SkipArg(LPTSTR str)
{
   if (*str == '"') {
      str++;
      while (*str && *str != '"') str++;
      if (*str == '"') str++;
   } else {
      while (*str && *str != ' ') str++;
   }
   while (*str && *str != ' ') str++;
   return str;
}

/**
   Create a file (OP_CREATE_FILE opcode handler)
*/
BOOL OpCreateFile(LPVOID *p)
{
   BOOL Result = TRUE;
   LPTSTR FileName = GetString(p);
   DWORD FileSize = GetInteger(p);
   LPVOID Data = *p;
   *p += FileSize;

   CHAR Fn[MAX_PATH];
   strcpy(Fn, InstDir);
   strcat(Fn, "\\");
   strcat(Fn, FileName);
   
#ifdef _DEBUG
   printf("CreateFile(%s, %lu)\n", Fn, FileSize);
#endif
   HANDLE hFile = CreateFile(Fn, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
   if (hFile != INVALID_HANDLE_VALUE)
   {
      DWORD BytesWritten;
      if (!WriteFile(hFile, Data, FileSize, &BytesWritten, NULL))
      {
         printf("Write failure\n");
         Result = FALSE;
      }
      if (BytesWritten != FileSize)
      {
         fprintf(stderr, "Write size failure\n");
         Result = FALSE;
      }
      CloseHandle(hFile);
   }
   else
   {
      fprintf(stderr, "Failed to create file '%s'\n", Fn);
      Result = FALSE;
   }

   return Result;
}

/**
   Create a directory (OP_CREATE_DIRECTORY opcode handler)
*/
BOOL OpCreateDirectory(LPVOID *p)
{
   LPTSTR DirectoryName = GetString(p);

   CHAR DirName[MAX_PATH];
   strcpy(DirName, InstDir);
   strcat(DirName, "\\");
   strcat(DirName, DirectoryName);
   
#ifdef _DEBUG
   printf("CreateDirectory(%s)\n", DirName);
#endif
   
   if (!CreateDirectory(DirName, NULL)){
      printf("Failed to create directory.\n");
      return FALSE;
   }
   
   return TRUE;
}

/**
   Create a new process and wait for it to complete (OP_CREATE_PROCESS
   opcode handler)
*/
BOOL OpCreateProcess(LPVOID *p)
{
   LPTSTR ImageName = GetString(p);
   LPTSTR CmdLine = GetString(p);

   STARTUPINFO StartupInfo;
   ZeroMemory(&StartupInfo, sizeof(StartupInfo));
   StartupInfo.cb = sizeof(StartupInfo);

   CHAR ApplicationName[MAX_PATH];
   strcpy(ApplicationName, InstDir);
   strcat(ApplicationName, "\\");
   strcat(ApplicationName, ImageName);

   CHAR CmdLine2[MAX_PATH];
   ExpandPath(CmdLine2, CmdLine);

   LPTSTR MyCmdLine = GetCommandLine();
   LPTSTR MyArgs = SkipArg(MyCmdLine);
   
   LPTSTR CmdLine3 = LocalAlloc(LMEM_FIXED, strlen(CmdLine2) + 1 + strlen(MyArgs) + 1);
   strcpy(CmdLine3, CmdLine2);
   strcat(CmdLine3, " ");
   strcat(CmdLine3, MyArgs);
   
#ifdef _DEBUG
   printf("CreateProcess(%s, %s)\n", ApplicationName, CmdLine2);
#endif
   
   PROCESS_INFORMATION ProcessInformation;
   BOOL r = CreateProcess(ApplicationName, CmdLine3, NULL, NULL,
                 TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation);

   LocalFree(CmdLine3);

   if (!r)
   {
      printf("Failed to createprocess %lu\n", GetLastError());
   }

   WaitForSingleObject(ProcessInformation.hProcess, INFINITE);

   if (!GetExitCodeProcess(ProcessInformation.hProcess, &ExitStatus))
      fprintf(stderr, "Failed to get exit status (error %lu).\n", GetLastError());

   CloseHandle(ProcessInformation.hProcess);
   CloseHandle(ProcessInformation.hThread);
   
   return TRUE;
}

void *SzAlloc(void *p, size_t size) { p = p; return malloc(size); }
void SzFree(void *p, void *address) { p = p; free(address); }
ISzAlloc alloc = { SzAlloc, SzFree };

#if WITH_LZMA

#define LZMA_UNPACKSIZE_SIZE 8
#define LZMA_HEADER_SIZE (LZMA_PROPS_SIZE + LZMA_UNPACKSIZE_SIZE)

BOOL OpDecompressLzma(LPVOID *p)
{
   DWORD CompressedSize = GetInteger(p);
#ifdef _DEBUG
   printf("LzmaDecode(%ld)\n", CompressedSize);
#endif
   
   Byte* src = (Byte*)*p;
   *p += CompressedSize;

   UInt64 unpackSize = 0;
   int i;
   for (i = 0; i < 8; i++)
      unpackSize += (UInt64)src[LZMA_PROPS_SIZE + i] << (i * 8);

   Byte* DecompressedData = LocalAlloc(LMEM_FIXED, unpackSize);
          
   SizeT lzmaDecompressedSize = unpackSize;
   SizeT inSizePure = CompressedSize - LZMA_HEADER_SIZE;
   ELzmaStatus status;
   SRes res = LzmaDecode(DecompressedData, &lzmaDecompressedSize, src + LZMA_HEADER_SIZE, &inSizePure,
                         src, LZMA_PROPS_SIZE, LZMA_FINISH_ANY, &status, &alloc);
   if (res != SZ_OK)
   {
      fprintf(stderr, "LZMA decompression failed.\n");
   }
   else
   {
      LPVOID decPtr = DecompressedData;
      ProcessOpcodes(&decPtr);
   }

   LocalFree(DecompressedData);
   return TRUE;
}
#endif

BOOL OpEnd(LPVOID* p)
{
   return FALSE;
}

BOOL OpSetEnv(LPVOID* p)
{
   LPTSTR Name = GetString(p);
   LPTSTR Value = GetString(p);
   CHAR ExpandedValue[MAX_PATH];
   ExpandPath(ExpandedValue, Value);
#ifdef _DEBUG
   printf("SetEnv(%s, %s)\n", Name, ExpandedValue);
#endif
   if (!SetEnvironmentVariable(Name, ExpandedValue))
   {
      fprintf(stderr, "Failed to set environment variable (error %lu).\n", GetLastError());
      return FALSE;
   }
   else
   {
      return TRUE;
   }
}

