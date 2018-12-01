# Pair War
### An operating system multi-threaded process simulation.
In this project, we are going to build “Pair War”, a game that will make use of POSIX threads. It is a simple card game with one dealer, 3 players, and a single deck of cards.

## Gameplay

Overview:

- The game is composed of three rounds.

- At the beginning of each round, the dealer shuffles the cards.

- The dealer deals the first three cards (one to each player) and waits for the round to finish before repeating the same process for the next round.

- A winner in a round is the first player to get a “pair”. In each round, a
different player is given the privilege to start.

Details:
- A deck of cards is the standard 52-card deck, and pairs match by rank (e.g. A and A, or 5 and 5, or J and J, or so on). The suits are ignored.  

- Initially, the dealer shuffles the deck and hands each player a single card in a round robin fashion (say starting from player 1 for round 1). Once the dealer is done handling the cards, the dealer places the
remaining deck of cards on the table and the first round begins.

- Each player (starting from player 1), draws a card from the deck and compares it to the card they have. If they are a pair, the player shows the hand, declares him/herself as a winner and the round ends.

- Otherwise when there is a no match, the player will discard a random card by placing it at the end of the deck of cards on the table and the next player proceeds.

- The players continue until they achieve a pair match.

- Once a winner is declared, the first round ends, the dealer will shuffle the deck and hands a card to each player. In the second round, the second player starts drawing a card from the deck. In the third round, the third player starts drawing a card from the deck.

- The game process finishes after the third round. A single run of the program should have 3 rounds with 3 winners.

## Implementation
- This project is to implemented in C using [POSIX threads](https://computing.llnl.gov/tutorials/pthreads)

- The main function creates a thread for the dealer and 3 threads for the players. Notice that we want to keep the threads synchronized to protect any shared objects.

- When a player wins, he/she needs to inform other players so they can exit. Each thread should print a message when it finishes (eg, “Player 1 wins and exits”, “Player 2 exits”, etc.)

- The main program takes a seed as an argument for the random number generation (which will be used in shuffling and in discarding cards).

## Output

- The dealer and the players will write into a log file each action they talk. The log file should be able to describe exactly what is happening at each step. The log file should look something like this:
PLAYER 1: hand 5
PLAYER 1: draws 7
PLAYER 1: discards 7
PLAYER 1: hand 5
DECK: contents of the deck, separated by spaces (e.g., 1 2 3)

- The final messages for a round should look something like:
PLAYER 2: hand 3
PLAYER 2: draws 3
PLAYER 2: hand 3 3
PLAYER 2: wins
PLAYER 2: exits round
PLAYER 1: exits round
PLAYER 3: exits round
DEALER: shuffle

- The output of the program to the screen (not in the log file) should indicate the hand for each player,
the status (win, lost) and the remaining deck of cards:
HAND card1 card2
WIN yes (or no)
DECK contents of the deck, separated by spaces (e.g., 1 2 3)

- The hand of the winner would show the winning pair and the hands of the other players should show only one card.
