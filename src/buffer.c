#include "buffer.h"

void InitBuffer(BUFFER* buffer, DWORD PageSize, DWORD PageCount)
{
   buffer->PageSize = PageSize;
   buffer->PageCount = PageCount;
   buffer->TotalBytes = PageSize * PageCount;
   buffer->ReadSemaphore = CreateSemaphore(NULL, 0, PageCount, NULL);
   buffer->WriteSemaphore = CreateSemaphore(NULL, PageCount, PageCount, NULL);
}

void WaitForData(READER* reader, DWORD Bytes)
{
   while (reader->BytesAvailable < Bytes)
   {
      WaitForSingleObject(reader->buffer->ReadSemaphore, INFINITE);
      reader->BytesAvailable += reader->buffer->PageSize;
   }
}

void WaitForAnyData(READER* reader)
{
   WaitForData(reader, 1);
}

LPVOID GetReadBuffer(READER* reader, DWORD* BytesAvailable)
{
   WaitForAnyData(reader);
   *BytesAvailable = min(reader->BytesAvailable, reader->buffer->TotalBytes - reader->ByteOffset);
   return reader->buffer->Pages + reader->ByteOffset;
}

void ReleaseData(READER* reader, DWORD Bytes)
{
   reader->BytesAvailable -= Bytes;
   reader->ByteOffset = (reader->ByteOffset + Bytes) % (reader->buffer->PageSize * reader->buffer->PageCount);
   DWORD PagesReleased = (reader->ByteOffset % reader->buffer->PageSize + Bytes) / reader->buffer->PageSize;
   ReleaseSemaphore(reader->buffer->WriteSemaphore, PagesReleased, NULL);
}

void InitWriter(WRITER* writer, BUFFER* buffer)
{
   writer->buffer = buffer;
   writer->ByteOffset = 0;
   writer->BytesAvailable = 0;
}

LPVOID* GetPage(WRITER* writer)
{
   WaitForSingleObject(writer->buffer->WriteSemaphore, INFINITE);
   LPVOID page = writer->buffer->Pages + writer->NextPageOffset;
   writer->NextPageOffset += writer->buffer->PageSize;
   return page;
}

void WriteComplete(WRITER* writer, DWORD bytes)
{
   ReleaseSemaphore(writer->buffer->WriteSemaphore, 1, NULL);
}

