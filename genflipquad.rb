#!/usr/bin/ruby
(0...512).each{|i|
	puts "" if (i % 16) == 0
	bits = ""
	(0...9).each{|j|
		bits << (i & 1).to_s
		i /= 2
	}
	bits = bits.reverse
	bits[1...8] = bits[1...8].reverse
	print ('0x' + (bits.to_i(2)).to_s(16).upcase).rjust(5) + ", "
}
puts

