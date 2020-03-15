# ocra

home   :: https://github.com/larsch/ocra/

issues :: http://github.com/larsch/ocra/issues

forum  :: http://groups.google.com/group/ruby-ocra

## Description

OCRA (One-Click Ruby Application) builds Windows executables from Ruby
source code. The executable is a self-extracting, self-running
executable that contains the Ruby interpreter, your source code and
any additionally needed ruby libraries or DLL.

## Features

* LZMA Compression (optional, default on)
* Both windowed/console mode supported
* Includes gems based on usage, or from a Bundler Gemfile

## Problems & Bug Reporiting

* Windows support only

If you experience problems with OCRA or have found a bug, please use
the issue tracker on GitHub (http://github.com/larsch/ocra/issues).
You can also join the Google Group discussion forum to ask questions
and get help (http://groups.google.com/group/ruby-ocra).

## Installation

Gem:

    gem install ocra

Alternatively you can download the gem at either
http://rubygems.org/gems/ocra or
https://github.com/larsch/ocra/releases/.

Stand-alone Version: Get ocrasa.rb from
https://github.com/larsch/ocra/releases/. Requires nothing but a
working Ruby installation on Windows.

## Synopsis

### Building an executable:

    ocra script.rb

Will package `script.rb`, the Ruby interpreter and all
dependencies (gems and DLLs) into an executable named
`script.exe`.

### Command line:
  
    ocra [options] script.rb [<other files> ...] [-- <script arguments> ...]

### Options:

    ocra --help

Ocra options:

    --help             Display this information.
    --quiet            Suppress output while building executable.
    --verbose          Show extra output while building executable.
    --version          Display version number and exit.

Packaging options:

    --dll dllname      Include additional DLLs from the Ruby bindir.
    --add-all-core     Add all core ruby libraries to the executable.
    --gemfile <file>   Add all gems and dependencies listed in a Bundler Gemfile.
    --no-enc           Exclude encoding support files

Gem content detection modes:

    --gem-minimal[=gem1,..]  Include only loaded scripts
    --gem-guess=[gem1,...]   Include loaded scripts & best guess (DEFAULT)
    --gem-all[=gem1,..]      Include all scripts & files
    --gem-full[=gem1,..]     Include EVERYTHING
    --gem-spec[=gem1,..]     Include files in gemspec (Does not work with Rubygems 1.7+)

    --[no-]gem-scripts[=..]  Other script files than those loaded
    --[no-]gem-files[=..]    Other files (e.g. data files)
    --[no-]gem-extras[=..]   Extra files (README, etc.)

Gem modes:

* *minimal*: loaded scripts
* *guess*: loaded scripts and other files
* *all*: loaded scripts, other scripts, other files (except extras)
* *full*: Everything found in the gem directory

File groups:

* *scripts*: .rb/.rbw files  
* *extras*: C/C++ sources, object files, test, spec, README  
* *files*: all other files  

Auto-detection options:

    --no-dep-run       Don't run script.rb to check for dependencies.
    --no-autoload      Don't load/include script.rb's autoloads.
    --no-autodll       Disable detection of runtime DLL dependencies.

Output options:

    --output <file>    Name the exe to generate. Defaults to ./<scriptname>.exe.
    --no-lzma          Disable LZMA compression of the executable.
    --innosetup <file> Use given Inno Setup script (.iss) to create an installer.

Executable options:

    --windows          Force Windows application (rubyw.exe)  
    --console          Force console application (ruby.exe)  
    --chdir-first      When exe starts, change working directory to app dir.  
    --icon <ico>       Replace icon with a custom one.  
    --debug            Executable will be verbose.  
    --debug-extract    Executable will unpack to local dir and not delete after.  

  
### Compilation:

* OCRA will load your script (using `Kernel#load`) and build
  the executable when it exits.

* Your program should 'require' all necessary files when invoked without
  arguments, so OCRA can detect all dependencies.

* DLLs are detected automatically but only those located in your Ruby
  installation are included.

* .rb files will become console applications. .rbw files will become
  windowed application (without a console window popping
  up). Alternatively, use the `--console` or
  `--windows` options.

### Running your application:

* The 'current working directory' is not changed by OCRA when running
  your application. You must change to the installation or temporary
  directory yourself. See also below.
* When the application is running, the OCRA_EXECUTABLE environment
  variable points to the .exe (with full path).
* The temporary location of the script can be obtained by inspected
  the $0 variable.
* OCRA does not set up the include path. Use `$:.unshift
  File.dirname($0)` at the start of your script if you need to
  'require' additional source files from the same directory as your
  main script.

### Pitfalls:

* Avoid modifying load paths at run time. Specify load paths using -I
  or RUBYLIB if you must, but don't expect OCRA to preserve them for
  runtime. OCRA may pack sources into other directories than you
  expect.
* If you use .rbw files or the `--windows` option, then check
  that your application works with rubyw.exe before trying with OCRA.
* Avoid absolute paths in your code and when invoking OCRA.

## REQUIREMENTS:

* Windows
* Working Ruby installation.
* MinGW Installation (when working with the source code only)

### Stand-alone version

Get ocrasa.rb from http://rubyforge.org/frs/?group_id=8185. Requires
nothing but a working Ruby installation on Windows.

## Technical details

OCRA first runs the target script in order to detect any files that
are loaded and used at runtime (Using `Kernel#require` and
`Kernel#load`).

OCRA embeds everything needed to run a Ruby script into a single
executable file. The file contains the .exe stub which is compiled
from C-code, and a custom opcode format containing instructions to
create directories, save files, set environment variables and run
programs. The OCRA script generates this executable and the
instructions to be run when it is launched.

When executed, the OCRA stub extracts the Ruby interpreter and your
scripts into a temporary directory. The directory will contains the
same directory layout as your Ruby installlation. The source files for
your application will be put in the 'src' subdirectory.

### Libraries

Any code that is loaded through `Kernel#require` when your
script is executed will be included in the OCRA
executable. Conditionally loaded code will not be loaded and included
in the executable unless the code is actually run when OCRA invokes
your script. Otherwise, OCRA won't know about it and will not include
the source files.

RubyGems are handled specially. Whenever a file from a Gem is
detected, OCRA will attempt to include all the required files from
that specific Gem, expect some unlikely needed files such as readme's
and other documentation. This behaviour can be controlled by using the
--gem-* options. Behaviour can be changed for all gems or specific
gems using --gem-*=gemname.

Libraries found in non-standard path (for example, if you invoke OCRA
with "ruby -I some/path") will be placed into the site dir
(lib/ruby/site_ruby). Avoid changing `$LOAD_PATH` or
`$:` from your script to include paths outside your source
tree, since OCRA may place the files elsewhere when extracted into the
temporary directory.

In case your script (or any of its dependencies) sets up autoloaded
module using `Kernel#autoload`, OCRA will automatically try to
load them to ensure that they are all included in the
executable. Modules that doesn't exist will be ignored (a warning will
be logged).

Dynamic link libraries (.dll files, for example WxWidgets, or other
source files) will be detected and included by OCRA.

### Including libraries non-automatically

If an application or framework is complicated enough that it tends
to confuse Ocra's automatic dependency resolution, then you can
use other means to specify what needs to be packaged with your app.

To disable automatic dependency resolution, use the `--no-dep-run`
option; with it, Ocra will skip executing your program during the
build process. This on the other hand requires using `--gem-full` option
(see more below); otherwise Ocra will not include all the necessary
files for the gems.

You will also probably need to use the `--add-all-core` option to
include the Ruby core libraries.

If your app uses gems, then you can specify them in a
Bundler (http://gembundler.com) Gemfile, then use the --gemfile
option to supply it to Ocra. Ocra will automatically include all
gems specified, and all their dependencies.

(Note: This assumes that the gems are installed in your system,
*not* locally packaged inside the app directory by "bundle package")

These options are particularly useful for packaging Rails
applications.  For example, to package a Rails 3 app in the
directory "someapp" and create an exe named "someapp.exe", without
actually running the app during the build, you could use the
following command:

    ocra someapp/script/rails someapp --output someapp.exe --add-all-core \
    --gemfile someapp/Gemfile --no-dep-run --gem-full --chdir-first -- server

Note the space between `--` and `server`! It's important; `server` is
an argument to be passed to rails when the script is ran.

Rails 2 apps can be packaged similarly, though you will have to
integrate them with Bundler (http://gembundler.com/rails23.html)
first.

### Gem handling

By default, Ocra includes all scripts that are loaded by your script
when it is run before packaging. Ocra detects which gems are using and
includes any additional non-script files from those gems, except
trivial files such as C/C++ source code, object files, READMEs, unit
tests, specs, etc.

This behaviour can be changed by using the --gem-* options. There are
four possible modes:

* *minimal*: Include only loaded scripts
* *guess*: Include loaded scripts and important files (DEFAULT)
* *all*: Include all scripts and important files
* *full*: Include all files

If you find that files are missing from the resulting executable, try
first with --gem-all=gemname for the gem that is missing, and if that
does not work, try --gem-full=gemname. The paranoid can use --gem-full
to include all files for all required gems.

### Creating an installer for your application

To make your application start up quicker, or to allow it to
keep files in its application directory between runs, or if
you just want to make your program seem more like a "regular"
Windows application, you can have Ocra generate an installer
for your app with the free Inno Setup software.

You will first have to download and install Inno Setup 5 or
later, and also add its directory to your PATH (so that Ocra
can find the ISCC compiler program). Once you've done that,
you can use the `--innosetup` option to Ocra to supply an
Inno Setup script. Do not add any [Files] or [Dirs] sections
to the script; Ocra will figure those out itself.

To continue the Rails example above, let's package the Rails 3
app into an installer. Save the following as `someapp.iss`:

  [Setup]
  AppName=SomeApp
  AppVersion=0.1
  DefaultDirName={pf}\SomeApp
  DefaultGroupName=SomeApp
  OutputBaseFilename=SomeAppInstaller

  [Icons]
  Name: "{group}\SomeApp"; Filename: "{app}\someapp.exe"
  Name: "{group}\Uninstall SomeApp"; Filename: "{uninstallexe}"

Then, run Ocra with this command:

    ocra someapp/script/rails someapp --output someapp.exe --add-all-core \
    --gemfile someapp/Gemfile --no-dep-run --gem-full --chdir-first --no-lzma \
    --innosetup someapp.iss -- server

If all goes well, a file named "SomeAppInstaller.exe" will be placed
into the Output directory.

### Environment variables

OCRA executables clear the RUBYLIB environment variable before your
script is launched. This is done to ensure that your script does not
use load paths from the end user's Ruby installation.

OCRA executables set the RUBYOPT environment variable to the value it
had when you invoked OCRA. For example, if you had "RUBYOPT=rubygems"
on your build PC, OCRA ensures that it is also set on PC's running the
executables.

OCRA executables set OCRA_EXECUTABLE to the full path of the
executable, for example

    ENV["OCRA_EXECUTABLE"] # => C:\Program Files\MyApp\MyApp.exe

### Working directory

The OCRA executable does not change the working directory when it is
launched, unless you use the `--chdir-first` option.

You should not assume that the current working directory when invoking
an executable built with .exe is the location of the source script. It
can be the directory where the executable is placed (when invoked
through the Windows Explorer), the users' current working directory
(when invoking from the Command Prompt), or even
`C:\\WINDOWS\\SYSTEM32` when the executable is invoked through
a file association.

With the `--chdir-first` option, the working directory will
always be the common parent directory of your source files. This
should be fine for most applications. However, if your application
is designed to run from the command line and take filenames as
arguments, then you cannot use this option.

If you wish to maintain the user's working directory, but need to
`require` additional Ruby scripts from the source directory, you can
add the following line to your script:

    $LOAD_PATH.unshift File.dirname($0)

### Load path mangling

Adding paths to `$LOAD_PATH` or `$:` at runtime is not
recommended. Adding relative load paths depends on the working
directory being the same as where the script is located (See
above). If you have additional library files in directories below the
directory containing your source script you can use this idiom:

    $LOAD_PATH.unshift File.join(File.dirname($0), 'path/to/script')

### Detecting

You can detect whether OCRA is currently building your script by
looking for the 'Ocra' constant. If it is defined, OCRA is currenly
building the executable from your script. For example, you can use
this to avoid opening a GUI window when compiling executables:

    app = MyApp.new
    app.main_loop unless defined?(Ocra)

### Additional files and resources

You can add additional files to the OCRA executable (for example
images) by appending them to the command line. They should be placed
in the source directory with your main script (or a subdirectory).

    ocra mainscript.rb someimage.jpeg docs/document.txt

This will create the following layout in the temporary directory when
your program is executed:

    src/mainscript.rb
    src/someimage.jpeg
    src/docs/document.txt

Both files, directoriess and glob patterns can be specified on the
command line. Files will be added as-is. If a directory is specified,
OCRA will include all files found below that directory. Glob patterns
(See Dir.glob) can be used to specify a specific set of files, for
example:

    ocra script.rb assets/**/*.png

### Command Line Arguments

To pass command line argument to your script (both while building and
when run from the resulting executable), specify them after a
`--` marker. For example:

    ocra script.rb -- --some-options=value

This will pass `--some-options=value` to the script when
build and when running the executable. Any extra argument specified by
the user when invoking the executable will be appended after the
compile-time arguments.

### Window/Console

By default, OCRA builds console application from .rb-files and
windowed applications (without console window) from .rbw-files.

Ruby on Windows provides two executables: ruby.exe is a console mode
application and rubyw.exe is a windowed application which does not
bring up a console window when launched using the Windows Explorer.
By default, or if the `--console` option is used, OCRA will
use the console runtime (ruby.exe). OCRA will automatically select the
windowed runtime when your script has the ".rbw" extension, or if you
specify the `--windows` command line option.

If your application works in console mode but not in windowed mode,
first check if your script works without OCRA using rubyw.exe. A
script that prints to standard output (using puts, print etc.) will
eventually cause an exception when run with rubyw.exe (when the IO
buffers run full).

You can also try wrapping your script in an exception handler that
logs any errors to a file:

    begin
      # your script here
    rescue Exception => e
      File.open("except.log") do |f|
        f.puts e.inspect
        f.puts e.backtrace
      end
    end

## CREDITS:

Thanks for Igor Pavlov for the LZMA compressor and decompressor. The
source code used was place into Public Domain by Igor Pavlov.

Erik Veenstra for rubyscript2exe which provided inspiration.

Dice for the default .exe icon (vit-ruby.ico,
http://ruby.morphball.net/vit-ruby-ico_en.html)

## LICENSE:

(The MIT License)

Copyright (c) 2009-2020 Lars Christensen

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
