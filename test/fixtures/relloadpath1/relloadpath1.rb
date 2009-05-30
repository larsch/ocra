require 'somemod'
require 'anothermod'
require 'athirdmod'
if not defined?(Ocra)
  normalized_paths = ENV['RUBYLIB'].split(File::PATH_SEPARATOR).map { |x| x.downcase.tr('/','\\') }
  duplicates = normalized_paths.size - normalized_paths.sort.uniq.size
  exit -1 if duplicates > 0
end
