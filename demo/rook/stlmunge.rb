#!/usr/bin/ruby

OUT = ARGV[0]

STDIN.read(80)
tri_count, = STDIN.read(4).unpack('V')

STDERR.puts "#{tri_count} triangles"

unique_points = {}

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

trivial_edges = 0
duplicate_edges = 0

0.upto(tri_count - 1) { |i|
  parts = STDIN.read(3*4 + 3*3*4 + 2).unpack("eee" + ("eee" + "eee" + "eee") + "v")

  nx, ny, nz, x0, y0, z0, x1, y1, z1, x2, y2, z2, atts = parts

  points = [
    Point3.new(x0, y0, z0 - 20),
    Point3.new(x1, y1, z1 - 20),
    Point3.new(x2, y2, z2 - 20)
  ]

  point_indices = points.collect { |p|
    i = unique_points[p]
    if i == nil
      i = unique_points.size
      unique_points[p] = i
    end
    i
  }

  edges = [
    Edge.new(point_indices[0], point_indices[1]),
    Edge.new(point_indices[1], point_indices[2]),
    Edge.new(point_indices[2], point_indices[0])
  ]

  edges.each { |e|
    if e.trivial?
      trivial_edges += 1
      next
    end

    if unique_edges[e]
      duplicate_edges += 1
      next
    end

    unique_edges[e] = 1
    $ends[e.a] ||= []
    $ends[e.a] << e
    $ends[e.b] ||= []
    $ends[e.b] << e
  }
}

STDERR.puts "#{unique_points.size} unique points."
STDERR.puts "#{unique_edges.size} unique edges."
STDERR.puts "#{2*unique_edges.size} endpoints."
STDERR.puts "#{unique_edges.size.to_f / (3*tri_count)} unique edges per input edge"
STDERR.puts "#{trivial_edges} edges rejected as trivial."
STDERR.puts "#{duplicate_edges} edges rejected as duplicate."

STDERR.puts "Greedily constructing strips...."

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

  # STDERR.puts "Built segment of #{segment.length} pieces"
  segments << segment
end

STDERR.puts "#{segments.length} segments total."
n =segments.inject(0) { |x, s| x + s.length }
STDERR.puts "#{n} points total."

STDERR.puts <<END
Indexed edge rep requires:
 - #{unique_points.size} matrix multiplies.
 - #{unique_edges.size} line draw calls.
 - #{unique_points.size * 12} bytes read from Flash.
 - #{unique_points.size * 12 * 2 + unique_edges.size * 4} bytes read/write from RAM.

Segment rep requires:
 - #{n} matrix multiplies.
 - #{n - segments.length} line draw calls.
 - #{n * 12} bytes read from Flash.
 - No significant non-display RAM traffic.
END

STDERR.puts "Output going into #{OUT}"

File.open(OUT + '/model.h', 'w') { |f|
  f.puts '#ifndef DEMO_ROOK_MODEL_H'
  f.puts '#define DEMO_ROOK_MODEL_H'
  f.puts
  f.puts '#include "etl/types.h"'
  f.puts '#include "demo/rook/geometry.h"'
  f.puts
  f.puts 'namespace demo {'
  f.puts 'namespace rook {'
  f.puts
  f.puts "static constexpr unsigned vertex_count = #{unique_points.size};"
  f.puts "static constexpr unsigned edge_count = #{unique_edges.size};"
  f.puts
  f.puts 'extern Vec3f const vertices[vertex_count];'
  f.puts 'extern etl::UInt16 const edges[edge_count][2];'
  f.puts
  f.puts '}  // namespace rook'
  f.puts '}  // namespace demo'
  f.puts
  f.puts '#endif  // DEMO_ROOK_MODEL_H'
}

File.open(OUT + '/model.cc', 'w') { |f|
  f.puts '#include "demo/rook/model.h"'
  f.puts
  f.puts 'namespace demo {'
  f.puts 'namespace rook {'
  f.puts
  f.puts "Vec3f const vertices[vertex_count] = {"
  unique_points.sort { |a, b| a[1] <=> b[1] }.each { |p, i|
    f.puts "  { #{p.x}, #{p.y}, #{p.z} },"
  }
  f.puts "};"

  f.puts "etl::UInt16 const edges[][2] = {"
  unique_edges.keys.sort { |a, b|
    if a.a == b.a then a.b <=> b.b else a.a <=> b.a end
  }.each { |e|
    f.puts "  { #{e.a}, #{e.b} },"
  }
  f.puts "};"

  f.puts
  f.puts '}  // namespace rook'
  f.puts '}  // namespace demo'
}
