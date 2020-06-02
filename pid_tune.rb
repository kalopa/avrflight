#!/usr/bin/env ruby
#

#
# Generate the following parameters (12 bytes):
#   int    kp;
#   int    ki;
#   int    kd;
#   uint_t k_div;
#   uint_t u_mul;
#   uint_t u_div;
#
def pid_generate(kp, ki, kd)
  puts "Generate PID factors for Kp: #{kp}, Ki: #{ki}, Kd: #{kd}."
  params = []
  rkp = kp.rationalize
  rki = ki.rationalize
  rkd = kd.rationalize
  kdiv = rkp.denominator.lcm(rki.denominator.lcm(rkp.denominator))
  umul = udiv = 1
  while true do
    if kdiv > 65535
      kdiv /= 2
      next
    end
    ikp = rkp.numerator * kdiv / rkp.denominator
    iki = rki.numerator * kdiv / rki.denominator
    ikd = rkd.numerator * kdiv / rkd.denominator
    break if ikp <= 65535 and iki <= 65535 and ikd <= 65535
    kdiv /= 2
  end
  params[0] = ikp
  params[1] = iki
  params[2] = ikd
  params[3] = kdiv
  params[4] = 0
  params[5] = 0
  params[6] = umul
  params[7] = udiv
  params
end

data = [0x55aa]
#
# Generate PID data for roll.
data += pid_generate(1.013, 0.3335, 0.2)
#
# Generate PID data for pitch.
data += pid_generate(1.013, 0.1, 0.2)
#
# Generate PID data for yaw.
data += pid_generate(1.013, 0.1, 0.2)

#
# Add value for ESC divider
data += [600]

#
# Now push out the hex codes.
p data
offset = 0
File.open("promdata.eep", "w") do |f|
  len = data.length * 2
  while len > 0
    nbytes = (len > 32) ? 32 : len
    cksum = nbytes
    cksum += (offset / 256) & 0xff
    cksum += (offset & 0xff)
    outstr = ":%02X%04X00" % [nbytes, offset]
    value = 0
    (0...nbytes).each do |i|
      if (i & 1) == 0
        value = data[(offset + i) / 2]
      end
      byte = value & 0xff
      outstr += "%02X" % (byte)
      cksum += byte
      value >>= 8
    end
    outstr += "%02X\n" % (-cksum & 0xff)
    f.puts outstr
    offset += nbytes
    len -= nbytes
  end
  f.puts ":00000001FF"
end
