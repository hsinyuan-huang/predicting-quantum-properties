//
// This code is created by Hsin-Yuan Huang (https://momohuang.github.io/).
// For more details, see the accompany paper:
//  "Predicting Many Properties of a Quantum System from Very Few Measurements".
//
#include <stdio.h>
#include <cmath>
#include <vector>
#include <sys/time.h>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <utility>
#include <algorithm>

using namespace std;

int system_size = -1;
int number_of_observables;

double renyi_sum_of_binary_outcome[100000000];
double renyi_number_of_outcomes[100000000];

//
// The following function reads the file: observable_file_name
// and updates [observables] and [observables_acting_on_ith_qubit]
//
vector<vector<pair<int, int> > > observables; // observables to predict
vector<vector<vector<int> > > observables_acting_on_ith_qubit;
void read_all_observables(char* observable_file_name){
    ifstream observable_fstream;
    observable_fstream.open(observable_file_name, ifstream::in);

    if(observable_fstream.fail()){
        fprintf(stderr, "\n====\nError: the input file \"%s\" does not exist.\n====\n", observable_file_name);
        exit(-1);
    }

    // Read in the system size
    int system_size_observable;
    observable_fstream >> system_size_observable;
    if(system_size == -1) system_size = system_size_observable;

    // Initialize the following numbers (will be changed later)
    number_of_observables = 0;

    //
    // The grammar of observables_acting_on_ith_qubit:
    //   observables_acting_on_ith_qubit[ith_qubit][0 or 1 or 2]
    //     returns a list of indices for observables that applies X (0), Y (1), Z (2) on the ith_qubit
    //
    observables_acting_on_ith_qubit.clear();
    vector<int> single_list;
    vector<vector<int> > pauli_list;
    pauli_list.push_back(single_list);
    pauli_list.push_back(single_list);
    pauli_list.push_back(single_list);
    for(int i = 0; i < system_size; i++){
        observables_acting_on_ith_qubit.push_back(pauli_list);
    }

    // Read in the local observables line by line
    string line;
    int observable_counter = 0;
    while(getline(observable_fstream, line)){
        if(line == "\n" || line == "") continue;
        istringstream single_line_stream(line);

        int k_local;
        single_line_stream >> k_local;

        vector<pair<int, int> > ith_observable;

        for(int k = 0; k < k_local; k++){
            char pauli_observable[5];
            int position_of_pauli;
            single_line_stream >> pauli_observable >> position_of_pauli;

            assert(pauli_observable[0] == 'X' || pauli_observable[0] == 'Y' || pauli_observable[0] == 'Z');

            int pauli_encoding = pauli_observable[0] - 'X'; // X -> 0, Y -> 1, Z -> 2

            observables_acting_on_ith_qubit[position_of_pauli][pauli_encoding].push_back(observable_counter);
            ith_observable.push_back(make_pair(position_of_pauli, pauli_encoding));
        }

        observables.push_back(ith_observable);
        observable_counter ++;
    }
    number_of_observables = observable_counter;
    observable_fstream.close();

    return;
}

//
// The following function reads the file: subsystem_file_name
// and updates [subsystems].
//
vector<vector<int> > subsystems; // subsystems to predict entropy
void read_all_subsystems(char* subsystem_file_name){
    ifstream subsystem_fstream;
    subsystem_fstream.open(subsystem_file_name, ifstream::in);

    if(subsystem_fstream.fail()){
        fprintf(stderr, "\n====\nError: the input file \"%s\" does not exist.\n====\n", subsystem_file_name);
        exit(-1);
    }

    // Read in the system size
    int system_size_subsystem;
    subsystem_fstream >> system_size_subsystem;
    if(system_size == -1) system_size = system_size_subsystem;

    // Read in the local observables line by line
    string line;
    int observable_counter = 0;
    while(getline(subsystem_fstream, line)){
        if(line == "\n" || line == "") continue;
        istringstream single_line_stream(line);

        int k_local;
        single_line_stream >> k_local;

        vector<int> ith_subsystem;

        for(int k = 0; k < k_local; k++){
            int position_of_the_qubit;
            single_line_stream >> position_of_the_qubit;
            ith_subsystem.push_back(position_of_the_qubit);
        }

        subsystems.push_back(ith_subsystem);
    }
    subsystem_fstream.close();

    return;
}

//
// The following function reads the file: measurement_file_name
// and updates [observables] and [observables_acting_on_ith_qubit]
//
vector<vector<int> > measurement_pauli_basis;
vector<vector<int> > measurement_binary_outcome;
void read_all_measurements(char* measurement_file_name){
    ifstream measurement_fstream;
    measurement_fstream.open(measurement_file_name, ifstream::in);

    if(measurement_fstream.fail()){
        fprintf(stderr, "\n====\nError: the input file \"%s\" does not exist.\n====\n", measurement_file_name);
        exit(-1);
    }

    // Read in the system size
    int system_size_measurement;
    measurement_fstream >> system_size_measurement;
    if(system_size == -1) system_size = system_size_measurement;
    if(system_size_measurement != system_size){
        fprintf(stderr, "\n====\nError: the system size do not match.\n====\n");
        exit(-1);
    }

    // Read in the measurements line by line
    string line;
    int measurement_counter = 0;
    while(getline(measurement_fstream, line)){
        if(line == "\n" || line == "") continue;
        istringstream single_line_stream(line);

        vector<int> empty_list;
        measurement_pauli_basis.push_back(empty_list);
        measurement_binary_outcome.push_back(empty_list);
        for(int ith_qubit = 0; ith_qubit < system_size; ith_qubit++){
            char pauli[10];
            int binary_outcome;
            single_line_stream >> pauli >> binary_outcome;
            assert(binary_outcome == 1 || binary_outcome == -1);

            measurement_pauli_basis[measurement_counter].push_back(pauli[0] - 'X');
            measurement_binary_outcome[measurement_counter].push_back(binary_outcome);
        }

        measurement_counter ++;
    }
}

//
// The following function prints the usage of this program.
//
void print_usage(){
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "./prediction_shadow -o [measurement.txt] [observable.txt]\n");
    fprintf(stderr, "    This option predicts the expectation of local observables.\n");
    fprintf(stderr, "    We would output the predicted value for each local observable given in [observable.txt]\n");
    fprintf(stderr, "<or>\n");
    fprintf(stderr, "./prediction_shadow -e [measurement.txt] [subsystem.txt]\n");
    fprintf(stderr, "    This option predicts the Renyi entanglement entropy.\n");
    fprintf(stderr, "    We would output the predicted entropy for each subsystem given in [subsystem.txt]\n");
    return;
}

int main(int argc, char* argv[]){
    if(argc != 4){
        print_usage();
        return -1;
    }

    //
    // Running the prediction of local observables
    //
    if(strcmp(argv[1], "-o") == 0){
        read_all_measurements(argv[2]);
        read_all_observables(argv[3]);

        // For every observable,
        // how many Pauli operators need to be matched to measure the observable
        // in the current measurement repetition.
        vector<int> how_many_pauli_to_match;
        how_many_pauli_to_match.resize(number_of_observables);

        // For every observable,
        // store the measurement up to this single-qubit measurement.
        vector<int> cumulative_measurement;
        cumulative_measurement.resize(number_of_observables);

        // For every observable,
        // store the number of times it has been measured.
        vector<int> number_of_measurements;
        number_of_measurements.resize(number_of_observables);

        // For every observable,
        // store the number of times it has been measured.
        vector<int> sum_of_measurement_results;
        sum_of_measurement_results.resize(number_of_observables);

        // Run through the measurement data:
        //    measurement_pauli_basis, measurement_binary_outcome
        // to compute the local observables
        for(int t = 0; t < (int)measurement_pauli_basis.size(); t++){
            for(int i = 0; i < (int)observables.size(); i++){
                how_many_pauli_to_match[i] = observables[i].size(); // initialize to k for k-local observable
                cumulative_measurement[i] = 1; // initialize to 1
            }

            for(int ith_qubit = 0; ith_qubit < system_size; ith_qubit++){
                int pauli = measurement_pauli_basis[t][ith_qubit];
                int binary_outcome = measurement_binary_outcome[t][ith_qubit];
                for(int i : observables_acting_on_ith_qubit[ith_qubit][pauli]){
                    how_many_pauli_to_match[i] --;
                    cumulative_measurement[i] *= binary_outcome;
                }
            }

            for(int i = 0; i < (int)observables.size(); i++){
                if(how_many_pauli_to_match[i] == 0){
                    number_of_measurements[i] ++;
                    sum_of_measurement_results[i] += cumulative_measurement[i];
                }
            }
        }

        for(int i = 0; i < (int)observables.size(); i++){
            if(number_of_measurements[i] == 0){
                fprintf(stderr, "%d-th Observable is not measured at all\n", i+1);
                printf("0\n");
            }
            printf("%f\n", 1.0 * sum_of_measurement_results[i] / number_of_measurements[i]);
        }
    }
    //
    // Running the prediction of entanglement entropy
    //
    else if(strcmp(argv[1], "-e") == 0){
        read_all_measurements(argv[2]);
        read_all_subsystems(argv[3]);

        for(int s = 0; s < (int)subsystems.size(); s++){
            int subsystem_size = (int)subsystems[s].size();


            for(int c = 0; c < (1 << (2 * subsystem_size)); c++){
                renyi_sum_of_binary_outcome[c] = 0;
                renyi_number_of_outcomes[c] = 0;
            }

            for(int t = 0; t < (int)measurement_pauli_basis.size(); t++){
                long long encoding = 0, cumulative_outcome = 1;

                renyi_sum_of_binary_outcome[0] += 1;
                renyi_number_of_outcomes[0] += 1;

                // Using gray code iteration over all 2^n possible outcomes
                for(long long b = 1; b < (1 << subsystem_size); b++){
                    long long change_i = __builtin_ctzll(b);
                    long long index_in_original_system = subsystems[s][change_i];

                    cumulative_outcome *= measurement_binary_outcome[t][index_in_original_system];
                    encoding ^= (measurement_pauli_basis[t][index_in_original_system] + 1) << (2LL * change_i);

                    renyi_sum_of_binary_outcome[encoding] += cumulative_outcome;
                    renyi_number_of_outcomes[encoding] += 1;
                }
            }

            int level_cnt[2 * subsystem_size], level_ttl[2 * subsystem_size];
            for(int i = 0; i < subsystem_size + 1; i++){
                level_cnt[i] = 0;
                level_ttl[i] = 0;
            }

            for(long long c = 0; c < (1 << (2 * subsystem_size)); c++){
                int nonId = 0;
                for(int i = 0; i < subsystem_size; i++){
                    nonId += ((c >> (2 * i)) & 3) != 0;
                }
                if(renyi_number_of_outcomes[c] >= 2)
                    level_cnt[nonId] ++;
                level_ttl[nonId] ++;
            }

            double predicted_entropy = 0;
            for(long long c = 0; c < (1 << (2 * subsystem_size)); c++){
                if(renyi_number_of_outcomes[c] <= 1) continue;

                int nonId = 0;
                for(int i = 0; i < subsystem_size; i++)
                    nonId += ((c >> (2 * i)) & 3) != 0;

                predicted_entropy += ((double)1.0) / (renyi_number_of_outcomes[c] * (renyi_number_of_outcomes[c] - 1)) * (renyi_sum_of_binary_outcome[c] * renyi_sum_of_binary_outcome[c] - renyi_number_of_outcomes[c]) / (1LL << subsystem_size) * level_ttl[nonId] / level_cnt[nonId];
            }

            printf("%f\n", -1.0 * log2(min(max(predicted_entropy, 1.0 / pow(2.0, subsystem_size)), 1.0 - 1e-9)));
        }
    }
    //
    // None of the above holds (the input is invalid)
    //
    else{
        print_usage();
        return -1;
    }
}
