/*
  Single Executable Bundle Stub

  This stub reads itself for embedded instructions to create directory
  and files in a temporary directory, launching a program.
*/

#include <windows.h>
#include <shellapi.h>
#include <string.h>
#include <tchar.h>
#include <stdio.h>

const BYTE Signature[] = { 0x41, 0xb6, 0xba, 0x4e };

#define OP_END 0
#define OP_CREATE_DIRECTORY 1
#define OP_CREATE_FILE 2
#define OP_CREATE_PROCESS 3
#define OP_DECOMPRESS_LZMA 4
#define OP_SETENV 5
#define OP_POST_CREATE_PROCRESS 6
#define OP_ENABLE_DEBUG_MODE 7
#define OP_CREATE_INST_DIRECTORY 8
#define OP_DELETE_INST_DIRECTORY 9
#define OP_MAX 10

BOOL ProcessImage(LPVOID p, DWORD size);
BOOL ProcessOpcodes(LPVOID* p);
void CreateAndWaitForProcess(LPTSTR ApplicationName, LPTSTR CommandLine);

BOOL OpEnd(LPVOID *p);
BOOL OpCreateFile(LPVOID *p);
BOOL OpCreateDirectory(LPVOID *p);
BOOL OpCreateProcess(LPVOID *p);
BOOL OpDecompressLzma(LPVOID *p);
BOOL OpSetEnv(LPVOID *p);
BOOL OpPostCreateProcess(LPVOID *p);
BOOL OpEnableDebugMode(LPVOID *p);
BOOL OpCreateInstDirectory(LPVOID *p);
BOOL OpDeleteInstDirectory(LPVOID *P);

#if WITH_LZMA
#include <LzmaDec.h>
#endif

typedef BOOL (*POpcodeHandler)(LPVOID*);

LPTSTR PostCreateProcess_ApplicationName = NULL;
LPTSTR PostCreateProcess_CommandLine = NULL;

DWORD ExitStatus = 0;
BOOL ExitCondition = FALSE;
BOOL DebugModeEnabled = FALSE;
TCHAR ImageFileName[MAX_PATH];

#if _CONSOLE
#define FATAL(...) { fprintf(stderr, "FATAL ERROR: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); }
#else
#define FATAL(...) { \
   TCHAR TextBuffer[1024]; \
   _sntprintf(TextBuffer, 1024, __VA_ARGS__); \
   MessageBox(NULL, TextBuffer, _T("OCRA"), MB_OK | MB_ICONWARNING); \
   }
#endif

#if _CONSOLE
#define DEBUG(...) { if (DebugModeEnabled) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } }
#else
#define DEBUG(...)
#endif

POpcodeHandler OpcodeHandlers[OP_MAX] = {
   &OpEnd,
   &OpCreateDirectory,
   &OpCreateFile,
   &OpCreateProcess,
#if WITH_LZMA
   &OpDecompressLzma,
#else
   NULL,
#endif
   &OpSetEnv,
   &OpPostCreateProcess,
   &OpEnableDebugMode,
   &OpCreateInstDirectory,
   &OpDeleteInstDirectory
};

TCHAR InstDir[MAX_PATH];

/** Decoder: Zero-terminated string */
LPTSTR GetString(LPVOID* p)
{
   LPTSTR str = *p;
   *p += lstrlen(str) + sizeof(TCHAR);
   return str;
}

/** Decoder: 32 bit unsigned integer */
DWORD GetInteger(LPVOID* p)
{
   DWORD dw = *(DWORD*)*p;
   *p += 4;
   return dw;
}

/**
   Handler for console events.
*/
BOOL WINAPI ConsoleHandleRoutine(DWORD dwCtrlType)
{
   // Ignore all events. They will also be dispatched to the child procress (Ruby) which should
   // exit quickly, allowing us to clean up.
   return TRUE;
}

BOOL OpCreateInstDirectory(LPVOID *p)
{
   DWORD LocalTestMode = GetInteger(p);

   /* Create an installation directory that will hold the extracted files */
   TCHAR TempPath[MAX_PATH];
   if (LocalTestMode) {
      // In debug mode, create the temp directory next to the exe
      strncpy(TempPath, ImageFileName, MAX_PATH);
      unsigned int i;
      for (i = strlen(TempPath)-1; i >= 0; --i) {
        if (TempPath[i] == '\\') {
          TempPath[i] = 0;
          break;
        }
      }
      if (strlen(TempPath) == 0) {
        FATAL("Unable to find directory containing exe");
        return FALSE;
      }
   } else {
      GetTempPath(MAX_PATH, TempPath);
   }
   GetTempFileName(TempPath, _T("ocrastub"), 0, InstDir);
   DEBUG("Creating installation directory: '%s'", InstDir);

   /* Attempt to delete the temp file created by GetTempFileName.
      Ignore errors, i.e. if it doesn't exist. */
   (void)DeleteFile(InstDir);

   if (!CreateDirectory(InstDir, NULL)){
      FATAL("Failed to create installation directory.");
      return FALSE;
   }
   return TRUE;
}

BOOL OpDeleteInstDirectory(LPVOID *p)
{
   DEBUG("Deleting temporary installation directory %s", InstDir);
   SHFILEOPSTRUCT shop;
   shop.hwnd = NULL;
   shop.wFunc = FO_DELETE;
   InstDir[lstrlen(InstDir) + sizeof(TCHAR)] = 0;
   shop.pFrom = InstDir;
   shop.pTo = NULL;
   shop.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
   SHFileOperation(&shop);
   return TRUE;
}

int CALLBACK _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
   /* Find name of image */
   if (!GetModuleFileName(NULL, ImageFileName, MAX_PATH)) {
      FATAL("Failed to get executable name (error %lu).", GetLastError());
      return -1;
   }
   
   /* Set up environment */
   SetEnvironmentVariable(_T("OCRA_EXECUTABLE"), ImageFileName);
   
   SetConsoleCtrlHandler(&ConsoleHandleRoutine, TRUE);

   /* Open the image (executable) */
   HANDLE hImage = CreateFile(ImageFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
   if (hImage == INVALID_HANDLE_VALUE) {
      FATAL("Failed to open executable (%s)", ImageFileName);
      return -1;
   }      

   /* Create a file mapping */
   DWORD FileSize = GetFileSize(hImage, NULL);
   HANDLE hMem = CreateFileMapping(hImage, NULL, PAGE_READONLY, 0, FileSize, NULL);
   if (hMem == INVALID_HANDLE_VALUE) {
      FATAL("Failed to create file mapping (error %lu)", GetLastError());
      CloseHandle(hImage);
      return -1;
   }

   /* Map the image into memory */
   LPVOID lpv = MapViewOfFile(hMem, FILE_MAP_READ, 0, 0, 0);
   if (lpv == NULL)
   {
      FATAL("Failed to map view of executable into memory (error %lu).", GetLastError());
   }
   else
   {
      if (!ProcessImage(lpv, FileSize))
         ExitStatus = -1;
      
      if (!UnmapViewOfFile(lpv))
         FATAL("Failed to unmap view of executable.");
   }

   if (!CloseHandle(hMem))
      FATAL("Failed to close file mapping.");

   if (!CloseHandle(hImage))
      FATAL("Failed to close executable.");

   if (PostCreateProcess_ApplicationName && PostCreateProcess_CommandLine)
   {
      DEBUG("**********");
      DEBUG("Starting app in: %s", InstDir);
      DEBUG("**********");
      CreateAndWaitForProcess(PostCreateProcess_ApplicationName, PostCreateProcess_CommandLine);
   }

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
      DEBUG("Good signature found.");
      DWORD OpcodeOffset = *(DWORD*)(pSig - 4);
      LPVOID pSeg = ptr + OpcodeOffset;
      return ProcessOpcodes(&pSeg);
   }
   else
   {
      FATAL("Bad signature in executable.");
      return FALSE;
   }
}

/**
   Process the opcodes in memory.
*/
BOOL ProcessOpcodes(LPVOID* p)
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
         FATAL("Invalid opcode '%lu'.", opcode);
         return FALSE;
      }
   }
   return TRUE;
}

/**
   Expands a specially formatted string, replacing \xFF with the
   temporary installation directory.
*/
void ExpandPath(LPTSTR* out, LPTSTR str)
{
   DWORD OutSize = lstrlen(str) + sizeof(TCHAR);
   LPTSTR a = str;
   while ((a = _tcschr(a, L'\xFF')))
   {
      OutSize += lstrlen(InstDir) - sizeof(TCHAR);
      a++;
   }

   *out = LocalAlloc(LMEM_FIXED, OutSize);
   
   LPTSTR OutPtr = *out;
   while ((a = _tcschr(str, '\xFF')))
   {
      int l = a - str;
      if (l > 0) {
         memcpy(OutPtr, str, l);
         OutPtr += l;
         str += l;
      }
      str += sizeof(TCHAR);
      lstrcpy(OutPtr, InstDir);
      OutPtr += lstrlen(OutPtr);
   }
   lstrcpy(OutPtr, str);
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

   TCHAR Fn[MAX_PATH];
   lstrcpy(Fn, InstDir);
   lstrcat(Fn, _T("\\"));
   lstrcat(Fn, FileName);
   
   DEBUG("CreateFile(%s, %lu)", Fn, FileSize);
   HANDLE hFile = CreateFile(Fn, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
   if (hFile != INVALID_HANDLE_VALUE)
   {
      DWORD BytesWritten;
      if (!WriteFile(hFile, Data, FileSize, &BytesWritten, NULL))
      {
         FATAL("Write failure (%lu)", GetLastError());
         Result = FALSE;
      }
      if (BytesWritten != FileSize)
      {
         FATAL("Write size failure");
         Result = FALSE;
      }
      CloseHandle(hFile);
   }
   else
   {
      FATAL("Failed to create file '%s'", Fn);
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

   TCHAR DirName[MAX_PATH];
   lstrcpy(DirName, InstDir);
   lstrcat(DirName, _T("\\"));
   lstrcat(DirName, DirectoryName);
   
   DEBUG("CreateDirectory(%s)", DirName);
   
   if (!CreateDirectory(DirName, NULL)){
      if (GetLastError() == ERROR_ALREADY_EXISTS) {
        DEBUG("Directory already exists");
      } else {
        FATAL("Failed to create directory '%s'.", DirName);
        return FALSE;
      }
   }
   
   return TRUE;
}

void GetCreateProcessInfo(LPVOID* p, LPTSTR* pApplicationName, LPTSTR* pCommandLine)
{
   LPTSTR ImageName = GetString(p);
   LPTSTR CmdLine = GetString(p);

   ExpandPath(pApplicationName, ImageName);

   LPTSTR ExpandedCommandLine;
   ExpandPath(&ExpandedCommandLine, CmdLine);

   LPTSTR MyCmdLine = GetCommandLine();
   LPTSTR MyArgs = SkipArg(MyCmdLine);
   
   *pCommandLine = LocalAlloc(LMEM_FIXED, lstrlen(ExpandedCommandLine) + sizeof(TCHAR) + lstrlen(MyArgs) + sizeof(TCHAR));
   lstrcpy(*pCommandLine, ExpandedCommandLine);
   lstrcat(*pCommandLine, _T(" "));
   lstrcat(*pCommandLine, MyArgs);
   
   LocalFree(ExpandedCommandLine);
}

/**
   Create a new process and wait for it to complete (OP_CREATE_PROCESS
   opcode handler)
*/
BOOL OpCreateProcess(LPVOID *p)
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
   PROCESS_INFORMATION ProcessInformation;
   STARTUPINFO StartupInfo;
   ZeroMemory(&StartupInfo, sizeof(StartupInfo));
   StartupInfo.cb = sizeof(StartupInfo);
   BOOL r = CreateProcess(ApplicationName, CommandLine, NULL, NULL,
                          TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation);

   if (!r)
   {
      FATAL("Failed to create process (%s): %lu", ApplicationName, GetLastError());
      return;
   }

   WaitForSingleObject(ProcessInformation.hProcess, INFINITE);

   if (!GetExitCodeProcess(ProcessInformation.hProcess, &ExitStatus))
      FATAL("Failed to get exit status (error %lu).", GetLastError());

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

BOOL OpEnableDebugMode(LPVOID* p)
{
  DebugModeEnabled = TRUE;
  return TRUE;
}

#if WITH_LZMA
void *SzAlloc(void *p, size_t size) { p = p; return LocalAlloc(LMEM_FIXED, size); }
void SzFree(void *p, void *address) { p = p; LocalFree(address); }
ISzAlloc alloc = { SzAlloc, SzFree };

#define LZMA_UNPACKSIZE_SIZE 8
#define LZMA_HEADER_SIZE (LZMA_PROPS_SIZE + LZMA_UNPACKSIZE_SIZE)

BOOL OpDecompressLzma(LPVOID *p)
{
   BOOL Success = TRUE;
   
   DWORD CompressedSize = GetInteger(p);
   DEBUG("LzmaDecode(%ld)", CompressedSize);
   
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
      FATAL("LZMA decompression failed.");
      Success = FALSE;
   }
   else
   {
      LPVOID decPtr = DecompressedData;
      if (!ProcessOpcodes(&decPtr))
         Success = FALSE;
   }

   LocalFree(DecompressedData);
   return Success;
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
   LPTSTR ExpandedValue;
   ExpandPath(&ExpandedValue, Value);
   DEBUG("SetEnv(%s, %s)", Name, ExpandedValue);

   BOOL Result = FALSE;
   if (!SetEnvironmentVariable(Name, ExpandedValue))
   {
      FATAL("Failed to set environment variable (error %lu).", GetLastError());
      Result = FALSE;
   }
   else
   {
      Result = TRUE;
   }
   LocalFree(ExpandedValue);
   return Result;
}
