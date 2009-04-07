= ocra

* FIX (url)

== DESCRIPTION:

OCRA (One-Click Ruby Application) builds Windows executables from Ruby
source code. The executable is a self-extracting, self-running package
that contains the Ruby interpreter, your source code and any
additional ruby libraries or DLL.

== FEATURES/PROBLEMS:

* FIX (list of features or problems)

* Your program should 'require' all necessary files when invoked
  without arguments, so ocra can detect all dependencies. Autoloaded
  constants should work.

* Ocra does not set up a include path. Use "$:.unshift
  File.dirname(__FILE__)" at the start of your script if you need to
  'require' additional source files in the same directory no matter
  what the user's current working directory is.

* DLLs needs to be added manually (e.g. sqlite3.dll, gdbm.dll,
  etc.). Use the --dll option.

== TECHINICAL DETAILS

* Library files from your Ruby installation are included in the same
  path (relative to the installation root). Ruby's default search
  paths will find them there.

* Library files from custom paths additional (e.g. ruby -I some/path)
  will be placed into the site dir (lib/ruby/site_ruby).

* The RUBYOPT and RUBYLIB variables are cleared before your program is
  launched by the executable in order not to interfere with any Ruby
  installation on the end user's installation.

* Autoloaded constants will be attempted loaded when building the
  executable. Modules that doesn't exist will be ignore (but a warning
  will be logged.)
  
== TODO:

* Handle environment variables (RUBYOPT, RUBYLIB).
* No-console stub.
* Clean up using manual recursive deletion (not SHop).
* Ensure cleanup using PendingFileRenameOperations

== SYNOPSIS:

  FIX (code sample of usage)

== REQUIREMENTS:

* Windows

== INSTALL:

* FIX (sudo gem install, anything else)

== LICENSE:

(The MIT License)

Copyright (c) 2009 FIX

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
'Software'), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
