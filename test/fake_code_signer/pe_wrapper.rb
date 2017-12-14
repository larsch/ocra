class FakeCodeSigner
  # This class exists to navigate and manipulate the data of the Windows
  # executable format, PE, see: https://en.wikipedia.org/wiki/Portable_Executable
  # also https://msdn.microsoft.com/en-us/library/ms809762.aspx
  class PEWrapper
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
    DATA_DIRECTORY_ENTRY_SIZE = 8

    def initialize(image)
      @image = image
    end

    # set digital signature address in security header
    def security_address=(offset)
      @image[security_address_offset, DWORD_SIZE] = raw_bytes(offset)
    end

    # set digital signature size in security header
    def security_size=(size)
      @image[security_size_offset, DWORD_SIZE] = raw_bytes(size)
    end

    # location of the digital signature
    def security_address
      deref(security_address_offset)
    end

    # size of the digital signature
    def security_size
      deref(security_size_offset)
    end

    # append data to end of executable
    def append_data(byte_string)
      @image << byte_string
    end

    private

    # convert an integer to a raw byte string
    def raw_bytes(int)
      [int].pack("L")
    end

    # security is the 4th element in the data directory array
    # see http://bytepointer.com/resources/pietrek_in_depth_look_into_pe_format_pt1_figures.htm
    def security_offset
      image_data_directory_offset + DATA_DIRECTORY_ENTRY_SIZE * 4
    end

    alias security_address_offset security_offset

    def security_size_offset
      security_offset + DWORD_SIZE
    end

    # dereferences a pointer
    # the only pointer type we support is an unsigned long (DWORD)
    def deref(ptr)
      @image[ptr, DWORD_SIZE].unpack("L").first
    end

    # offset of e_lfanew (which stores the offset of the actual PE header)
    # see: https://msdn.microsoft.com/en-us/library/ms809762.aspx for more info
    def e_lfanew_offset
      DOS_HEADER_SIZE - DWORD_SIZE
    end

    # We dereference e_lfanew_offset to get the actual pe_header_offset
    def pe_header_offset
      deref(e_lfanew_offset)
    end

    # offset of the IMAGE_OPTIONAL_HEADER struct
    # see: https://msdn.microsoft.com/en-us/library/windows/desktop/ms680313(v=vs.85).aspx
    def image_optional_header_offset
      pe_header_offset + PE_SIGNATURE_SIZE + IMAGE_FILE_HEADER_SIZE
    end

    # DataDirectory is an array
    # and is the last element of the IMAGE_OPTIONAL_HEADER struct
    # see: https://msdn.microsoft.com/en-us/library/windows/desktop/ms680339(v=vs.85).aspx
    def data_directories_array_size
      DATA_DIRECTORY_ENTRY_SIZE * NUMBER_OF_DATA_DIRECTORY_ENTRIES
    end

    # Since the DataDirectory is the last item in the optional header
    # struct we find the DD offset by navigating to the end of the struct
    # and then going back `data_directories_array_size` bytes
    def image_data_directory_offset
      image_optional_header_offset + IMAGE_OPTIONAL_HEADER_SIZE -
        data_directories_array_size
    end
  end
end
