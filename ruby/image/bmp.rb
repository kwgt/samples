#! /usr/bin/env ruby
# coding: utf-8

#
# bmp file reader
#
#   Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gmail.com>
#

require 'pathname'
require 'bmp/bmp'

class BMP
  class << self
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

    def read_header(src)
      io  = open(src)
      ret = {}

      tmp = io.read(14).unpack("a2L<x2x2L<")
      ret[:magic]     = tmp[0]
      ret[:file_size] = tmp[1]
      ret[:offset]    = tmp[2]

      raise("invalid magic number") if ret[:magic] != "BM"
      raise("illeagal file size") if ret[:file_size] != io.size

      tmp = io.read(4).unpack("L<")

      case tmp[0]
      when 12
        read_os2_header(io, ret)
        read_os2_palette(io, ret)

      when 40
        read_win_header(io, ret)
        read_win_palette(io, ret)

      else
        raise("invalid header")
      end

      return ret

    ensure
      io&.close
    end

    def read_os2_header(io, h)
      tmp = io.read(8).unpack("s<s<S<S<")

      h[:type]          = :OS2
      h[:img_width]     = tmp[0]
      h[:img_height]    = tmp[1]
      h[:planes]        = tmp[2]
      h[:bit_count]     = tmp[3]

      tmp = h[:offset] - (14 + 12)
      raise("broken header") if tmp < 0

      h[:num_palette]   = tmp / 3

      case h[:bit_count]
      when 1, 4, 8
        if h[:num_palette] > (1 << h[:bit_count])
          raise("invalid header")
        end

      when 24, 32
        if h[:num_palette] != 0
          raise("invalid header")
        end

      else
        raise("not supoorted bit count #{h[:bit_count]}")
      end
    end
    private :read_os2_header

    def read_os2_palette(io, h)
      if h[:num_palette] > 0
        h[:palette] = []
        h[:num_palette].times {
          h[:palette] << io.read(3).unpack("CCC")
        }
      end
    end
    private :read_os2_palette

    def read_win_header(io, h)
      tmp = io.read(36).unpack("l<l<S<S<L<L<l<l<L<L<")

      h[:type]          = :WINDOWS
      h[:img_width]     = tmp[0]
      h[:img_height]    = tmp[1]
      h[:planes]        = tmp[2]
      h[:bit_count]     = tmp[3]
      h[:compression]   = tmp[4]
      h[:img_size]      = tmp[5]
      h[:x_resolution]  = tmp[6]
      h[:y_resolution]  = tmp[7]
      h[:num_palette]   = tmp[8]
      h[:cir_important] = tmp[9]

      tmp = h[:offset] - (14 + 40)
      raise("broken header") if tmp < 0 or h[:num_palette] != (tmp / 4)

      case h[:bit_count]
      when 1, 4, 8
        if h[:num_palette] > (1 << h[:bit_count])
          raise("invalid header")
        end

      when 24, 32
        if tmp != 0
          raise("invalid header")
        end

      else
        raise("not supoorted bit count #{h[:bit_count]}")
      end
    end
    private :read_win_header

    def read_win_palette(io, h)
      if h[:num_palette] > 0
        h[:palette] = []
        h[:num_palette].times {
          h[:palette] << io.read(4).unpack("CCCx")
        }
      end
    end
    private :read_win_palette
  end

  def initialize(path, **opt)
    @header = BMP.read_header(path)
    @path   = path

    eval_grayscale_opt(opt[:grayscale])
  end

  attr_reader :header

  def eval_grayscale_opt(val)
    #
    # グレースケールかを行う場合の計数を設定
    # ※整数演算化のため1024倍した値で保持
    #
    case val
    when :BT601
      @grayscale = {
        :r => 306, # 0.2990
        :g => 601, # 0.5870
        :b => 117  # 0.1140
      }

    when :BT709
      @grayscale = {
        :r => 218, # 0.2126
        :g => 732, # 0.7152
        :b =>  74  # 0.0722
      }

    when :JP_ANALOG
      @grayscale = {
        :r => 307, # 0.3000
        :g => 604, # 0.5900
        :b => 113  # 0.1100
      }

    when nil, false
      # nothing

    else
      raise("invalid grascale method")
    end
  end
  private :eval_grayscale_opt

  def width
    return @header[:img_width]
  end

  def height
    return @header[:img_height]
  end

  def data
    if @header[:type] == :WINDOWS and @header[:compression] != 0
      raise("compressioned bmp is not support")
    end

    io = File.open(@path, "rb")
    io.seek(@header[:offset])

    if not @grayscale
      case @header[:bit_count]
      when 1
        ret = extract_image1(io)

      when 4
        ret = extract_image4(io)

      when 8
        ret = extract_image8(io)

      when 24
        ret = extract_image24(io)

      when 32
        ret = extract_image32(io)

      else
        raise("unsupported bit depth")
      end

    else
      case @header[:bit_count]
      when 1
        ret = extract_grayscale1(io)

      when 4
        ret = extract_grayscale4(io)

      when 8
        ret = extract_grayscale8(io)

      when 24
        ret = extract_grayscale24(io)

      when 32
        ret = extract_grayscale32(io)

      else
        raise("unsupported bit depth")
      end
    end

    return ret

  ensure
    io&.close
  end
end

if __FILE__ == $0
  require 'png'

  bmp = BMP.new(ARGV[0], :grayscale => :BT601)
  p bmp.header

  enc = PNG::Encoder.new(bmp.width, bmp.height, :pixel_format => :GRAYSCALE)
  IO.binwrite("gs.png", enc << bmp.data)

  bmp = BMP.new(ARGV[0], :grayscale => false)

  enc = PNG::Encoder.new(bmp.width, bmp.height, :pixel_format => :RGB)
  IO.binwrite("color.png", enc << bmp.data)
end
