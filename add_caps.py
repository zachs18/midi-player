import sys
import re

line_re = re.compile("(^.*[[])(.+)([]].*$)")

for line in sys.stdin:
	match = line_re.match(line)
	if not match:
		print("no match", line, file=sys.stderr, end='')
		print(line, end='')
		continue
	a, b, c = match.groups()
	b=int(b)
	print(a, "%5d" % b, c, sep='')
	if 97 <= b <= 122:
		print(a, "%5d" % (b-32), c, sep='')
