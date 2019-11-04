#! /usr/bin/env ruby
# coding: utf-8

#
# scan keys
#
#   Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gmail.com>
#

class Hash
  def scan_keys
    ret = []

    self.each_pair { |k, v|
      if v.kind_of?(Hash) or v.kind_of?(Array)
        v.scan_keys.each { |a|
          ret << a.prepend(k)
        }
      else
        ret << [k]
      end
    }

    return ret
  end
end

class Array
  def scan_keys
    ret = []

    self.each.with_index { |v, i|
      if v.kind_of?(Hash) or v.kind_of?(Array)
        v.scan_keys.each { |a|
          ret << a.prepend(i)
        }

      else
        ret << [i]
      end
    }

    return ret
  end
end

if $0 == __FILE__
  #require 'pry'
  #binding.pry

  h = {:a => 0, "b" => 1, :c => [{:d => "s", :e => "x"}, 0]}
  pp h
  pp h.scan_keys
end
