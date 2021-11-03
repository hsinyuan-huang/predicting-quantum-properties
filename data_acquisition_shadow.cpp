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
#include <cassert>
#include <sstream>
#include <utility>
#include <algorithm>

using namespace std;
const int INF = 999999999; // This is a very large number we call infinity

double eta = 0.9; // This is a hyperparameter that should be tuned

int system_size;
int number_of_observables;
int number_of_measurements_per_observable;
int max_k_local;

//
// The following function reads the file: observable_file_name
// and updates [observables] and [observables_acting_on_ith_qubit]
//
vector<vector<pair<int, int> > > observables; // observables to predict
vector<vector<vector<int> > > observables_acting_on_ith_qubit;
vector<double> observables_weight;

void read_all_observables(char* observable_file_name){
    ifstream observable_fstream;
    observable_fstream.open(observable_file_name, ifstream::in);

    if(observable_fstream.fail()){
        fprintf(stderr, "\n====\nError: the input file \"%s\" does not exist.\n====\n", observable_file_name);
        exit(-1);
    }

    // Read in the system size
    observable_fstream >> system_size;

    // Initialize the following numbers (will be changed later)
    max_k_local = 0;
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
        if(line == "\n") continue;
        istringstream single_line_stream(line);

        int k_local;
        single_line_stream >> k_local;
        max_k_local = max(max_k_local, k_local);

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

        double weight;
        int X = single_line_stream.rdbuf()->in_avail();
        if(X == 0) weight = 1.0;
        else single_line_stream >> weight;

        observables_weight.push_back(weight);

        observables.push_back(ith_observable);
        observable_counter ++;
    }
    number_of_observables = observable_counter;
    observable_fstream.close();

    return;
}

//
// The following function prints the usage of this program.
//
void print_usage(){
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "./shadow_data_acquisition -d [number of measurements per observable] [observable.txt]\n");
    fprintf(stderr, "    This is the derandomized version of classical shadow.\n");
    fprintf(stderr, "    We would output a list of Pauli measurements to measure all observables\n");
    fprintf(stderr, "    in [observable.txt] for at least [number of measurements per observable] times.\n");
    fprintf(stderr, "<or>\n");
    fprintf(stderr, "./shadow_data_acquisition -r [number of total measurements] [system size]\n");
    fprintf(stderr, "    This is the randomized version of classical shadow.\n");
    fprintf(stderr, "    We would output a list of Pauli measurements for the given [system size]\n");
    fprintf(stderr, "    with a total of [number of total measurements] repetitions.\n");
    return;
}

//
// The following function performs multiplicative weight update,
// which is used in derandomizing the random Pauli measurement for classical shadows.
//
vector<double> log1ppow1o3k; // log1ppow1o3k[k] = log(1 + (e^(-eta / 2) - 1) / 3^k)
double sum_log_value = 0.0;
int sum_cnt = 0.0;
double fail_prob_pessimistic(int cur_num_of_measurements, int how_many_pauli_to_match, double weight, double shift){ // stands for "failure probability by pessimistic estimator"
    double log1pp0 = (how_many_pauli_to_match < INF? log1ppow1o3k[how_many_pauli_to_match] : 0.0);

    if(floor(weight * number_of_measurements_per_observable) <= cur_num_of_measurements)
        return 0;

    double log_value = -eta / 2 * cur_num_of_measurements + log1pp0;
    sum_log_value += (log_value / weight);
    sum_cnt ++;
    return 2 * exp((log_value / weight) - shift);
}

int main(int argc, char* argv[]){
    if(argc != 4){
        print_usage();
        return -1;
    }

    //
    // Running the randomized version of classical shadows
    //
    if(strcmp(argv[1], "-r") == 0){
        //
        // Setup random seed for this run
        //
        struct timeval time;
        gettimeofday(&time,NULL);
        srand((time.tv_sec * 1000) + (time.tv_usec / 1000));

        //
        // Read in the parameter
        //
        system_size = stoi(argv[3]);
        int number_of_total_measurements = stoi(argv[2]);
        char Pauli[] = {'X', 'Y', 'Z'};

        //
        // Randomized version of classical shadows
        //
        for(int i = 0; i < number_of_total_measurements; i++){
            for(int j = 0; j < system_size; j++){
                printf("%c ", Pauli[rand() % 3]);
            }
            printf("\n");
        }
    }
    //
    // Running the derandomized version of classical shadows
    //
    else if(strcmp(argv[1], "-d") == 0){
        read_all_observables(argv[3]);

        //
        // Precompute some constants for efficient usage in the derandomization process
        //
        double expm1eta = expm1(-eta / 2); // expm1eta = e^(-eta / 2) - 1
        for(int k = 0; k < max_k_local+1; k++){
            log1ppow1o3k.push_back(log1p(pow(1.0/3.0, k) * expm1eta));
        }

        //
        // We want to measure each local observable this many times
        //
        number_of_measurements_per_observable = stoi(argv[2]);

        //
        // Derandomized version of classical shadows
        //

        // For every observable,
        // how many times the observable has been measured
        // in all previous measurement repetitions
        vector<int> cur_num_of_measurements; // stands for "current number of measurements"
        cur_num_of_measurements.resize(number_of_observables, 0); // initialize to zero

        // For every observable,
        // how many Pauli operators need to be matched to measure the observable
        // in the current measurement repetition.
        vector<int> how_many_pauli_to_match;
        how_many_pauli_to_match.resize(number_of_observables);

        for(int measurement_repetition = 0; measurement_repetition < INF; measurement_repetition++){
            for(int i = 0; i < (int)observables.size(); i++)
                how_many_pauli_to_match[i] = observables[i].size(); // initialize to k for k-local observable

            double shift = (sum_cnt == 0)? 0: sum_log_value / sum_cnt;
            sum_log_value = 0.0;
            sum_cnt = 0;

            for(int ith_qubit = 0; ith_qubit < system_size; ith_qubit++){
                double prob_of_failure[3]; // for choosing X, Y, or Z
                double smallest_prob_of_failure = -1;

                //
                // if we choose to measure pauli for ith_qubit in the current repetition
                //
                for(int pauli = 0; pauli < 3; pauli ++){
                    prob_of_failure[pauli] = 0;

                    // for every Pauli observable p, we can calculate a score
                    for(int p = 0; p < 3; p ++){
                        for(int i : observables_acting_on_ith_qubit[ith_qubit][p]){
                            if(pauli == p){
                                int pauli_to_match_next_step = how_many_pauli_to_match[i] == INF? INF: how_many_pauli_to_match[i]-1;
                                double prob_next_step = fail_prob_pessimistic(cur_num_of_measurements[i], pauli_to_match_next_step, observables_weight[i], shift);
                                double prob_current_step = fail_prob_pessimistic(cur_num_of_measurements[i], how_many_pauli_to_match[i], observables_weight[i], shift);
                                prob_of_failure[pauli] += prob_next_step - prob_current_step;
                            }
                            else{
                                double prob_next_step = fail_prob_pessimistic(cur_num_of_measurements[i], INF, observables_weight[i], shift);
                                double prob_current_step = fail_prob_pessimistic(cur_num_of_measurements[i], how_many_pauli_to_match[i], observables_weight[i], shift);
                                prob_of_failure[pauli] += prob_next_step - prob_current_step;
                            }
                        }
                    }

                    if(smallest_prob_of_failure == -1)
                        smallest_prob_of_failure = prob_of_failure[pauli];
                    else
                        smallest_prob_of_failure = min(smallest_prob_of_failure, prob_of_failure[pauli]);
                }

                // Pick one with lowest failure probability
                int the_best_pauli = 0;
                for(int pauli = 0; pauli < 3; pauli ++){
                    if(smallest_prob_of_failure == prob_of_failure[pauli]){
                        printf("%c ", 'X' + pauli);
                        the_best_pauli = pauli;
                        break;
                    }
                }

                for(int pauli = 0; pauli <= 2; pauli ++){
                    for(int i : observables_acting_on_ith_qubit[ith_qubit][pauli]){
                        if(the_best_pauli == pauli){
                            if(how_many_pauli_to_match[i] != INF)
                                how_many_pauli_to_match[i] -= 1;
                        }
                        else how_many_pauli_to_match[i] = INF;
                    }
                }
            }
            printf("\n");

            for(int i = 0; i < (int)observables.size(); i++)
                if(how_many_pauli_to_match[i] == 0) cur_num_of_measurements[i] ++;

            //
            // Check the number of measurements for all the observables
            //
            int success = 0;
            for(int i = 0; i < (int)observables.size(); i++)
                if(cur_num_of_measurements[i] >= floor(observables_weight[i] * number_of_measurements_per_observable))
                    success += 1;
            fprintf(stderr, "[Status %d: %d]\n", measurement_repetition+1, success);

            if(success == (int)observables.size()) break;
        }
    }
}
