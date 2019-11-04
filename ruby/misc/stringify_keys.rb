#
# stringify_keys
#
#   Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gmail.com>
#

class Hash
  def stringify_keys
    ret = {}

    self.each { |k, v|
      k = k.to_s if k.kind_of?(Symbol)
      v = v.stringify_keys if v.kind_of?(Hash) or v.kind_of?(Array)
      ret[k] = v
    }

    return ret
  end

  def stringify_keys!
    self.each_value { |v|
      v.stringify_keys! if v.kind_of?(Hash) or v.kind_of?(Array)
    }

    self.keys.each { |k|
      self[k.to_s] = self.delete(k) if k.kind_of?(Symbol)
    }

    return self
  end
end

class Array
  def stringify_keys
    ret = []

    self.each { |v|
      v = v.stringify_keys if v.kind_of?(Hash) or v.kind_of?(Array)
      ret << v
    }

    return ret
  end

  def stringify_keys!
    self.each { |v|
      v.stringify_keys! if v.kind_of?(Hash) or v.kind_of?(Array)
    }

    return self
  end
end

if $0 == __FILE__
  #require 'pry'
  #binding.pry

  h = {:a => 0, "b" => 1, :c => [{:d => "s", :e => "x"}, 0]}
  p h
  p h.stringify_keys
  p h.stringify_keys!
  p h
end
