#ifndef _STUB_BUFFER_H
#define _STUB_BUFFER_H

typedef struct Buffer
{
   DWORD PageSize;
   DWORD PageCount;
   DWORD TotalBytes;
   HANDLE ReadSemaphore;
   HANDLE WriteSemaphore;
   LPVOID Pages;
} BUFFER;

void InitBuffer(BUFFER* buffer, DWORD PageSize, DWORD PageCount);

typedef struct
{
   BUFFER* buffer;
   DWORD BytesAvailable;
   DWORD ByteOffset;
} READER;

void WaitForData(READER* reader, DWORD Bytes);
void WaitForAnyData(READER* reader);
void ReleaseData(READER* reader, DWORD Bytes);
LPVOID GetReadBuffer(READER* reader, DWORD* BytesAvailable);

typedef struct
{
   BUFFER* buffer;
   DWORD BytesAvailable;
   DWORD ByteOffset;
} WRITER;

void InitWriter(WRITER* writer, BUFFER* buffer);
LPVOID* GetWriteBuffer(WRITER* writer, DWORD* size);
void WriteComplete(WRITER* writer, DWORD bytes);

#endif // _STUB_BUFFER_H
