#! /usr/bin/env ruby
# coding: utf-8

#
# pgm file reader
# 

require 'pathname'

class PGM
  class << self
    DIGIT = "0123456789".b
    SPACE = " \t\r\n\f\v".b

    def open(src)
      case src
      when String
        ret = File.open(src, "rb")

      when Pathname
        ret = src.open("rb")

      when IO
        ret = src.clone
        ret.binmode

      else
        raise("can not read")
      end

      return ret
    end
    private :open

    def digit?(c)
      return DIGIT.include?(c)
    end
    private :digit?

    def space?(c)
      return SPACE.include?(c)
    end
    private :space?

    def read_header(src)
      io  = open(src)
      st  = :MAGIC
      buf = ""
      cm  = false
      wd  = nil
      ht  = nil
      mg  = nil

      loop {
        ch = io.getc

        if cm
          cm = false if ch == "\n"
          next

        elsif ch == "#"
          cm = true
          next
        end

        case st
        when :MAGIC
          buf << ch
          if buf.size == 2
            st = :DM1
            raise("invalid magic number") if buf != "P5"
          end

        when :DM1
          if digit?(ch)
            st  = :WIDTH
            buf = ch

          elsif !space?(ch)
            raise("illeagal sequence")
          end

        when :WIDTH
          if digit?(ch)
            buf << ch

          elsif space?(ch)
            st  = :DM2
            wd  = buf.to_i

          else
            raise("illeagal sequence")
          end

        when :DM2
          if digit?(ch)
            st  = :HEIGHT
            buf = ch

          elsif !space?(ch)
            raise("illeagal sequence")
          end

        when :HEIGHT
          if digit?(ch)
            buf << ch

          elsif space?(ch)
            st  = :DM3
            ht  = buf.to_i

          else
            raise("illeagal sequence")
          end

        when :DM3
          if digit?(ch)
            st  = :MAX_GRAY
            buf = ch

          elsif !space?(ch)
            raise("illeagal sequence")
          end

        when :MAX_GRAY
          if digit?(ch)
            buf << ch

          elsif space?(ch)
            mg = buf.to_i
            raise("invalid max gray") if mg != 255
            break

          else
            raise("illeagal sequence")
          end
        end
      }

      ret = {
        :type     => :grayscale,
        :width    => wd,
        :height   => ht,
        :max_gray => mg,
        :offset   => io.tell
      }

      return ret

    ensure
      io&.close
    end

    def read_data(src)
      return PGM.new(src).data
    end
  end

  def initialize(path)
    @header = PGM.read_header(path)
    @path   = Pathname.new(path)
  end

  def data
    off  = @header[:offset]
    size = @header[:width] * @header[:height]

    raise("invalid file") if @path.size != off + size

    return IO.binread(@path, size, off)
  end
end

if __FILE__ == $0
  p PGM.read_header(ARGV[0])
end
