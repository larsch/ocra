/**
   Changes the Icon in a PE executable.
*/

#include <windows.h>
#include <stdio.h>

#pragma pack(push, 2)

/* Icon file header */
typedef struct
{
   WORD Reserved;
   WORD ResourceType;
   WORD ImageCount;
} IconFileHeader;

/* Icon File directory entry structure */
typedef struct
{
   BYTE Width;
   BYTE Height;
   BYTE Colors;
   BYTE Reserved;
   WORD Planes;
   WORD BitsPerPixel;
   DWORD ImageSize;
   DWORD ImageOffset;
} IconDirectoryEntry;

/* Group Icon Resource directory entry structure */
typedef struct
{
   BYTE Width;
   BYTE Height;
   BYTE Colors;
   BYTE Reserved;
   WORD Planes;
   WORD BitsPerPixel;
   DWORD ImageSize;
   WORD ResourceID;
} IconDirResEntry, *PIconDirResEntry;

/* Group Icon Structore (RT_GROUP_ICON) */
typedef struct
{
   WORD Reserved;
   WORD ResourceType;
   WORD ImageCount;
   IconDirResEntry Enries[0]; /* Number of these is in ImageCount */
} GroupIcon;

#pragma pack(pop)

BOOL UpdateIcon(LPTSTR ExecutableFileName, LPTSTR IconFileName)
{
   HANDLE h = BeginUpdateResource(ExecutableFileName, FALSE);
   if (h == INVALID_HANDLE_VALUE)
   {
      printf("Failed to BeginUpdateResource\n");
      return FALSE;
   }

   /* Read the Icon file */
   HANDLE hIconFile = CreateFile(IconFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
   if (hIconFile == INVALID_HANDLE_VALUE)
   {
      fprintf(stderr, "Failed to open icon file.\n");
      return FALSE;
   }
   DWORD Size = GetFileSize(hIconFile, NULL);
   BYTE* Data = LocalAlloc(LMEM_FIXED, Size);
   DWORD BytesRead;
   if (!ReadFile(hIconFile, Data, Size, &BytesRead, NULL))
   {
      fprintf(stderr, "Failed to read icon file.\n");
      return FALSE;
   }
   CloseHandle(hIconFile);
   
   IconFileHeader* header = (IconFileHeader*)Data;
   IconDirectoryEntry* entries = (IconDirectoryEntry*)(header + 1);
   
   /* Create the RT_ICON resources */
   int i;
   for (i = 0; i < header->ImageCount; ++i)
   {
      BOOL b = UpdateResource(h, MAKEINTRESOURCE(RT_ICON), MAKEINTRESOURCE(101 + i), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), Data + entries[i].ImageOffset, entries[i].ImageSize);
      if (!b)
      {
         fprintf(stderr, "failed to UpdateResource %lu\n", GetLastError());
         return FALSE;
      }
   }

   /* Create the RT_GROUP_ICON structure */
   DWORD GroupIconSize = sizeof(GroupIcon) + header->ImageCount * sizeof(IconDirectoryEntry);
   GroupIcon* gi = (GroupIcon*)LocalAlloc(LMEM_FIXED, GroupIconSize);
   gi->Reserved = 0;
   gi->ResourceType = header->ResourceType;
   gi->ImageCount = header->ImageCount;
   for (i = 0; i < header->ImageCount; ++i)
   {
      IconDirResEntry* e = &gi->Enries[i];
      e->Width = entries[i].Width;
      e->Height = entries[i].Height;
      e->Colors = entries[i].Colors;
      e->Reserved = entries[i].Reserved;
      e->Planes = entries[i].Planes;
      e->BitsPerPixel = entries[i].BitsPerPixel;
      e->ImageSize = entries[i].ImageSize;
      e->ResourceID = 101 + i;
   }

   /* Save the RT_GROUP_ICON resource */
   BOOL b = UpdateResource(h, MAKEINTRESOURCE(RT_GROUP_ICON), MAKEINTRESOURCE(100), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), gi, GroupIconSize);
   if (!b)
   {
      fprintf(stderr, "Failed to create group icon.\n");
      return FALSE;
   }

   if (!EndUpdateResource(h, FALSE))
   {
      fprintf(stderr, "Failed to EndUpdateResource.\n");
      return FALSE;
   }
   
   return TRUE;
}

int main(int argc, char* argv[])
{
   if (argc == 3)
   {
      if (UpdateIcon(argv[1], argv[2]))
         return 0;
      else
         return -1;
   }
   else
   {
      fprintf(stderr, "Usage: edicon.exe <exefile> <icofile>\n");
      return -1;
   }
}
