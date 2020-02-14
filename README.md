# Predicting Many Properties of a Quantum System using Very Few Measurements

We require `g++` and `python` (version 3).

### Step 1:
In your terminal, perform the following to compile the C++ codes to executable files:
```shell
> g++ -std=c++0x -O3 data_acquisition_shadow.cpp -o data_acquisition_shadow
> g++ -std=c++0x -O3 prediction_shadow.cpp -o prediction_shadow
```

### Step 2:
The executable `data_acquisition_shadow` could be used to produce an efficient measurement scheme for predicting many local properties from very few measurements. There are two ways to use this program:

#### 1. Randomized measurements:
```shell
> ./data_acquisition_shadow -r [number of measurements] [system size]
```
This generates random Pauli measurements. There would be `[number of measurements]` repetitions on a system with `[system size]` qubits.
You may then use this set of randomized measurements to perform the experiment.

###### A concrete example of using the randomized measurements:
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

###### Concrete Examples of using the derandomized measurements:

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
X X X X X X X X X Y
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

### Step 3:
Perform physical experiments using the generated scheme to gather the measurement data. The `[measurement file]` should be structured as follows.
```
[system size]
[X/Y/Z for qubit 1] [0/1 for qubit 1] ...
[X/Y/Z for qubit 1] [0/1 for qubit 1] ...
...
```
The first line consists of the number of qubits `[system size]` in the quantum system.
For each of the following line, it is a single-shot measurement result.
A single-shot measurement result consists of measuring in X, Y, Z for each qubit, and the corresponding binary measurement outcome.

**Advanced tips:**
In practice, it may be more economic to run the same measurement schemes for multiple times. Our framework / program could also operate properly in such a scenario.
As the repetition increases, the prediction would also become more accurate.
However, one should be careful in finding the best ratio between the number of measurement bases and the number of repetitions for each basis.



### Step 4:
