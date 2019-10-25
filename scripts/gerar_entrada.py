import random

# Quantidade de testes
iters = 15

# Incremento no valor de N (é bom ser um múltiplo de 36)
step = 180

# Probabilidade de ser infinito
pinf = 0.2

# Peso máximo
pmax = 20

n = 36
for i in range(iters):
	print(f"Test {i + 1} / {iters}")
	with open(f"entradas/entrada-{n:06}.txt", "w") as f:
		f.write(f"{n}\n")
		for i in range(n):
			l = []
			for j in range(n):
				if i == j or random.random() < pinf:
					p = 0
				else:
					p = random.randint(0, pmax)
				l.append(p)
			f.write(" ".join([str(x) for x in l]) + "\n")
	n += step
