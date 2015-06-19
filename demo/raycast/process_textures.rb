#!/usr/bin/env ruby

IN = ARGV[0]
OUTDIR = ARGV[1]
NAME = ARGV[2]

TEXWIDTH = 64
TEXHEIGHT = 64

input = nil

STDERR.puts "Loading #{IN}..."

File.open(IN) { |f|
  input = f.readlines.flat_map { |line|
    if line =~ /^#.*/
      []
    else
      line.split(/\s+/)
    end
  }
}

magic = input.shift
if magic != "P3"
  raise "Bad PNM format (expected P3)"
end

width = input.shift.to_i
height = input.shift.to_i

if height != TEXHEIGHT or (width % TEXWIDTH) != 0
  raise "Unexpected size #{width}x#{height}"
end

cmax = input.shift.to_i
if ((cmax + 1) & cmax) != 0
  raise "Non-power-of-two color range #{cmax}"
end

cshift = (Math.log2(cmax + 1) - 2).to_i

samples = []
(width * height).times {
  r = input.shift.to_i >> cshift
  g = input.shift.to_i >> cshift
  b = input.shift.to_i >> cshift

  samples << (r | (g << 2) | (b << 4))
}

if not input.empty?
  raise "garbage at end of input"
end

texture_count = width / TEXWIDTH
STDERR.puts "Texture count: #{texture_count}"

pal_top = Array.new(256, 0)
pal_bot = Array.new(256, 0)
colors_used = 1  # reserve color 0 for ceiling/floor
color_indices = {}
textures = []

(0...texture_count).each { |texnum|
  STDERR.puts "Processing texture #{texnum} (#{colors_used} allocated)"
  tex = []
  (0...(TEXWIDTH)).each { |x|
    (0...(TEXHEIGHT/2)).each { |y|
      y_ = TEXHEIGHT - 1 - y
      top_sample = samples[y * width + (x + texnum * TEXWIDTH)]
      bot_sample = samples[y_ * width + (x + texnum * TEXWIDTH)]
      color_pair = (top_sample << 8) | bot_sample

      pi = color_indices[color_pair]

      if pi == nil
        if colors_used == 256
          raise "Out of colors :-("
        end

        pi = colors_used
        colors_used += 1
        pal_top[pi] = top_sample
        pal_bot[pi] = bot_sample
        color_indices[color_pair] = pi
      end
      tex << pi
    }
  }
  textures << tex
}

STDERR.puts "Success: #{colors_used} colors used."

STDERR.puts "Output going into #{OUTDIR}."

File.open("#{OUTDIR}/#{NAME}.h", 'w') { |f|
  f.puts <<-END.gsub(/^ {4}/, '')
    #ifndef DEMO_RAYCAST_#{NAME.upcase}_H
    #define DEMO_RAYCAST_#{NAME.upcase}_H

    #include <cstdint>

    #include "demo/raycast/texture.h"
   
    namespace demo {
    namespace raycast {

    static constexpr unsigned #{NAME}_color_count = #{colors_used};
    extern std::uint8_t const #{NAME}_palette_top[#{NAME}_color_count];
    extern std::uint8_t const #{NAME}_palette_bot[#{NAME}_color_count];

    static constexpr unsigned #{NAME}_texture_count = #{texture_count};
    extern Texture const #{NAME}_tex[#{NAME}_texture_count];

    }  // namespace raycast
    }  // namespace demo
    
    #endif  // DEMO_RAYCAST_#{NAME.upcase}_H
  END
}

File.open("#{OUTDIR}/#{NAME}.cc", 'w') { |f|
  f.puts <<-END.gsub(/^ {4}/, '')
    #include "demo/raycast/#{NAME}.h"

    namespace demo {
    namespace raycast {
  END

  f.puts "std::uint8_t const #{NAME}_palette_top[#{colors_used}] {"
  (0...colors_used).each { |pi| f.puts "  0x#{pal_top[pi].to_s(16)}," }
  f.puts "};"

  f.puts "std::uint8_t const #{NAME}_palette_bot[#{colors_used}] {"
  (0...colors_used).each { |pi| f.puts "  0x#{pal_bot[pi].to_s(16)}," }
  f.puts "};"

  f.puts "Texture const #{NAME}_tex[#{texture_count}] {"
  texpix = TEXWIDTH * TEXHEIGHT / 2
  (0...texture_count).each { |ti|
    f.puts "  // texture #{ti}"
    f.print "  {\n    "
    (0...(texpix)).each { |si|
      f.print "0x#{textures[ti][si].to_s(16)}, "
      f.print "\n    " if (si % 8) == 7
    }
    f.puts "},"
  }
  f.puts "};"

  f.puts <<-END.gsub(/^ {4}/, '')
    }  // namespace raycast
    }  // namespace demo
  END
}
