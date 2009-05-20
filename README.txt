= ocra

* http://rubyforge.org/projects/ocra/
* http://github.com/larsch/ocra/

== DESCRIPTION:

OCRA (One-Click Ruby Application) builds Windows executables from Ruby
source code. The executable is a self-extracting, self-running
executable that contains the Ruby interpreter, your source code and
any additionally needed ruby libraries or DLL.

== FEATURES/PROBLEMS:

* LZMA Compression (optional, default on)
* Windows support only
* Ruby 1.9 support
* Both console programs and desktop programs supported (no console will
  pop up with .rbw files).

== TODO:

* Clean up using manual recursive deletion (not SHop).

== SYNOPSIS:

ocra.rb [option] your/script.rb

* OCRA will load your script (using Kernel#load) and build the
  executable when it exits.

* Your program should 'require' all necessary files when invoked without
  arguments, so ocra can detect all dependencies.

* Autoloaded constants (e.g. modules and classes) should work. Ocra
  attempts to load all autoload definitions.

* Ocra does not set up the include path. Use "$:.unshift
  File.dirname(__FILE__)" at the start of your script if you need to
  'require' additional source files in the same directory no matter
  what the user's current working directory is.

* DLLs needs to be added manually (e.g. sqlite3.dll, gdbm.dll,
  etc.). Use the --dll option.

== REQUIREMENTS:

* Windows
* Working Ruby installation
* MinGW Installation (for building the stub)

== INSTALL:

=== Gem

* gem install ocra

Can also be downloaded from http://rubyforge.org/frs/?group_id=8185

=== Stand-alone

Get ocrasa.rb from http://rubyforge.org/frs/?group_id=8185. Requires
nothing but a working Ruby installation on Windows.

== TECHNICAL DETAILS

The Ocra stub extracts the contents into a temporary directory
(Windows' default temporary directory). The directory will contains
the same directory layout as your Ruby installlation. The source files
for your application will be put in the 'src' subdirectory.

Libraries found in non-standard path (for example, if you invoke Ocra
with "ruby -I some/path") will be placed into the site dir
(lib/ruby/site_ruby).

The RUBYLIB variable is cleared before your program is launched by the
executable in order not to interfere with any Ruby installation on the
end user's installation.

Ocra executables set the RUBYOPT environment variable is set to
whatever value is set when you first invoked ocra.rb to build the
executable. For example, if you had "RUBYOPT=rubygems" on your build
PC, Ocra ensures that it is also set on PC's running the executables.

Autoloaded constants will be attempted loaded when building the
executable. Modules that doesn't exist will be ignore (but a warning
will be logged.)

=== Working directory

You should not assume that the current working directory when invoking
an executable built with .exe is not the location of the source
script. It can be the directory where the executable is placed (when
invoked through the Windows Explorer), the users' current working
directory (when invoking from the Command Prompt), or even
C:\WINDOWS\SYSTEM32 when the executable is invoked through a file
association. You can optionally change the directory yourself:

   Dir.chdir(File.dirname(__FILE__))
  
=== $LOAD_PATH/$: mangling

Adding paths to $LOAD_PATH or $: at runtime is not recommended. Adding
relative load paths depends on the working directory being the same as
where the script is located (See above). If you have additional
library files in directories below the directory containing your
source script you can use this idiom:

   $LOAD_PATH.unshift File.join(File.dirname(__FILE__), 'path/to/script')

== CREDITS:

Thanks for Igor Pavlov for the LZMA compressor and decompressor. The
source code used was place into Public Domain by Igor Pavlov.

Erik Veenstra for rubyscript2exe which provided inspiration.

Dice for the default .exe icon (vit-ruby.ico,
http://ruby.morphball.net/vit-ruby-ico_en.html)

== LICENSE:

(The MIT License)

Copyright (c) 2009 Lars Christensen

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
