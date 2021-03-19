# Predicting Properties of Quantum Many-Body Systems

This open source implementation allows the prediction of many properties: few-body observables, two-point correlation functions, subsystem entanglement entropy, from very few measurements.

We require `g++` and `python` version 3.

On the experimental side, we require **single-qubit Pauli measurements** (i.e., each measurement measures all qubits in some Pauli X, Y, or Z- basis). This should be readily available in most quantum machines.

An introduction to this procedure and the underlying theory can be found in our papers:

- Efficient estimation of Pauli observables by derandomization: https://arxiv.org/abs/2103.07510.

- Predicting Many Properties of a Quantum System from Very Few Measurements: https://arxiv.org/abs/2002.08953, https://www.nature.com/articles/s41567-020-0932-7.

### Quick Start

The following is for using a command line interface to run the C++ implementation. The most important code in this repo is the creation of measurement schemes, see `data_acquisition_shadow.cpp` or `data_acquisition_shadow.py`.

```shell
# Compile the codes
> g++ -std=c++0x -O3 data_acquisition_shadow.cpp -o data_acquisition_shadow
> g++ -std=c++0x -O3 prediction_shadow.cpp -o prediction_shadow

# Generate observables you want to predict
> g++ -O3 -std=c++0x generate_observables.cpp -o generate_observables
> ./generate_observables

# Create measurement scheme (stored in scheme.txt) using derandomized version of classical shadows
> ./data_acquisition_shadow -d 100 generated_observables.txt 1> scheme.txt

# Do the physical experiments
# Store the data in measurement.txt

# Predicting many few-body observables
> ./prediction_shadow -o measurement.txt observables.txt
# Predicting many subsystem entanglement entropy
> ./prediction_shadow -e measurement.txt subsystems.txt
```

Since many people are using Python, we have implemented `data_acquisition_shadow.py` which is the Python version of `data_acquisition_shadow.cpp` and `prediction_shadow.py` which is the Python version of `prediction_shadow.cpp`. The purpose of the two codes is only to facilitate understanding of the procedure and it could be orders of magnitude slower than the C++ implementation. It can be used through the command line interface
```shell
> python data_acquisition_shadow.py -d 10 observables.txt
> python prediction_shadow.py -o measurement.txt observables.txt
```
or by importing into a Python code
```python
import data_acquisition_shadow
import prediction_shadow

# randomized classical shadow consisting of 100 parallel measurements in a 20-qubit system
measurement_procedure = data_acquisition_shadow.randomized_classical_shadow(100, 20)

# measurement_procedure = [a list of 100 parallel measurements, each being [a list of 20 single-qubit Pauli bases]]
print(measurement_procedure)

# full_measurement = [a list of [list of (XYZ basis, +-1 outcome) for each qubit]]
# one_observable = [a list of (Pauli-XYZ, index for the qubit)]
estimate_exp(full_measurement, one_observable)
```
Currently, `prediction_shadow.py` only support the `-o` option for prediction expectation value of observables.

### Step 1: Compile the code
In your terminal, perform the following to compile the C++ codes to executable files:
```shell
> g++ -std=c++0x -O3 data_acquisition_shadow.cpp -o data_acquisition_shadow
> g++ -std=c++0x -O3 prediction_shadow.cpp -o prediction_shadow
```

### Step 2: Prepare the measurements
The executable `data_acquisition_shadow` could be used to produce an efficient measurement scheme for predicting many local properties from very few measurements. There are two ways to use this program:

#### 1. Randomized measurements:
```shell
> ./data_acquisition_shadow -r [number of measurements] [system size]
```
This generates random Pauli measurements. There would be `[number of measurements]` repetitions on a system with `[system size]` qubits.
You may then use this set of randomized measurements to perform the experiment.

##### A concrete example of using the randomized measurements:
```shell
> ./data_acquisition_shadow -r 5 3
X X Z
Y Z X
Y Z Z
X X Z
Y X Z
```
A random output of 5 measurement repetitions for a system of 3 qubits is outputted.

#### 2. Derandomized measurements:
```shell
> ./data_acquisition_shadow -d [measurements per observable] [observable file]
```
If you already know a list of local observables you want to measure, this option creates a derandomized version of the randomized measurement.
We consider local observables as observables that act on few qubits but do not have to be geometrically local.
Furthermore, since every local observables could be decomposed to tensor product of Pauli matrices, we only focus on tensor product of Pauli matrices.
The generated measurement scheme would allow you to measure all the local observables given in `[observable file]` for at least `[measurements per observable]` times.
**An important note** is that: if we increase `[measurements per observable]`, the ratio `number of measurement repetitions` / `[measurements per observable]` would not be constant, but would actually **decrease**.
This is because the derandomization procedure would find a more efficient approach to measure the observables.

The format of the `[observable file]` is given as follows.
An example of a list of local observables could be found in `observables.txt`.
```
[number of qubits / system size]
[k-local] X/Y/Z [ith qubit] X/Y/Z [jth qubit] ...
[k-local] X/Y/Z [ith qubit] X/Y/Z [jth qubit] ...
...
```
We consider `[ith qubit]` to take value from `0` to `[number of qubits / system size] - 1`.

One could also consider Pauli observables with weights given as follows.
```
[number of qubits / system size]
[k-local] X/Y/Z [ith qubit] X/Y/Z [jth qubit] ... [weight]
[k-local] X/Y/Z [ith qubit] X/Y/Z [jth qubit] ... [weight]
...
```
We consider `[weight]` to be a floating point number in between `0.0` to `1.0`. A smaller weight means the observable is less important and will be measured less often in the derandomization procedure.

##### Concrete Examples of using the derandomized measurements:

This example consider measuring all the observables in the example file `observable.txt` at least 1 time. The output is the measurement basis for each repetition interleaved with `[Status T: X]`. `T` stands for `T`-th measurement repetitions and `X` stands for the minimum number of measurements in all the observables we hope to predict.
```shell
# A pre-specified [observable file]: observables.txt
> ./data_acquisition_shadow -d 1 observables.txt
X Y X Y Y X Y X Y X
[Status 1: 0]
X X Y X Y Y X Y X Y
[Status 2: 0]
X X X X X Y X X Y X
[Status 3: 0]
X X X X X X X X X X
[Status 4: 1]
```

The following example consider a much longer list of local observables.
We first generate the file `generated_observables.txt` consisting of a large number of 3- and 4-local observables.
Then we use `data_acquisition_shadow` to generate an efficient measurement scheme for measuring all local observables for at least 100 times.

```shell
# Automatic generation of [observable file] using Python
> python generate_observables.py
> ./data_acquisition_shadow -d 100 generated_observables.txt
```

Alternatively, we show an example using C++ to generate the file containing the list of observables. C++ is much more efficient than Python, because C++ is a lower-level language.
```shell
# Automatic generation of [observable file] using C++
> g++ -O3 -std=c++0x generate_observables.cpp -o generate_observables
> ./generate_observables
> ./data_acquisition_shadow -d 100 generated_observables.txt
```

Because the generated measurement schemes could be quite long, we provide the following options based on shell commands (`1>` and `2>`).

```shell
# Some more advanced options using shell commands
# + Output the measurement scheme to a file
> ./data_acquisition_shadow -d 100 generated_observables.txt 1> scheme.txt
# + Don't output [Status X: X]
> ./data_acquisition_shadow -d 100 generated_observables.txt 2> /dev/null
# + Do both
> ./data_acquisition_shadow -d 100 generated_observables.txt 1> scheme.txt 2> /dev/null
```

### Step 3: Perform the measurements
Perform physical experiments using the generated scheme to gather the measurement data. The `[measurement file]` should be structured as follows. An example of the format is given in `measurement.txt`.
```
[system size]
[X/Y/Z for qubit 1] [-1/1 for qubit 1] ...
[X/Y/Z for qubit 1] [-1/1 for qubit 1] ...
...
```
The first line consists of the number of qubits `[system size]` in the quantum system.
A single-shot measurement result is recorded in each of the following lines.
A single-shot measurement result consists of whether we measure in X, Y, Z-basis for each qubit, and the corresponding binary measurement outcome.

**Advanced tips:**
In practice, it may be more economic to run the same measurement schemes for multiple times. Our framework / program could also operate properly in such a scenario.
As the repetition increases, the prediction would also become more accurate.
However, the ratio between the number of measurement bases (`Nb`) and the number of repetitions (`Nr`) for each basis is an important hyper-parameter that should be properly tuned.

For local observables, when `N = Nb x Nr` is fixed, it is preferred to have `Nr = 1`. However, if changing the bases every time would be less economic, then you could also try a larger `Nr > 1`.

For entanglement entropy, when `N = Nb x Nr` is fixed, `Nr = 1` may no longer be preferable. We should consider `Nr` as a hyper-parameter, and try `Nr = 1, 2, 4, 8, 16, 32, 64, 128, ...` to see which yields the best performance.

### Step 4: Predict physical properties
The executable `prediction_shadow` could be used to predict many local properties from the measurement data obtained in Step 3. There are two ways to use this program:

#### 1. Local observables:
```shell
> ./prediction_shadow -o [measurement.txt] [observable.txt]
```
This command allows the prediction of local observables, where the observables act on few qubits but do not have to be geometrically local.
The prediction is based on the measurement data `[measurement.txt]` obtained in Step 3. An example of the measurement txt file is given in `measurement.txt`.
The local observables are given in `[observable.txt]`. The format can be found in Step 1. An example of the observable txt file is given in `observables.txt`.

##### A concrete example for predicting local observables:
```shell
> ./prediction_shadow -o measurement.txt observables.txt
0.049015
0.006748
0.014625
-0.005204
0.017512
-0.024036
-0.022046
-0.002698
-0.038271
0.003201
-0.023298
-0.003155
-0.003189
0.002247
1.000000
1.000000
```
This predicts 16 local observables given in `observables.txt` from the randomized measurements given in `measurement.txt`.
The randomized measurements are performed on a system of 10 qubits, where two consecutive qubits form [a singlet state](https://en.wikipedia.org/wiki/Singlet_state) (a total of 5 singlet states).

#### 2. Subsystem entanglement entropy:
```shell
> ./prediction_shadow -e [measurement.txt] [subsystem.txt]
```
This command allows the prediction of subsystem entanglement entropy. In particular, we output [Renyi entropy of order 2](https://en.wikipedia.org/wiki/R%C3%A9nyi_entropy).
`[measurement.txt]` is the measurement data obtained in Step 3. An example of the measurement txt file is given in `measurement.txt`.
`[subsystem.txt]` contains a list of subsystems that we want to predict their entanglement entropy. The format of `[subsystem.txt]` is
```
[system size]
[subsystem 1 size] [position of qubit 1] [position of qubit 2] ...
[subsystem 2 size] [position of qubit 1] [position of qubit 2] ...
```
The first line `[system size]` indicates the number of qubits in the entire system.
In each of the following line, we specify one subsystem.
`[subsystem T size]` is the size of the `T`-th subsystem.
Then we specify what are the positions for the qubits in the `T`-th subsystem. `[position of qubit X]` is the position of the `X`-th qubit in the subsystem.
An example can be found in `subsystems.txt`.

##### A concrete example for predicting entanglement entropy:

```shell
> ./prediction_shadow -e measurement.txt subsystems.txt
0.000000
2.000000
0.000277
1.996378
1.035726
0.003031
```
This predicts the entanglement entropy for six subsystems given in `subsystems.txt` from the randomized measurements given in `measurement.txt`.
The randomized measurements are performed on a system of 10 qubits, where two consecutive qubits form [a singlet state](https://en.wikipedia.org/wiki/Singlet_state) (a total of 5 singlet states).
