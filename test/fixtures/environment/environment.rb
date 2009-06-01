if $0 == __FILE__
  File.open("environment", "wb") do |f|
    f.write(Marshal.dump(ENV.to_hash))
  end
end
