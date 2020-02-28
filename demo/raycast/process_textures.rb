#!/usr/bin/env ruby

IN = ARGV[0]
OUTHDR = ARGV[1]
OUTIMPL = ARGV[2]
NAME = ARGV[3]

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

def samp2pal(s)
  r = s[0] >> 6
  g = s[1] >> 6
  b = s[2] >> 6
  (r | (g << 2) | (b << 4))
end

CSHIFT = (8 - Math.log2(cmax + 1)).to_i

samples = []
(width * height).times {
  r = input.shift.to_i << CSHIFT
  g = input.shift.to_i << CSHIFT
  b = input.shift.to_i << CSHIFT

  samples << [r, g, b]
}

if not input.empty?
  raise "garbage at end of input"
end

texture_count = width / TEXWIDTH
STDERR.puts "Texture count: #{texture_count}"

$pal_top = Array.new(256, 0)
$pal_bot = Array.new(256, 0)
$dark = Array.new(256, 0)
$colors_used = 1  # reserve color 0 for ceiling/floor
$color_indices = {}
$new_dark_colors = 0
textures = []

def alloc(top_sample, bot_sample)
  color_pair = (samp2pal(top_sample) << 8) | samp2pal(bot_sample)

  pi = $color_indices[color_pair]

  if pi == nil
    if $colors_used == 256
      raise "Out of colors :-("
    end

    pi = $colors_used
    $colors_used += 1
    $pal_top[pi] = samp2pal(top_sample)
    $pal_bot[pi] = samp2pal(bot_sample)
    $color_indices[color_pair] = pi
  end

  pi
end

def alloc_and_darken(top_sample, bot_sample)
  pi = alloc(top_sample, bot_sample)

  colors_before = $colors_used
  top_dark = top_sample.collect { |x| x / 2}
  bot_dark = bot_sample.collect { |x| x / 2}
  di = alloc(top_dark, bot_dark)

  if di >= colors_before
    $new_dark_colors += 1
  end

  if $dark[pi] == 0
    $dark[pi] = di
  elsif $dark[pi] != di
    raise "Darkening bug?"
  end

  pi
end

(0...texture_count).each { |texnum|
  STDERR.puts "Processing texture #{texnum} (#{$colors_used} allocated)"
  tex = []
  (0...(TEXWIDTH)).each { |x|
    (0...(TEXHEIGHT/2)).each { |y|
      y_ = TEXHEIGHT - 1 - y
      top_sample = samples[y * width + (x + texnum * TEXWIDTH)]
      bot_sample = samples[y_ * width + (x + texnum * TEXWIDTH)]

      tex << alloc_and_darken(top_sample, bot_sample)
    }
  }
  textures << tex
}

STDERR.puts "Success: #{$colors_used} colors used."
STDERR.puts "#{$new_dark_colors} colors consumed by palette darkening."

STDERR.puts "Output going into #{OUTHDR} / #{OUTIMPL}."

File.open(OUTHDR, 'w') { |f|
  f.puts <<-END.gsub(/^ {4}/, '')
    #ifndef DEMO_RAYCAST_#{NAME.upcase}_H
    #define DEMO_RAYCAST_#{NAME.upcase}_H

    #include <cstdint>

    #include "demo/raycast/texture.h"
   
    namespace demo {
    namespace raycast {

    static constexpr unsigned #{NAME}_color_count = #{$colors_used};
    extern std::uint8_t const #{NAME}_palette_top[#{NAME}_color_count];
    extern std::uint8_t const #{NAME}_palette_bot[#{NAME}_color_count];

    static constexpr unsigned #{NAME}_texture_count = #{texture_count};
    extern Texture const #{NAME}_tex[#{NAME}_texture_count];

    extern std::uint8_t const #{NAME}_darken[#{NAME}_color_count];

    }  // namespace raycast
    }  // namespace demo
    
    #endif  // DEMO_RAYCAST_#{NAME.upcase}_H
  END
}

File.open(OUTIMPL, 'w') { |f|
  f.puts <<-END.gsub(/^ {4}/, '')
    #include "demo/raycast/#{NAME}.h"

    namespace demo {
    namespace raycast {
  END

  f.puts "std::uint8_t const #{NAME}_palette_top[#{$colors_used}] {"
  (0...$colors_used).each { |pi| f.puts "  0x#{$pal_top[pi].to_s(16)}," }
  f.puts "};"

  f.puts "std::uint8_t const #{NAME}_palette_bot[#{$colors_used}] {"
  (0...$colors_used).each { |pi| f.puts "  0x#{$pal_bot[pi].to_s(16)}," }
  f.puts "};"

  f.puts "std::uint8_t const #{NAME}_darken[#{$colors_used}] {"
  (0...$colors_used).each { |pi| f.puts "  #{$dark[pi].to_s(10)}," }
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
