#
# symbolize_keys
#
#   Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gmai.com>
#

class Hash
  def symbolize_keys
    ret = {}

    self.each { |k, v|
      k = k.to_sym if k.kind_of?(String)
      v = v.symbolize_keys if v.kind_of?(Hash) or v.kind_of?(Array)
      ret[k] = v
    }

    return ret
  end

  def symbolize_keys!
    self.each_value { |v|
      v.symbolize_keys! if v.kind_of?(Hash) or v.kind_of?(Array)
    }

    self.keys.each { |k|
      self[k.to_sym] = self.delete(k) if k.kind_of?(String)
    }

    return self
  end
end

class Array
  def symbolize_keys
    ret = []

    self.each { |v|
      v = v.symbolize_keys if v.kind_of?(Hash) or v.kind_of?(Array)
      ret << v
    }

    return ret
  end

  def symbolize_keys!
    self.each { |v|
      v.symbolize_keys! if v.kind_of?(Hash) or v.kind_of?(Array)
    }

    return self
  end
end

if $0 == __FILE__
  #require 'pry'
  #binding.pry

  h = {:a => 0, "b" => 1, :c => [{:d => "s", "e" => "x"}, 0]}
  p h
  p h.symbolize_keys
  p h
  p h.symbolize_keys!
  p h
end
