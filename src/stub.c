
/*
  Single Executable Bundle Stub

  This stub reads itself for embedded instructions to create directory
  and files in a temporary directory, launching a program.
*/

#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include "buffer.h"

const BYTE Signature[] = { 0x41, 0xb6, 0xba, 0x4e };

#define OP_END 0
#define OP_CREATE_DIRECTORY 1
#define OP_CREATE_FILE 2
#define OP_CREATE_PROCESS 3
#define OP_DECOMPRESS_LZMA 4
#define OP_SETENV 5
#define OP_POST_CREATE_PROCRESS 6
#define OP_MAX 7

/** Signature for function that reads data from source (file or decompressor) */
typedef void *(*ReadDataFunc)(LPVOID*, DWORD);

BOOL ProcessImage(LPVOID p, DWORD size);
BOOL ProcessOpcodes(READER* reader);
void CreateAndWaitForProcess(LPTSTR ApplicationName, LPTSTR CommandLine);

void *ReadDataFile(LPVOID *handle, DWORD dwSize);
void *ReadDataDecompress(LPVOID *handle, DWORD dwSize);

BOOL OpEnd(READER* reader);
BOOL OpCreateFile(READER* reader);
BOOL OpCreateDirectory(READER* reader);
BOOL OpCreateProcess(READER* reader);
BOOL OpDecompressLzma(READER* reader);
BOOL OpSetEnv(READER* reader);
BOOL OpPostCreateProcess(READER* reader);


#include <LzmaDec.h>
#define LZMA_UNPACKSIZE_SIZE 8
#define LZMA_HEADER_SIZE (LZMA_PROPS_SIZE + LZMA_UNPACKSIZE_SIZE)
void *SzAlloc(void *p, size_t size) { p = p; return malloc(size); }
void SzFree(void *p, void *address) { p = p; free(address); }
ISzAlloc alloc = { SzAlloc, SzFree };


void GetBytes(LPBYTE data, DWORD size, READER* reader);
LPTSTR GetString(READER* reader);
DWORD GetInteger(READER* reader);

/** Decoder: Byte-array */
void GetBytes(LPBYTE data, DWORD size, READER* reader)
{
   WaitForData(reader, size);
   DWORD ByteCount = min(size, reader->buffer->TotalBytes - reader->ByteOffset);
   CopyMemory(data, reader->buffer->Pages + reader->ByteOffset, ByteCount);
   if (ByteCount < size)
      CopyMemory(data + ByteCount, reader->buffer->Pages, size - ByteCount);
   ReleaseData(reader, size);
}

/** Decoder: Zero-terminated string */
LPTSTR GetString(READER* reader)
{
   DWORD StringLength = GetInteger(reader);
   LPTSTR result = LocalAlloc(LMEM_FIXED, StringLength + 1);
   GetBytes((LPBYTE)result, StringLength, reader);
   result[StringLength] = 0;
   return result;
}

/** Decoder: 32 bit unsigned integer */
DWORD GetInteger(READER* reader)
{
   WaitForData(reader, 4);
   DWORD offset = reader->ByteOffset;
   DWORD result = 0;
   int i;
   LPBYTE data = (LPBYTE)reader->buffer->Pages;
   for (i = 0; i < 4; i++)
   {
      DWORD ByteOffset = (offset + i) % reader->buffer->TotalBytes;
      BYTE value = data[ByteOffset];
      result += (DWORD)value << (i * 8);
   }
   ReleaseData(reader, 4);
   return result;
}

typedef struct
{
   LPTSTR FileName;
   DWORD dwOffset;
   BUFFER* buffer;
} FileReaderParam;

DWORD FileReader(LPVOID param)
{
   FileReaderParam* arg = (FileReaderParam*)param;
   HANDLE hImage = CreateFile(arg->FileName, GENERIC_READ,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL, OPEN_EXISTING, 0, NULL);
   if (hImage == INVALID_HANDLE_VALUE) {
      fprintf(stderr, "Failed to open executable (%s)\n", arg->FileName);
      return -1;
   }

   LONG offsetHigh = 0;
   DWORD result = SetFilePointer(hImage, arg->dwOffset, &offsetHigh, FILE_BEGIN);
   DWORD lastError = GetLastError();
   if (result == INVALID_SET_FILE_POINTER && lastError != NO_ERROR)
   {
      return 0;
   }

   WRITER writer;
   InitWriter(&writer, arg->buffer);

   DWORD dwFileOffset = 0;
   DWORD FileSize = GetFileSize(hImage, NULL);

   while (dwFileOffset < FileSize)
   {
      DWORD WriteCapacity = 0;
      LPVOID* page = GetWriteBuffer(&writer, &WriteCapacity);
      DWORD BytesToRead = min(FileSize - dwFileOffset, WriteCapacity);
      DWORD BytesRead = 0;
      if (!ReadFile(hImage, page, BytesToRead, &BytesRead, NULL))
      {
         fprintf(stderr, "ReadFile failed: %ld\n", GetLastError());
         return 0;
      }
      WriteComplete(&writer, BytesRead);
   }
}

typedef struct
{
   READER* reader;
   BUFFER* output;
} DecompressorParam;

DWORD Decompressor(LPVOID param)
{
   DecompressorParam* arg = (DecompressorParam*)param;
   
   // DWORD CompressedSize = GetInteger(arg->reader);
   BYTE LzmaHeader[LZMA_HEADER_SIZE];
   GetBytes(LzmaHeader, LZMA_HEADER_SIZE, arg->reader);

   UInt64 DecompressedSize = 0;
   int i;
   for (i = 0; i < 8; i++)
      DecompressedSize += (UInt64)LzmaHeader[LZMA_PROPS_SIZE + i] << (i * 8);
   
#ifdef _DEBUG
   printf("LzmaDecode(%ld)\n", CompressedSize);
#endif
   
   //SizeT lzmaDecompressedSize = DecompressedSize;
   // SizeT inSizePure = CompressedSize - LZMA_HEADER_SIZE;

   CLzmaDec state;
   SRes res;
   LzmaDec_Construct(&state);
   res = LzmaDec_Allocate(&state, LzmaHeader, LZMA_PROPS_SIZE, &alloc);
   if (res != SZ_OK)
      return FALSE;

   LzmaDec_Init(&state);

   BYTE* outBuf = 0;
   BYTE* inBuf = 0;
   
   LzmaDec_Init(&state);

   WRITER writer;
   writer.buffer = arg->output;
   writer.ByteOffset = 0;
   
   for (;;)
   {
      SRes res;
         
      ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
      ELzmaStatus status;

      /* Set up write buffer */
      DWORD WriteBufferSize;
      outBuf = (BYTE*)GetWriteBuffer(&writer, &WriteBufferSize);
      SizeT outProcessed = min(DecompressedSize, WriteBufferSize);
      /*
      if (outProcessed > WriteBufferSize)
      {
         outProcessed = (SizeT)DecompressedSize;
         finishMode = LZMA_FINISH_END;
      }
      */

      /* Set up read buffer */
      DWORD ReadBufferSize;
      inBuf = (BYTE*)GetReadBuffer(arg->reader, &ReadBufferSize);
      SizeT inProcessed = ReadBufferSize;
         
      res = LzmaDec_DecodeToBuf(&state, outBuf, &outProcessed,
                                inBuf, &inProcessed, finishMode, &status);
         
      fprintf(stderr, "Decoded %u to %u %d %d\n", inProcessed, outProcessed, res, status);

      /* Release the bytes read */
      ReleaseData(arg->reader, inProcessed);
      /* Notify completed write */
      WriteComplete(&writer, outProcessed);

      DecompressedSize -= outProcessed;
         
      if (res != SZ_OK || DecompressedSize == 0)
         break;
         
      if (inProcessed == 0 && outProcessed == 0)
      {
         if (status != LZMA_STATUS_FINISHED_WITH_MARK)
            return FALSE; // SZ_ERROR_DATA;
         return res;
      }
   }

   LzmaDec_Free(&state, &alloc);
   
   if (res != SZ_OK)
   {
      fprintf(stderr, "LZMA decompression failed.\n");
   }
   else
   {
   }
}

typedef BOOL (*POpcodeHandler)(READER* reader);

LPTSTR PostCreateProcess_ApplicationName = NULL;
LPTSTR PostCreateProcess_CommandLine = NULL;

DWORD ExitStatus = 0;
BOOL ExitCondition = FALSE;

POpcodeHandler OpcodeHandlers[OP_MAX] = {
   &OpEnd,
   &OpCreateDirectory,
   &OpCreateFile,
   &OpCreateProcess,
   &OpDecompressLzma,
   &OpSetEnv,
   &OpPostCreateProcess
};

CHAR InstDir[MAX_PATH];

/**
   Handler for console events.
*/
BOOL WINAPI ConsoleHandleRoutine(DWORD dwCtrlType)
{
   // Ignore all events. They will also be dispatched to the child procress (Ruby) which should
   // exit quickly, allowing us to clean up.
   return TRUE;
}

int main(int argc, char** argv)
{
   CHAR TempPath[MAX_PATH];
   GetTempPath(MAX_PATH, TempPath);
   GetTempFileName(TempPath, "ocrastub", 0, InstDir);
#ifdef _DEBUG
   printf("Temporary directory: %s\n", InstDir);
#endif

   SetConsoleCtrlHandler(&ConsoleHandleRoutine, TRUE);

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

   /* Set up environment */
   SetEnvironmentVariable("OCRA_EXECUTABLE", ImageFileName);

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
      if (!ProcessImage(lpv, FileSize))
         ExitStatus = -1;
      
      if (!UnmapViewOfFile(lpv))
         fprintf(stderr, "Failed to unmap view of executable.\n");
   }

   if (!CloseHandle(hMem))
      fprintf(stderr, "Failed to close file mapping.\n");

   if (!CloseHandle(hImage))
      fprintf(stderr, "Failed to close executable.\n");

   if (PostCreateProcess_ApplicationName && PostCreateProcess_CommandLine)
   {
      CreateAndWaitForProcess(PostCreateProcess_ApplicationName, PostCreateProcess_CommandLine);
   }

   /* Remove the temporary directory */
   SHFILEOPSTRUCT shop;
   shop.hwnd = NULL;
   shop.wFunc = FO_DELETE;
   InstDir[strlen(InstDir)+1] = 0;
   shop.pFrom = InstDir;
   shop.pTo = NULL;
   shop.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
   SHFileOperation(&shop);
#ifdef _DEBUG
   printf("Removing temporary files\n");
#endif

   ExitProcess(ExitStatus);

   /* Never gets here */
   return 0;
}

/**
   Process the image by checking the signature and locating the first
   opcode.
*/
BOOL ProcessImage(LPVOID ptr, DWORD size)
{
   LPVOID pSig = ptr + size - 4;
   if (memcmp(pSig, Signature, 4) == 0)
   {
#ifdef _DEBUG
      printf("Good signature found.\n");
#endif
      DWORD OpcodeOffset = *(DWORD*)(pSig - 4);
      LPVOID pSeg = ptr + OpcodeOffset;
      return ProcessOpcodes(&pSeg);
   }
   else
   {
      fprintf(stderr, "Bad signature in executable.\n");
      return FALSE;
   }
}

/**
   Process the opcodes in memory.
*/
BOOL ProcessOpcodes(READER* reader)
{
   while(!ExitCondition)
   {
      DWORD opcode = GetInteger(p);
      if (opcode < OP_MAX)
      {
         if (!OpcodeHandlers[opcode](p))
            return FALSE;
      }
      else
      {
         fprintf(stderr, "Invalid opcode '%lu'.\n", opcode);
         return FALSE;
      }
   }
   return TRUE;
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
BOOL OpCreateFile(ReadDataFunc pReadData)
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
BOOL OpCreateDirectory(ReadDataFunc pReadData)
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
      printf("Failed to create directory '%s'.\n", DirName);
      return FALSE;
   }
   
   return TRUE;
}

void GetCreateProcessInfo(LPVOID* p, LPTSTR* pApplicationName, LPTSTR* pCommandLine)
{
   LPTSTR ImageName = GetString(p);
   LPTSTR CmdLine = GetString(p);

   *pApplicationName = LocalAlloc(LMEM_FIXED, MAX_PATH);
   ExpandPath(*pApplicationName, ImageName);

   CHAR CmdLine2[MAX_PATH];
   ExpandPath(CmdLine2, CmdLine);

   LPTSTR MyCmdLine = GetCommandLine();
   LPTSTR MyArgs = SkipArg(MyCmdLine);
   
   *pCommandLine = LocalAlloc(LMEM_FIXED, strlen(CmdLine2) + 1 + strlen(MyArgs) + 1);
   strcpy(*pCommandLine, CmdLine2);
   strcat(*pCommandLine, " ");
   strcat(*pCommandLine, MyArgs);
}

/**
   Create a new process and wait for it to complete (OP_CREATE_PROCESS
   opcode handler)
*/
BOOL OpCreateProcess(ReadDataFunc pReadData)
{
   LPTSTR ApplicationName;
   LPTSTR CommandLine;
   GetCreateProcessInfo(p, &ApplicationName, &CommandLine);
   CreateAndWaitForProcess(ApplicationName, CommandLine);
   LocalFree(ApplicationName);
   LocalFree(CommandLine);
   return TRUE;
}

void CreateAndWaitForProcess(LPTSTR ApplicationName, LPTSTR CommandLine)
{
   
#ifdef _DEBUG
   printf("CreateProcess(%s, %s)\n", ApplicationName, CmdLine2);
#endif

   PROCESS_INFORMATION ProcessInformation;
   STARTUPINFO StartupInfo;
   ZeroMemory(&StartupInfo, sizeof(StartupInfo));
   StartupInfo.cb = sizeof(StartupInfo);
   BOOL r = CreateProcess(ApplicationName, CommandLine, NULL, NULL,
                          TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation);

   if (!r)
   {
      printf("Failed to create process (%s): %lu\n", ApplicationName, GetLastError());
   }

   WaitForSingleObject(ProcessInformation.hProcess, INFINITE);

   if (!GetExitCodeProcess(ProcessInformation.hProcess, &ExitStatus))
      fprintf(stderr, "Failed to get exit status (error %lu).\n", GetLastError());

   CloseHandle(ProcessInformation.hProcess);
   CloseHandle(ProcessInformation.hThread);
}

/**
 * Sets up a process to be created after all other opcodes have been processed. This can be used to create processes
 * after the temporary files have all been created and memory has been freed.
 */
BOOL OpPostCreateProcess(LPVOID* p)
{
   if (PostCreateProcess_ApplicationName || PostCreateProcess_CommandLine)
   {
      return FALSE;
   }
   else
   {
      GetCreateProcessInfo(p, &PostCreateProcess_ApplicationName, &PostCreateProcess_CommandLine);
      return TRUE;
   }
}


#if WITH_LZMA

BOOL OpDecompressLzma(ReadDataFunc pReadData)
{
   DWORD CompressedSize = GetInteger(p);
#ifdef _DEBUG
   printf("LzmaDecode(%ld)\n", CompressedSize);
#endif
   
   Byte* src = (Byte*)*p;
   *p += CompressedSize;

   UInt64 DecompressedSize = 0;
   int i;
   for (i = 0; i < 8; i++)
      DecompressedSize += (UInt64)src[LZMA_PROPS_SIZE + i] << (i * 8);

   // Byte* DecompressedData = LocalAlloc(LMEM_FIXED, DecompressedSize);
   Byte* DecompressedData = VirtualAlloc(NULL, DecompressedSize, MEM_COMMIT, PAGE_READWRITE);

   
   
   SizeT lzmaDecompressedSize = DecompressedSize;
   SizeT inSizePure = CompressedSize - LZMA_HEADER_SIZE;
   ELzmaStatus status;

   CLzmaDec state;
   SRes res;
   LzmaDec_Construct(&state);
   res = LzmaDec_Allocate(&state, src, LZMA_PROPS_SIZE, &alloc);
   if (res != SZ_OK)
      return FALSE;

   LzmaDec_Init(&state);
   DWORD offset = 0;
   DWORD BlockSize = 16384;
   DWORD SrcOffset = 0;

#define IN_BUF_SIZE (1 << 16)
#define OUT_BUF_SIZE (1 << 16)   

   BYTE* inBuf = src + LZMA_HEADER_SIZE;
   BYTE* outBuf = DecompressedData;
   
   size_t inPos = 0, inSize = 0, outPos = 0;
   inSize = inSizePure;
   LzmaDec_Init(&state);
   for (;;)
   {
     {
        SRes res;
        SizeT inProcessed = inSize - inPos;
        SizeT outProcessed = OUT_BUF_SIZE - outPos;
        ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
        ELzmaStatus status;
        if (outProcessed > DecompressedSize)
        {
           outProcessed = (SizeT)DecompressedSize;
           finishMode = LZMA_FINISH_END;
        }
        
        res = LzmaDec_DecodeToBuf(&state, outBuf + outPos, &outProcessed,
                                  inBuf + inPos, &inProcessed, finishMode, &status);

        fprintf(stderr, "Decoded %u to %u %d %d\n", inProcessed, outProcessed, res, status);
        inPos += inProcessed;
        outPos += outProcessed;
        DecompressedSize -= outProcessed;
        outBuf += outProcessed;
        outPos = 0;
        
        if (res != SZ_OK || DecompressedSize == 0)
           break;
        
        if (inProcessed == 0 && outProcessed == 0)
        {
           if (status != LZMA_STATUS_FINISHED_WITH_MARK)
              return FALSE; // SZ_ERROR_DATA;
           return res;
        }
     }
     if (inPos == inSize)
     {
        inBuf += IN_BUF_SIZE;
        inSize = IN_BUF_SIZE;
        inPos = 0;
     }
   }

   LzmaDec_Free(&state, &alloc);
   
   if (res != SZ_OK)
   {
      fprintf(stderr, "LZMA decompression failed.\n");
   }
   else
   {
      LPVOID decPtr = DecompressedData;
      ProcessOpcodes(&decPtr);
   }

   if (!VirtualFree(DecompressedData, DecompressedSize, MEM_DECOMMIT))
      fprintf(stderr, "Virtual free.. %lx?\n", GetLastError());

   // LocalFree(DecompressedData);
   return TRUE;
}
#endif

BOOL OpEnd(LPVOID* p)
{
   ExitCondition = TRUE;
   return TRUE;
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

