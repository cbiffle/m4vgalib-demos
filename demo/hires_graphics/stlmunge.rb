#!/usr/bin/ruby

STDIN.read(80)
tri_count, = STDIN.read(4).unpack('V')

STDERR.puts "#{tri_count} triangles"

unique_edges = {}
$ends = {}

CH = 10000

class Point3
  attr_reader :x
  attr_reader :y
  attr_reader :z

  def initialize(x, y, z)
    @x = x
    @y = y
    @z = z
  end

  def hash
    ((x * CH).round.hash) + ((y * CH).round.hash) + ((z * CH).round.hash)
  end

  def eql?(other)
    ((x * CH).round) == ((other.x * CH).round) &&
      ((y * CH).round) == ((other.y * CH).round) &&
      ((z * CH).round) == ((other.z * CH).round)
  end

  def <(other)
    if x != other.x
      return x < other.x
    elsif y != other.y
      return y < other.y
    elsif z != other.z
      return z < other.z
    else
      return false
    end
  end

  def to_s
    "<" + [ x, y, z ].each { |f| f.to_s }.join(", ") + ">"
  end
end

class Edge
  attr_accessor :a
  attr_accessor :b

  def initialize(a, b)
    if a < b
      @a = a
      @b = b
    else
      @b = a
      @a = b
    end
  end

  def hash
    @a.hash ^ @b.hash
  end

  def eql?(other)
    @a.eql?(other.a) && @b.eql?(other.b)
  end

  def to_s
    "Edge(" + @a.to_s + ", " + @b.to_s + ")"
  end

  def trivial?
    @a.eql?(@b)
  end
end

0.upto(tri_count - 1) { |i|
  parts = STDIN.read(3*4 + 3*3*4 + 2).unpack("eee" + ("eee" + "eee" + "eee") + "v")

  nx, ny, nz, x0, y0, z0, x1, y1, z1, x2, y2, z2, atts = parts

  points = [
    Point3.new(x0, y0, z0),
    Point3.new(x1, y1, z1),
    Point3.new(x2, y2, z2)
  ]

  edges = [
    Edge.new(points[0], points[1]),
    Edge.new(points[1], points[2]),
    Edge.new(points[2], points[0])
  ]

  edges.each { |e|
    next if e.trivial?
    unique_edges[e] = 1
    $ends[e.a] ||= []
    $ends[e.a] << e
    $ends[e.b] ||= []
    $ends[e.b] << e
    raise 'noes' if $ends[e.a].equal? $ends[e.b]
  }
}

STDERR.puts "#{unique_edges.size} unique edges."
STDERR.puts "#{2*unique_edges.size} unique points."
STDERR.puts "#{unique_edges.size.to_f / (3*tri_count)} unique edges per input edge"

#unique_edges.each_key { |e|
#  puts "{"
#  puts "  { #{e.a.x}f, #{e.a.y}f, #{e.a.z}f },"
#  puts "  { #{e.b.x}f, #{e.b.y}f, #{e.b.z}f },"
#  puts "},"
#}

STDERR.puts "Greedily constructing strips."

segments = []

while not $ends.empty?
  p = $ends.keys.first
  edges = $ends[p]

  e = edges.first

  if $ends[e.b]
    $ends[e.b].delete(e)
    $ends.delete(e.b) if $ends[e.b].empty?
  else
    STDERR.puts "Edge #{e} detached at #{e.b}"
  end

  if $ends[e.a]
    $ends[e.a].delete(e)
    $ends.delete(e.a) if $ends[e.a].empty?
  else
    STDERR.puts "Edge detached at #{e.a}"
  end

  segment = [e.a, e.b]

  while not $ends.empty?
    if $ends[segment.first]
      e2 = $ends[segment.first].shift
      $ends.delete(segment.first) if $ends[segment.first].empty?
      if e2.a.eql? segment.first
        $ends[e2.b].delete(e2)
        $ends.delete(e2.b) if $ends[e2.b].empty?
        segment.insert(0, e2.b)
      else
        $ends[e2.a].delete(e2)
        $ends.delete(e2.a) if $ends[e2.a].empty?
        segment.insert(0, e2.a)
      end
    elsif $ends[segment.last]
      e2 = $ends[segment.last].shift
      $ends.delete(segment.last) if $ends[segment.last].empty?
      if e2.a.eql? segment.last
        $ends[e2.b].delete(e2)
        $ends.delete(e2.b) if $ends[e2.b].empty?
        segment << e2.b
      else
        $ends[e2.a].delete(e2)
        $ends.delete(e2.a) if $ends[e2.a].empty?
        segment << e2.a
      end
    else
      break
    end
  end

  STDERR.puts "Built segment of #{segment.length} pieces"
  segments << segment
end

STDERR.puts "#{segments.length} segments total."
n =segments.inject(0) { |x, s| x + s.length }
STDERR.puts "#{n} points total."

segments.each { |seg|
  STDOUT.write [ seg.length ].pack("V")
  seg.each { |p|
    STDOUT.write [ p.x, p.y, p.z ].pack("eee")
  }
}
STDOUT.write [ 0 ].pack("V")

