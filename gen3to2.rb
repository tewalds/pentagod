#!/usr/bin/ruby

def get_bits(i)
	bits = []
	while i > 0
		bits << (i & 1)
		i /= 2
	end
	return (bits + ([0]*10))[0..10]
end


(0...64).each{|i|
	bi = get_bits(i)
	(0...64).each{|j|
		bj = get_bits(j)

		v = 0
		m = 1
		(0...10).each{|k|
			if bi[k] == 1 && bj[k] == 1
				v = 0
				break
			end
			v += m*(2*bi[k] + bj[k])
			m *= 3
		}
		print ('0x' + v.to_s(16).upcase).rjust(5) + ", "
		puts if j % 16 == 15
	}
}
puts

