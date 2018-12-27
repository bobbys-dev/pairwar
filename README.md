# Pair War
### An operating system multi-threaded process simulation.
A game simulating multiple threads using shared resources (details in Implementation section below). Three players attempt to get a match from a single deck of cards.

## How to run
`make`
`./pairwar.o [int seed]`
A game.log file will produce game events.

## Gameplay

Overview:

- The game is composed of three rounds.

- At the beginning of each round, the dealer shuffles the cards and deals one card to each player.

- A winner in a round is the first player to get a “pair”.

Details:
- A deck of cards is the standard 52-card deck, and pairs match by rank (e.g. A and A, or 5 and 5, or J and J, or so on). The suits are ignored. The cards map A to K : 1 to 13.

- Each player draws a card from the deck and compares it to the card they have. If they are a pair, the player shows the hand, declares him/herself as a winner and the round ends.

- Otherwise when there is a no match, the player will discard a random card by placing it at the end of the deck of cards on the table and the next player proceeds.

- The players continue until they achieve a pair match.

- Once a winner is declared, the round ends, the dealer will shuffle the deck and start another round.

- The game process finishes after the third round. A single run of the program should have 3 rounds with 3 winners.

## Implementation
- This project is to implemented in C using [POSIX threads](https://computing.llnl.gov/tutorials/pthreads)

- The main function creates a thread for the dealer and 3 threads for the players. Notice that we want to keep the threads synchronized to protect any shared objects.

- When a player wins, he/she needs to inform other players so they can exit. Each thread should print a message when it finishes (eg, “Player 1 wins and exits”, “Player 2 exits”, etc.)

- The main program takes a seed as an argument for the random number generation (which will be used in shuffling and in discarding cards).

## Output

- The dealer and the players will write into a log file each action they take.

- The output of the program to the screen (not in the log file) should indicate the hand for each player,
the status (win, lost) and the remaining deck of cards.
