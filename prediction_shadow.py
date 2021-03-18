import sys

def estimate_exp(full_measurement, one_observable):
    sum_product, cnt_match = 0, 0

    for single_measurement in full_measurement:
        not_match = 0
        product = 1

        for pauli_XYZ, position in one_observable:
            if pauli_XYZ != single_measurement[position][0]:
                not_match = 1
                break
            product *= single_measurement[position][1]
        if not_match == 1: continue

        sum_product += product
        cnt_match += 1

    return sum_product, cnt_match

if __name__ == "__main__":
    def print_usage():
        print("Usage:\n", file=sys.stderr)
        print("./prediction_shadow -o [measurement.txt] [observable.txt]", file=sys.stderr)
        print("    This option predicts the expectation of local observables.", file=sys.stderr)
        print("    We would output the predicted value for each local observable given in [observable.txt]", file=sys.stderr)

    if len(sys.argv) != 4:
        print_usage()

    with open(sys.argv[2]) as f:
        measurements = f.readlines()
    system_size = int(measurements[0])

    full_measurement = []
    for line in measurements[1:]:
        single_meaurement = []
        for pauli_XYZ, outcome in zip(line.split(" ")[0::2], line.split(" ")[1::2]):
            single_meaurement.append((pauli_XYZ, int(outcome)))
        full_measurement.append(single_meaurement)

    with open(sys.argv[3]) as f:
        content = f.readlines()
    assert(system_size == int(content[0]))

    for line in content[1:]:
        one_observable = []
        for pauli_XYZ, position in zip(line.split(" ")[1::2], line.split(" ")[2::2]):
            one_observable.append((pauli_XYZ, int(position)))
        sum_product, cnt_match = estimate_exp(full_measurement, one_observable)
        print(sum_product / cnt_match)
