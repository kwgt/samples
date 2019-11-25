

MARKER = [0x00, 0x00, 0x00, 0x01]

fc     = 0
marker = []
slice  = nil

until ARGF.eof
  marker << ARGF.getbyte

  if marker == MARKER
    print "\r#{fc}"

    if not slice.nil?
      IO.binwrite("%04d.264" % fc, slice)
      fc += 1
    end

    marker.clear
    slice = "\x00\x00\x00\x01".b
  end

  if marker.size == 4
    slice << marker.shift
  end
end

print "\n"
