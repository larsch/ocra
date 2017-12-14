require File.join File.dirname(__FILE__), "fake_code_signer/pe_wrapper"

# This class digitally signs an executable.
# In reality, it doesn't create a "valid" digital signature - but it sets up the executable
# headers correctly with the appropriate size and offsets. It also appends the "signature" to
# the end of the executable, complete with 'padding' between the end of the file and sig.
# Although this signature is not 'valid' in the sense that it meets the authenticode standard
# it still meets the expectations of Ocra which only needs the appended "signature" to match
# the size and location as specificed in the header.
#
# The purpose of this class is to create a "signed" executable so that we can test Ocra's
# ability to build executables that are resilient to authenticode code signing, i.e that
# Ocra executables still work after they've been digitally signed.
#
# For more information see:
# The PE Format deep dive: https://msdn.microsoft.com/en-us/library/ms809762.aspx
# Reverse engineering the authenticode format: https://www.cs.auckland.ac.nz/~pgut001/pubs/authenticode.txt
class FakeCodeSigner
  FAKE_SIG = "fake signature"

  def initialize(input_file:, output_file:, padding: 4)
    @output_file = output_file
    @padding = padding
    @image = File.binread(input_file)
  end

  def sign
    if pe_wrapper.security_size != 0
      raise "Binary already signed, nothing to do!"
    end

    # Below we access an instance of the IMAGE_DATA_DIRECTORY struct.
    # This instance is called IMAGE_DIRECTORY_ENTRY_SECURITY and it contains information about the digital signature
    # see: https://msdn.microsoft.com/en-us/library/windows/desktop/ms680305(v=vs.85).aspx

    # write the offset (address) of the digital signature to the security header (VirtualAddress field)
    pe_wrapper.security_address = @image.size + @padding

    # write the size of the digital signature to the security header (Size field)
    pe_wrapper.security_size = FAKE_SIG.size

    # append the "digital signature" to the end of the executable, complete with padding
    pe_wrapper.append_data(padding_string + FAKE_SIG)

    # Write out the "signed" image
    File.binwrite(@output_file, pe_wrapper.to_s)
  end

  private

  def padding_string
    "\x0" * @padding
  end

  def pe_wrapper
    @pe_wrapper ||= PEWrapper.new(@image)
  end
end
