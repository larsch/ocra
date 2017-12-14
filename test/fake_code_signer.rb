class FakeCodeSigner
  FAKE_SIG = "fake signature"

  def initialize(input:, output:, padding: 4)
    @input = input
    @output = output
    @padding = padding
    @image = File.binread(input)
  end

  def sign
    if pe_header.security_size !=0
      puts "Binary already signed, nothing to do!"
      return
    elsif @input == @output
      puts "input and output files must be different!"
      return
    end

    @image[pe_header.security_offset, 4] = raw_bytes(@image.size + @padding)

    @image[pe_header.security_offset + 4, 4] = raw_bytes(FAKE_SIG.size)
    @image << padding_string << FAKE_SIG

    File.binwrite(@output, @image)
  end

  private

  def padding_string
    "\x0" * @padding
  end

  def raw_bytes(int)
    [int].pack("L")
  end

  def pe_header
    @pe_header ||= PEHeader.new(@image)
  end
end

class PEHeader
  # size of a DWORD is 2 words (4 bytes)
  DWORD_SIZE = 4

  # struct IMAGE_DOS_HEADER
  # https://www.nirsoft.net/kernel_struct/vista/IMAGE_DOS_HEADER.html
  DOS_HEADER_SIZE = 64

  # first field from IMAGE_NT_HEADERS
  # https://msdn.microsoft.com/en-us/library/windows/desktop/ms680336(v=vs.85).aspx
  PE_SIGNATURE_SIZE = 4

  # struct IMAGE_FILE_HEADER
  # see: https://msdn.microsoft.com/en-us/library/windows/desktop/ms680313(v=vs.85).aspx
  IMAGE_FILE_HEADER_SIZE = 20

  # struct IMAGE_OPTIONAL_HEADER
  # https://msdn.microsoft.com/en-us/library/windows/desktop/ms680339(v=vs.85).aspx
  IMAGE_OPTIONAL_HEADER_SIZE = 224

  # http://bytepointer.com/resources/pietrek_in_depth_look_into_pe_format_pt1_figures.htm
  NUMBER_OF_DATA_DIRECTORY_ENTRIES = 16

  # struct IMAGE_DATA_DIRECTORY
  # https://msdn.microsoft.com/en-us/library/windows/desktop/ms680305(v=vs.85).aspx
  DATA_DIRECTORY_ENTRY_SIZE = 4

  def initialize(image)
    @image = image
  end

  # security is the 4th element in the data directory array
  def security_offset
    image_data_directory_offset + data_directory_entry_size * 4
  end

  # location of the digital signature
  def security_address
    deref(security_offset)
  end

  # size of the digital signature
  def security_size
    deref(security_offset + 4)
  end

  private

  # the only pointer type we support is an unsigned long (DWORD)
  def deref(ptr)
    @image[ptr, 4].unpack("L").first
  end

  def e_lfanew_offset
    DOS_HEADER_SIZE - DWORD_SIZE
  end

  def pe_header_offset
    deref(e_lfanew_offset)
  end

  def image_optional_header_offset
    pe_header_offset + PE_SIGNATURE_SIZE + IMAGE_FILE_HEADER_SIZE
  end

  # 2 DWORDs
  def data_directory_entry_size
    4 * 2
  end

  def data_directories_array_size
    data_directory_entry_size * NUMBER_OF_DATA_DIRECTORY_ENTRIES
  end

  def image_data_directory_offset
    image_optional_header_offset + IMAGE_OPTIONAL_HEADER_SIZE -
      data_directories_array_size
  end
end
