#
# Queue with Timeout
#
#   Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gmail.com>
#

class TimedQueue
  class Timeout < StandardError; end

  def initialize(tmo)
    @que  = []
    @tmo  = tmo
    @mon  = Monitor.new
    @cond = @mon.new_cond
  end

  def enq(obj)
    @mon.synchronize {
      @que << obj
      @cond.signal
    }
  end

  alias :push :enq
  alias :<< :enq

  def deq
    @mon.synchronize {
      @cond.wait(@tmo)
      raise(Timeout) if @que.empty?

      return @que.shift
    }
  end

  alias :shift :deq
end

if $0 == __FILE__
  require "test/unit"
  require "timeout"

  class TestTimedQueue < Test::Unit::TestCase
    test "normal #1" do
      q = TimedQueue.new(1)

      q << :a
      q << :b
      q << :c

      assert_equal(:a, q.deq)
      assert_equal(:b, q.deq)
      assert_equal(:c, q.deq)
    end

    test "timeout" do
      q = TimedQueue.new(1)

      assert_raise_kind_of(TimedQueue::Timeout) {
        Timeout.timeout(5) {q.deq}
      }
    end
  end
end
