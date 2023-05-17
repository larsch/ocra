# frozen_string_literal: true

require_relative "lib/ocran/version"

Gem::Specification.new do |spec|
  spec.name = "ocran"
  spec.version = Ocran::VERSION
  spec.authors = ["Andi Idogawa", "Lars Christensen"]
  spec.email = ["andi@idogawa.com"]

  spec.licenses = ["MIT"]

  spec.summary = "OCRAN (One-Click Ruby Application Next) builds Windows executables from Ruby source code."
  spec.description = "OCRAN (One-Click Ruby Application Next) builds Windows executables from Ruby source code. 
  The executable is a self-extracting, self-running executable that contains the Ruby interpreter, your source code and any additionally needed ruby libraries or DLL.
  
  This is a fork of OCRA that is compatible with ruby version after 2.6. 
  Migration guide: make sure to write ocran instead of ocra in your code. For instance: OCRAN_EXECUTABLE

  usage: 
    ocra helloworld.rb
    helloworld.exe

  See readme at https://github.com/largo/ocran
  Report problems in the github issues. Contributions welcome.
  This gem contains executables. We plan to build them on github actions for security.
  "
  spec.homepage = "https://github.com/largo/ocran"
  spec.required_ruby_version = ">= 2.0.0"

  spec.metadata["homepage_uri"] = spec.homepage
  spec.metadata["source_code_uri"] = "https://github.com/largo/ocran"
  spec.metadata["changelog_uri"] = "https://github.com/largo/ocran/History.txt"

  # Specify which files should be added to the gem when it is released.
  # The `git ls-files -z` loads the files in the RubyGem that have been added into git.
    #   spec.files = Dir.chdir(__dir__) do
    #     `git ls-files -z`.split("\x0").reject do |f|
    #       (f == __FILE__) || f.match(%r{\A(?:(?:bin|test|spec|features)/|\.(?:git|circleci)|appveyor)})
    #     end
    #   end
  spec.files = Dir.glob("bin/ocran") + Dir.glob("lib/**/**")  + Dir.glob("share/ocran/**") 
  spec.bindir = "bin"
  spec.executables = %w[ocran]
  spec.require_paths = ["lib"]

  # Uncomment to register a new dependency of your gem
  # spec.add_dependency "example-gem", "~> 1.0"

  # For more information and examples about making a new gem, check out our
  # guide at: https://bundler.io/guides/creating_gem.html
end
