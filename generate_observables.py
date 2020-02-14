observable_file = open('generated_observables.txt', 'w')

system_size = 20
print(system_size, file = observable_file)

for i in range(system_size - 1):
    for j in range(system_size - 1):
        if j == i or j == i + 1 or j+1 == i: continue
        print("4 Y {} Y {} X {} X {}".format(i, i+1, j, j+1), file = observable_file)

for i in range(system_size - 1):
    for j in range(system_size):
        if j == i or j == i + 1: continue
        for j2 in range(system_size):
            if j2 == i or j2 == i + 1 or j2 == j: continue
            print("4 X {} X {} Z {} Z {}".format(i, i+1, j, j2), file = observable_file)

for i in range(system_size - 1):
    for j in range(system_size):
        if j == i or j == i + 1: continue
        print("3 X {} X {} Z {}".format(i, i+1, j), file = observable_file)
