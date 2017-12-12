Ocra & Code signing in Windows
===============

In order to distribute our app on Windows we use the [Ocra](https://github.com/larsch/ocra) project to bundle our Ruby source files together with the Ruby interpreter and libraries to form a single executable.

Further, and as part of our build process we want to digitally sign our files. [Code signing](https://msdn.microsoft.com/en-us/library/ms537361(v=vs.85).aspx) achieves two things: (1) It establishes our identity as a developer and (2) It ensures integrity of our code (ensuring that it hasn't
been tampered with). Also, since it establishes us as a 'trusted developer' it reduces interference from Anti-Virus software and so on.

Unfortunately, due to their fragile nature, Ocra executables are broken by the code signing process and this has prevented us from signing our
app until now.

In this document I'll describe how Ocra works, how code signing in Windows works and finally an approach to get code signing working with Ocra.

(TL;DR given at the end of the document).

## How Ocra Works

At a high level Ocra works by writing custom data after the end of the executable image -- and then reading this custom data into memory at runtime. The custom data contains "opcodes" and embedded Ruby source files. The opcodes contain instructions like "create temp directory", "create file" (i.e extract embedded Ruby file to a temp folder on disk), "create process" and so on. It is this runtime execution of opcodes that allow the magic of Ocra to happen.

### Building the executable

It's a [known trick](https://edn.embarcadero.com/article/27979) that data written past the end of a Windows executable will be ignored by the Kernel, and Ocra takes advantage of this to store Ruby files as well as instructions for how to extract and run those Ruby files.

In more detail, Ocra does the following when building an executable for a Ruby app:

* First, the [stub.c](https://github.com/larsch/ocra/blob/master/src/stub.c) file is compiled into a `stub.exe`. This program only does two things -- it maps itself into memory and then
executes the opcodes stored in its 'custom data' section.
* Ocra then creates the output executable (e.g `hello_world.exe`) and [writes `stub.exe`](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/bin/ocra#L1021-L1032) to the start of the file.
* The ["opcodes"](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/bin/ocra#L1009-L1016)  are appended to the end of the output executable.  The Ruby source files (and Ruby interpreter binary itself)  are embedded as long string parameters associated with the opcodes.
* For example, the Ruby interpreter (`ruby.exe`) is [embedded as one long string](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/bin/ocra#L901) as the [first parameter](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/bin/ocra#L1161) to the `OP_CREATEFILE` opcode.
* After writing the opcodes, and [a final `OP_END`](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/bin/ocra#L1076)  instruction that indicates the end of the opcodes, Ocra
writes an [`opcodes_offset`](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/bin/ocra#L1077) value. This value is a relative offset to the start of the opcodes that tells the
program where to start processing.
* Finally, after the offset, are the [last 4 bytes of the file](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/bin/ocra#L1078). These bytes form Ocra's own "signature" and they are a [hard-coded value](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/src/stub.c#L13). This signature is **not** the same as the "digital signature" inserted by code signing and likely exists to check the executable has not been corrupted.

After Ocra has finished building the output executable, the file will look like the following:

```
| stub.exe content | opcodes | op_end opcode | opcodes offset | ocra signature |
```

To reiterate, only the `stub.exe` content is used by the Kernel, everything else is ignored. The opcodes and so on are only
accessed by the program itself once it is mapped into memory.

### Running the executable

Once we have an output executable with the structure shown in the previous diagram, Ocra can execute it. When the executable
is run the process is as follows:

* It opens its associated file on disk for reading and [maps itself into memory](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/src/stub.c#L309)
* It navigates to the end of the mapping and examines the [final 4 bytes](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/src/stub.c#L375-L376) -- these bytes are the "Ocra signature"
* It verifies that these bytes are equal to the [hard-coded signature](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/src/stub.c#L13) (if they are not then it will exit with a ["Bad Executable"](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/src/stub.c#L385) error message)
* Once the signature is validated, it reads the [4 bytes prior to the signature](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/src/stub.c#L379) -- these bytes are the `opcodes_offset`
* It [start processing](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/src/stub.c#L381) the opcodes from the `opcodes_offset` until the [`OP_END` instruction is reached](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/src/stub.c#L678). It processes the opcodes by looking up the [associated function pointer](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/src/stub.c#L72-L87) for that opcode and executing it.

These opcodes include instructions such as ["create a temporary file"](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/src/stub.c#L469-L506) (i.e extract an embedded Ruby source file to disk) and ["create and run a process"](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/src/stub.c#L604-L623) (i.e run the Ruby source file with the embedded Ruby interpreter binary).

As is clear from above, the Ocra binary is quite a fragile thing. If the last 4 bytes are not the hard-coded signature it expects, then it will error out.

## How Code Signing Works

A signed file is a standard Windows executable with signature data appended
to it. A standard Windows executable file is known as a "PE" file ([Portable Execution format](https://en.wikipedia.org/wiki/Portable_Executable)).

The structure of a PE file is as follows (image taken from https://en.wikipedia.org/wiki/Portable_Executable):

![PE Format](https://www.dropbox.com/s/zds11h7e282vlv6/Screenshot%202017-12-12%2018.02.16.png?raw=1)

From the diagram we see the format starts with a legacy MSDOS header. This header is represented by the [IMAGE_DOS_HEADER struct](https://www.nirsoft.net/kernel_struct/vista/IMAGE_DOS_HEADER.html) - which has  a member called `e_lfanew` which stands for "long file address for the New Executable" and it contains the offset (known as an RVA - Relative Virtual Address) to the start of the PE header proper.

From this offset we get to the the PE header (also known as an `NT header` or `COFF header`) and it is a structure of type [IMAGE_NT_HEADERS](https://msdn.microsoft.com/en-us/library/windows/desktop/ms680336(v=vs.85).aspx). It is followed by another header which contains information on executables (referred to as an [optional header](https://msdn.microsoft.com/en-us/library/windows/desktop/ms680339(v=vs.85).aspx), but it's always present for executables).  This header defines [a number of data directories](http://bytepointer.com/resources/pietrek_in_depth_look_into_pe_format_pt1_figures.htm).

One of these directory entries, `IMAGE_DIRECTORY_ENTRY_SECURITY`, is used to store [information](https://msdn.microsoft.com/en-us/library/windows/desktop/ms680305(v=vs.85).aspx) about the
signature. Specifically it stores the location of the signature in the file (in the `VirtualAddress` field) and also its size (in the `Size` field).

The digital signature itself is just a hash of the file that is signed with the developer's private key - and it is appended to the end of the executable.

Because the signature appears after the EOF of the proper executable it is not treated as "executable code" (recall the [custom data](https://edn.embarcadero.com/article/27979) trick).

To summarize, after code signing, an executable will change in the following way:

* The `IMAGE_DIRECTORY_ENTRY_SECURITY` element in the PE header will have its `Size` field changed from zero (as is the case of an unsigned PE) to the size of the digital signature.
* The `VirtualAddress` field will be updated to point to the location of the digital signature.
* The digital signature will be appended to the executable.

## Why Code Signing Breaks Ocra Executables

Ocra expects the last 4 bytes of the executable to be its own hard-coded signature. However, since the Code Signing process appends the digital signature to the end of the executable this is no longer true. As a result attempting to run a Code Signed Ocra
executable will result in the ["Bad Signature in Executable"](https://github.com/larsch/ocra/blob/9c5fe287887f16db0c00ad54a549ff383f4c8d91/src/stub.c#L385) error message.

## Suggested Fix

The proposed fix is conceptually quite simple: we teach Ocra to know when it's been signed and to look in a different place for its expected data (its own signature and its `opcodes_offset`, etc).

This means Ocra now has two code paths:

### Unsigned codepath

(`IMAGE_DIRECTORY_ENTRY_SECURITY.Size == 0`)

The executable layout looks exactly as Ocra expects, nothing changes:

`| stub.exe content | opcodes | op_end opcode | opcodes offset | ocra signature |`

### Signed codepath

(`IMAGE_DIRECTORY_ENTRY_SECURITY.Size != 0`)

The layout has changed slightly, we  have a digital signature following the ocra signature:

`| stub.exe content | opcodes | op_end opcode | opcodes offset | ocra signature | dig sig |`

We  must look at the `IMAGE_DIRECTORY_ENTRY_SECURITY.VirtualAddress` header to locate the starting byte of the digital signature. This starting byte will be immediately after the Ocra signature so we tell
Ocra to go there for the Ocra signature instead of the end of the file, as it did previously. From there
Ocra is able to find the Ocra signature and also the `opcodes_offset` field (which appears immediately before) and start processing opcodes.

### Issues

Unfortunately, the Code Signing process does not append the Digital Signature immediately after the Ocra Signature. There are usually a few NUL bytes in between the two. These NUL bytes are likely for alignment reasons that are not documented. Nonetheless, doing a small search back from the `VirtualAddress` (start of Digital Signature) until it finds the first non-NUL bytes appears to be an effective workaround and I have not noticed any problems with this approach on multiple test files. We do need to ensure the last byte of the Ocra signature is not a NUL byte though, but since the Ocra signature is hard-coded and completely under our control this is a non-issue.

## Conclusion

I've tested the above approach on a number of files (including our code base) and it has worked successfully.

I recommend we open a PR upstream and get the changes merged in as other developer's are likely to also want
Ocra Code Signing support.

## TL;DR

Ocra expects the executable to look like this:

```
| stub.exe content | opcodes | op_end opcode | opcodes offset | ocra signature |
```

But, after Code Signing it looks like this instead:

```
| stub.exe content | opcodes | op_end opcode | opcodes offset | ocra signature | dig sig |
```

This breaks Ocra and prevents Ocra executables working after they've been signed.

The proposed fix is to update [stub.c](https://github.com/larsch/ocra/blob/master/src/stub.c) to use the executable headers to locate where the Digital Signature starts and tell Ocra to look for its data backwards from there instead of from the end of the file.

## Resources

* [Peering Inside the PE](https://msdn.microsoft.com/en-us/library/ms809762.aspx)
* [Authenticode Reverse-engineered](https://www.cs.auckland.ac.nz/~pgut001/pubs/authenticode.txt)
* [Winnt.h](http://www.rensselaer.org/dept/cis/software/g77-mingw32/include/winnt.h)
* [PE Format (MSDN)](https://msdn.microsoft.com/en-us/library/windows/desktop/ms680547(v=vs.85).aspx)
* [Portable Execution Format Wikipedia Page](https://en.wikipedia.org/wiki/Portable_Executable)
* [Introduction To Code Signing](https://msdn.microsoft.com/en-us/library/ms537361(v=vs.85).aspx)
