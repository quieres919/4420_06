#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <vector>
#include <sys/wait.h>
#include <cstdlib>
/// In this lab we will use pipes to implement a majority voting system that involves one judge and N voters. 
/// N is specified by the user and should be equal to or greater than 3. The judge is the parent process, and the voters are child processes. 
/// At each round, every voter will randomly choose to vote yes or no, show the vote on the screen, and send the vote to the judge. 
/// The judge will then declare the final decision following the majority rule and display it on the screen. The
/// voting continues until it reaches the maximal number of rounds, and all processes should be terminated properly before exit. The same group votes for all the rounds.

int main() {
    /// Declarations 
    int N;
    pid_t pid;
    int rounds;
    int voter_id = -1;
    /// Number voters input
    std::cout << "Enter the number of voters (N >= 3): ";
    std::cin >> N;
    if(N < 3){
        std::cerr << "Number of voters < 3. Exiting." << std::endl;
        return 1;
    }

    /// Declare vector for piopes, each pipe is an array of two integers (read and write ends)
    std::vector<int[2]> pipes(N);
    
    /// Number of rounds input
    std::cout << "Enter the number of rounds: ";
    std::cin >> rounds;
    if(rounds < 1){
        std::cerr << "Number of rounds < 1. Exiting." << std::endl;
        return 1;
    }

    /// Judge process creates N pipes using pipe()
    for(int i = 0; i < N; i++){
        if(pipe(pipes[i]) == -1){
            std::cerr << "Pipe creation failed. Exiting." << std::endl;
            return 1;
        }
    }

    /// Judge creates N child processes using fork
    for(int i = 0; i < N; i++){
        pid = fork();
        if(pid < 0){
            std::cerr << "Fork failed. Exiting." << std::endl;
            return 1;
        } else if(pid  == 0){
            /// Child process
            voter_id = i;

            break;
        }
    }

    /// Loop through rounds
    for(int round = 0; round < rounds; round++){
        int yes_votes = 0;
        int no_votes = 0;
        int vote; 

        /// Loop through voters to either vote (child process) or count votes (judge process)
        for(int i = 0; i < N; i++){
            if(voter_id == i){
                /// Child process randomly chooses to vote yes or no, displays the vote, and sends it to the judge
                vote = rand() % 2; // Randomly choose 0 (no) or 1 (yes)
                std::cout << "Voter " << voter_id << " votes: " << (vote == 1 ? "Yes" : "No") << std::endl;
                write(pipes[voter_id][1], &vote, sizeof(vote)); // Send vote to judge
            } else if(voter_id == -1){
                /// Judge process reads votes from all voters and counts them
                read(pipes[i][0], &vote, sizeof(vote)); // Read vote from voter i
                if(vote == 1){
                    yes_votes++;
                } else {
                    no_votes++;
                }
            }
        }
        ///std::cout << "Round " << round + 1 << " results: Yes votes = " << yes_votes << ", No votes = " << no_votes << std::endl;
        ///std::cout << std::endl;
    }
}