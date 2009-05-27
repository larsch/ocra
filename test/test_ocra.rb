require "test/unit"
require "ocra"
require "tmpdir"
require "fileutils"
require "rbconfig"
include FileUtils

class TestOcra < Test::Unit::TestCase

  DefaultArgs = [ '--quiet', '--no-lzma' ]

  TESTED_OCRA = ENV['TESTED_OCRA'] || 'ocra.rb'

  def initialize(*args)
    super(*args)
    @testnum = 0
    @ocra = File.expand_path(File.join(File.dirname(__FILE__), '..', 'bin', TESTED_OCRA))
    ENV['RUBYOPT'] = ""
  end

  def ocra
    @ocra
  end

  OcraRoot = File.expand_path(File.join(File.dirname(__FILE__), '..'))

  FixturePath = File.expand_path(File.join(File.dirname(__FILE__), 'fixtures'))

  # Sets up an directory with a copy of a fixture and yields to the
  # block, then cleans up everything. A fixture here is a hierachy of
  # files located in test/fixtures.
  def with_fixture(name)
    path = File.join(FixturePath, name)
    FileUtils.cp_r path, '.'
    begin
      cd name do
        yield
      end
    ensure
      rm_rf 'name'
    end
  end

  # Sets up temporary environment variable and yields to the block.
  def with_env(hash)
    old = {}
    hash.each do |k,v|
      old[k] = ENV[k]
      ENV[k] = v
    end
    begin
      yield
    ensure
      hash.each do |k,v|
        ENV[k] = old[k]
      end
    end
  end
  
  def setup
    @testnum += 1
    @tempdirname = ".ocratest-#{$$}-#{@testnum}"
    Dir.mkdir @tempdirname
    Dir.chdir @tempdirname
  end

  def teardown
    Dir.chdir '..'
    FileUtils.rm_rf @tempdirname
  end
  
  def test_helloworld
    with_fixture 'helloworld' do
      assert system("ruby", ocra, "helloworld.rb", *DefaultArgs)
      assert File.exist?("helloworld.exe")
      assert system("helloworld.exe")
    end
  end

  def test_writefile
    with_fixture 'writefile' do
      assert system("ruby", ocra, "writefile.rb", *DefaultArgs)
      assert File.exist?("writefile.exe")
      assert system("writefile.exe")
      assert File.exist?("output.txt")
      assert "output", File.read("output.txt")
    end
  end

  def test_exitstatus
    with_fixture 'exitstatus' do
      assert system("ruby", ocra, "exitstatus.rb", *DefaultArgs)
      system("exitstatus.exe")
      assert_equal 167, $?.exitstatus
    end
  end

  def test_arguments
    with_fixture 'arguments' do
      assert system("ruby", ocra, "arguments.rb", *DefaultArgs)
      assert File.exist?("arguments.exe")
      system("arguments.exe foo \"bar baz\"")
      assert_equal 5, $?.exitstatus
    end
  end

  def test_stdout_redir
    with_fixture 'stdoutredir' do
      assert system("ruby", ocra, "stdoutredir.rb", *DefaultArgs)
      assert File.exist?("stdoutredir.exe")
      system("stdoutredir.exe > output.txt")
      assert File.exist?("output.txt")
      assert_equal "Hello, World!\n", File.read("output.txt")
    end
  end

  def test_stdin_redir
    with_fixture 'stdinredir' do
      assert system("ruby", ocra, "stdinredir.rb", *DefaultArgs)
      assert File.exist?("stdinredir.exe")
      system("stdinredir.exe < input.txt")
      assert 104, $?.exitstatus
    end
  end

  def test_gdbmdll
    with_fixture 'gdbmdll' do
      assert system("ruby", ocra, "gdbmdll.rb", *DefaultArgs)
      with_env 'PATH' => '.' do
        system("gdbmdll.exe")
        assert_equal 104, $?.exitstatus
      end
    end
  end

  def test_relative_require
    with_fixture 'relativerequire' do
      assert system("ruby", ocra, "relativerequire.rb", *DefaultArgs)
      assert File.exist?("relativerequire.exe")
      system("relativerequire.exe")
      assert_equal 160, $?.exitstatus
    end
  end

  # Test that autoloaded files which are not actually loaded while
  # running the script through Ocra are included in the resulting
  # executable.
  def test_autoload
    with_fixture 'autoload' do
      assert system("ruby", ocra, "autoload.rb", *DefaultArgs)
      assert File.exist?("autoload.exe")
      File.unlink('foo.rb')
      assert system("autoload.exe")
    end
  end

  # Test that autoload statement which point to non-existing files are
  # ignored by Ocra (a warning may be logged).
  def test_autoload_missing
    with_fixture 'autoloadmissing' do
      assert system("ruby", ocra, "autoloadmissing.rb", *DefaultArgs)
      assert File.exist?("autoloadmissing.exe")
      assert system("autoloadmissing.exe")
    end
  end

  # Test that Ocra picks up autoload statement nested in modules.
  def test_autoload_nested
    with_fixture 'autoloadnested' do
      assert system("ruby", ocra, "autoloadnested.rb", *DefaultArgs)
      assert File.exist?("autoloadnested.exe")
      File.unlink('foo.rb')
      assert system("autoloadnested.exe")
    end
  end

  # Test that we can use custom include paths when invoking Ocra (ruby
  # -I somepath). In this case the lib scripts are put in the src/
  # directory.
  def test_relative_loadpath1_ilib
    with_fixture 'relloadpath1' do
      assert system('ruby', '-I', 'lib', ocra, 'relloadpath1.rb', *DefaultArgs)
      assert File.exist?('relloadpath1.exe')
      assert system('relloadpath1.exe')
    end
  end

  # Same as above with './lib'
  def test_relative_loadpath_idotlib
    with_fixture 'relloadpath1' do
      assert system('ruby', '-I', './lib', ocra, 'relloadpath1.rb', *DefaultArgs)
      assert File.exist?('relloadpath1.exe')
      assert system('relloadpath1.exe')
    end
  end

  # Test that we can use custom include paths when invoking Ocra (env
  # RUBYLIB=lib). In this case the lib scripts are put in the src/
  # directory.
  def test_relative_loadpath_rubyliblib
    with_fixture 'relloadpath1' do
      with_env 'RUBYLIB' => 'lib' do
        assert system('ruby', ocra, 'relloadpath1.rb', *DefaultArgs)
        assert File.exist?('relloadpath1.exe')
        assert system('relloadpath1.exe')
      end
    end
  end

  # Same as above with './lib'
  def test_relative_loadpath_rubylibdotlib
    with_fixture 'relloadpath1' do
      with_env 'RUBYLIB' => './lib' do
        assert system('ruby', ocra, 'relloadpath1.rb', *DefaultArgs)
        assert File.exist?('relloadpath1.exe')
        assert system('relloadpath1.exe')
      end
    end
  end

  # Relative path with .. prefix (../lib).
  def test_relative_loadpath2_idotdotlib
    with_fixture 'relloadpath2' do
      cd 'src' do
        assert system('ruby', '-I', '../lib', ocra, 'relloadpath2.rb', *DefaultArgs)
        assert File.exist?('relloadpath2.exe')
        assert system('relloadpath2.exe')
      end
    end
  end

  # Test that scripts which modify $LOAD_PATH with a relative path
  # (./lib) work correctly.
  def test_relloadpath3
    with_fixture 'relloadpath3' do
      assert system('ruby', ocra, 'relloadpath3.rb', *DefaultArgs)
      assert File.exist?('relloadpath3.exe')
      assert system('relloadpath3.exe')
    end
  end

  # Test that scripts which modify $LOAD_PATH with a relative path
  # (../lib) work correctly.
  def test_relloadpath4
    with_fixture 'relloadpath4' do
      cd 'src' do
        assert system('ruby', ocra, 'relloadpath4.rb', *DefaultArgs)
        assert File.exist?('relloadpath4.exe')
        assert system('relloadpath4.exe')
      end
    end
  end

  def test_version
    assert_match(/^Ocra \d+(\.\d)+$/, `ruby #{ocra} --version`)
  end

  def test_icon
    with_fixture 'helloworld' do
      icofile = File.join(OcraRoot, 'src', 'vit-ruby.ico')
      assert system("ruby", ocra, '--icon', icofile, "helloworld.rb", *DefaultArgs)
      assert File.exist?("helloworld.exe")
      assert system("helloworld.exe")
    end
  end

  def test_resource
    with_fixture 'resource' do
      assert system("ruby", ocra, "resource.rb", "resource.txt", "res/resource.txt", *DefaultArgs)
      assert File.exist?("resource.exe")
      assert system("resource.exe")
    end
  end
end

