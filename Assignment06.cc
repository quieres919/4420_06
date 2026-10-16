#include <iostream> 
#include <unistd.h> 
#include <sys/types.h> 
#include <vector> 
#include <array>
#include <sys/wait.h> 
#include <cstdlib> 
#include <ctime> 
#include <random> 

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
    std::mt19937 generator; 

    /// Number voters input 
    std::cout << "Enter the number of voters (N >= 3): "; 
    std::cin >> N; 
    if(N < 3){ 
        std::cerr << "Number of voters < 3. Exiting." << std::endl; 
        return 1; 
    } 

    /// Declare vector for pipes, each pipe is an array of two integers (read and write ends) 
    std::vector<std::array<int, 2>> pipes(N); 
    std::vector<std::array<int, 2>> round_control(N); 

    /// Number of rounds input 
    std::cout << "Enter the number of rounds: "; 
    std::cin >> rounds; 
    if(rounds < 1){ 
        std::cerr << "Number of rounds < 1. Exiting." << std::endl; 
        return 1; 
    } 

    /// Judge process creates N pipes using pipe() 
    for(int i = 0; i < N; i++){ 
        if(pipe(pipes[i].data()) == -1){ 
            std::cerr << "Pipe creation failed. Exiting." << std::endl; 
            return 1; 
        } 
        if(pipe(round_control[i].data()) == -1){ 
            std::cerr << "Round control pipe creation failed. Exiting." << std::endl; 
            return 1; 
        } 
    } 

    /// Judge creates N child processes using fork 
    for(int i = 0; i < N; i++){ 
        pid = fork(); 
        if(pid < 0){ 
            std::cerr << "Fork failed. Exiting." << std::endl; 
            return 1; 
        } else if(pid == 0){ 
            /// Child process 
            voter_id = i; 
            std::seed_seq seed{ 
                static_cast<unsigned int>(std::time(nullptr)), 
                static_cast<unsigned int>(getpid()), 
                static_cast<unsigned int>(voter_id) 
            }; 
            generator.seed(seed); 

            break; 
        } 
    } 

    /// Loop through rounds 
    for(int round = 0; round < rounds; round++){ 
        int yes_votes = 0; 
        int no_votes = 0; 
        int vote;  

        /// Judge process signals the start of the round to all voters using the round_control pipes 
        if(voter_id == -1){ 
            std::cout << "Round " << round + 1 << " begins." << std::endl; 
            char start_round = 1; 
            for(int i = 0; i < N; i++){ 
                write(round_control[i][1], &start_round, sizeof(start_round)); 
            } 
        } else { 
            /// Child process waits for the start signal from the judge 
            char start_round; 
            read(round_control[voter_id][0], &start_round, sizeof(start_round)); 
        } 

        /// Judge collects votes; each voter sends its vote through its own pipe
        if(voter_id == -1){ 
            /// Judge process reads votes from all voters and counts them
            for(int i = 0; i < N; i++){ 
                read(pipes[i][0], &vote, sizeof(vote)); 

                if(vote == 1){ 
                    yes_votes++; 
                } else { 
                    no_votes++; 
                } 
            }

            /// Final judge process declares the final decision following the majority rule and displays it on the screen
            if(yes_votes > no_votes){ 
                std::cout << "Round " << round + 1 << ": Final decision is YES" << std::endl; 
            } else if(no_votes > yes_votes){ 
                std::cout << "Round " << round + 1 << ": Final decision is NO" << std::endl; 
            } else { 
                std::cout << "Round " << round + 1 << ": Final decision is TIE" << std::endl; 
            } 

        } else { 
            /// Child process randomly chooses to vote yes or no, displays the vote, and sends it to the judge
            vote = static_cast<int>(generator() % 2); // Randomly choose 0 (no) or 1 (yes) 
            std::cout << "Voter " << voter_id << " votes: " << (vote == 1 ? "Yes" : "No") << std::endl; 
            write(pipes[voter_id][1], &vote, sizeof(vote)); // Send vote to judge
        }
    } 

    if(voter_id != -1){ 
        _exit(0); 
    } 

    for(int i = 0; i < N; i++){ 
        waitpid(-1, nullptr, 0); 
    } 
}