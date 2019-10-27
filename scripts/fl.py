from functools import reduce
import math
import sys

def pm(m):
	for l in m:
		for c in l:
			print(c, end=" ")
		print("")

# Multiplicação normal de matriz com soma dos produtos
def ml(m):
	mr = []
	n = len(m)

	for i in range(n):
		mr.append([])
		for j in range(n):
			s = 0
			for k in range(n):
				s += m[i][k] * m[k][j]
			mr[-1].append(s)

	return mr

# Multiplicação alterada de matriz com minimo das somas.
def fml(m):
	mr = []
	n = len(m)

	for i in range(n):
		mr.append([])
		for j in range(n):
			mini = math.inf
			for k in range(n):
				mini = min(mini, m[i][k] + m[k][j])
			mr[-1].append(mini)

	return mr

if len(sys.argv) < 2:
	m = [
		[0, 2, 0, 5, 0, 0],
		[0, 0, 0, 0, 0, 0],
		[0, 2, 0, 0, 0, 5],
		[0, 0, 0, 0, 1, 0],
		[3, 9, 3, 0, 0, 0],
		[0, 0, 0, 0, 1, 0],
	]
	N = len(m)
else:
	with open(sys.argv[1]) as f:
		N = int(f.readline())
		m = []
		for i in range(N):
			linha = f.readline()
			m.append([float(x.strip()) for x in linha.split(" ")])

# Setando os 0s como infinito.
for i in range(len(m)):
	for j in range(len(m[i])):
		if m[i][j] == 0 and i != j:
			m[i][j] = math.inf

i = 1
while i < N:
	r = fml(m)
	#pm(r)
	m = r
	i *= 2
"""
	try:
		input()
	except:
		break
"""

pm(m)
