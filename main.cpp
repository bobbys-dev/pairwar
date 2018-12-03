#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <deque>
#include <algorithm>
#include <fstream>
#include <stdio.h>

////////////////////////////////////////////////////////////////
// type of events
#define SHUFFLE_N_TIMES 4
#define NUM_THREADS 3
#define NUM_ROUNDS 3

#define NO_WINNER 1
#define HAS_WINNER 0

//#define COUT_EVENTS
///#define FAKEOUT_DELAY

using namespace std;

struct Deck
{
   deque<int> cards;
};

struct Player
{
   int id;
   deque<int> hand;
};

pthread_mutex_t mutexdeck;
pthread_mutex_t mutexstatus;
pthread_cond_t status_cv;
Deck* deck = new Deck;
int round_status = NO_WINNER;

void init_deck(Deck*&);
void shuffle_deck(Deck*&, const int = 1);
void deal_round(Player* player[], const int);
void print_game_status(const Player* const player[], const int);
void print_deck(const Deck*, const int print_option = 0);
void print_hand(const Player* p, const int print_option = 0);
void player_draw(Player*& p, Deck*& d);
void player_discard(Player*&, Deck*&);
void push_back_deck(Deck*&, const int);
int pop_front_deck();
void* player_makes_move(void*);
void exit_round(Player*& p);
void log_event(const string);

int main(int argc, char *argv[]) {

   // Check the number of parameters
   if (argc < 2) {
        // Tell the user how to run the program
        std::cout << "Usage: " << argv[0] << " <seed_int> " << std::endl;
        return 1;
   }
   srand(atoi(argv[1]));

   Player* p1 = new Player;
   Player* p2 = new Player;
   Player* p3 = new Player;
   Player* player[NUM_THREADS] = {p1,p2,p3};

   for (int n = 0; n < NUM_THREADS; n++) {
      player[n]->id = n; //set player ID
   }

   pthread_t threads[NUM_THREADS];
   pthread_mutex_init(&mutexdeck, NULL);
   pthread_mutex_init(&mutexstatus, NULL);

   for (int r = 0; r < NUM_ROUNDS; r++) {
      init_deck(deck);
      //deal first hand
      cout << "[+]ROUND " << r << endl;
      deal_round(player, NUM_THREADS);
      print_game_status(player, NUM_THREADS);

      int rc;
      //play a round
      while (round_status) {
         for (int i = 0; i < NUM_THREADS; i++) {
            rc = pthread_create(&threads[i], NULL, player_makes_move, (void *)player[i]);
            if (rc) {
               printf("[-]ERROR; return code from pthread_create() is %d\n", rc);
               exit(-1);
            }
         }
         /* wait for the other threads to finish */
         for (int i = 0; i < NUM_THREADS; i++) {
            if (pthread_join(threads[i], NULL)) {
               fprintf(stderr, "[-]Error joining thread\n");
               exit(2);
            }
         }
      }
      print_game_status(player, NUM_THREADS);

      //make players exit
      for (int n = 0; n < NUM_THREADS; n++) {
         string e = "PLAYER ";
         e += to_string(player[n]->id);
         e += ": exits round";
         exit_round(player[n]);
         log_event(e);
      }
      round_status = NO_WINNER;
   }
   pthread_mutex_destroy(&mutexdeck);
   pthread_mutex_destroy(&mutexstatus);
   pthread_exit(NULL);
}

/**
* This method initializes a deck and calls a shuffle
* Precondition: Deck is not initiated
* Postcondtion: Deck is initiated and shuffled
* @param times is number of times to sweep through deck
**/
void init_deck(Deck*& d) {
   /**
   * Initialize a 52 card deck and shuffle
   * {Ace	2	3	4	5	6	7	8	9	10	Jack	Queen	King} maps to
   * {1    2   3  4	5	6	7	8	9	10	11 	12    13}
   **/
   delete d;
   d = new Deck;
   for (int i = 1; i <= 13; i++) {
      for (int j = 0; j < 4; j++) {
         d->cards.push_back(i);
      }
   }
   shuffle_deck(d,SHUFFLE_N_TIMES);
}

/**
* This helper method shuffles the deck by swapping a random card with next card
* Precondition: Deck as non-zero cards
* Postcondtion: Deck is non-sorted
* @param times is number of times to sweep through deck
**/
void shuffle_deck(Deck*& d, const int times) {
   int sz = d->cards.size();
   int position;
   log_event("DEALER: shuffling");
   for (int j = 0; j < times; j++) {
      for (int i = 0 ; i < sz; i++) {
         position = rand() % sz;
#ifdef DEBUG_DECK
         cout << "[+]swapping positions " << i << " & " << position << endl;
#endif
         iter_swap(d->cards.begin() + i, d->cards.begin() + position);
      }
   }
}

/**
* This method deals a round to players. The top card of the deck is given to
* players in order starting from the first.
* Precondition: Players have 0 or 1 card in hand. Players are not drawing from
* the deck
* Postcondtion: there are num_players fewer cards in the deck and each player has
* one more card in hand.
* @param player is array of references to Players
* @param num_players is the number of Players
**/
void deal_round(Player* player[], const int num_players) {
   log_event("DEALER: dealing round");
   for (int i = 0; i < num_players; i++) {
      player[i]->hand.push_back(pop_front_deck());
   }
}

/**
* Print each Player's hands and cards in deck
**/
void print_game_status(const Player* const player[], const int num_players) {
   cout << "******Game snapshot******" << endl;
   for (int i = 0; i < num_players; i++) {
      print_hand(player[i], 1);
   }
   print_deck(deck,1);
   cout << "*************************" << endl;
}

/**
* Helper method to print cards in deck
**/
void print_deck(const Deck* d, const int print_option) {
   string e = "# cards in deck: ";
   e += to_string(d->cards.size()); e += ". Deck:\n";

   for (int i = d->cards.size() - 1; i >= 0; i--) {
      e +=  to_string(d->cards.at(i)); e+= " ";
   }
   log_event(e);

   if (print_option) {
      cout << e << endl;
   }
}

/**
* Print each Player's hands and cards in deck
**/
void print_hand(const Player* p, const int print_option) {
   string e = "PLAYER "; e += to_string(p->id); e+= ": hand ";

   for (int i = p->hand.size() - 1; i >= 0; i--) {
      e +=  to_string(p->hand.at(i)); e += " ";
   }
   log_event(e);
   if (print_option) {
      cout << e << endl;
   }
}


/**
* Helper method removes top card from Deck and inserts into Player's hand
* Precondtion: Deck mutex is locked by thread (player)
* Postcondtion: Deck has 1 less card if there was more than 0.
**/
void player_draw(Player*& p, Deck*& d) {
   string e = "";
   e += "PLAYER "; e += to_string(p->id); e+= ": draws ";
   if (d->cards.size() > 0) {
      int card = pop_front_deck();
      e += to_string(card);
      p->hand.push_back(card);
   } else {
      e += "nothing from empty deck." ;
   }
   log_event(e);
}

/**
* Helper method removes top card from Deck and inserts into Player's hand
* Precondtion: Deck mutex is locked by thread (player).  Player has >0 cards
* Postcondtion: Deck has 1 more card. Player has 1 less card
**/
void player_discard(Player*& p, Deck*& d) {
   string e = "";
   e += "PLAYER "; e += to_string(p->id); e += ": discards ";

   int cards_in_hand = p->hand.size();
   int position = rand() % cards_in_hand;

   e += to_string(p->hand.at(position));

   push_back_deck(d, p->hand.at(position));
   p->hand.erase(p->hand.begin() + position);

   log_event(e);
}

/**
* Helper method pushes a card
* Precondtion: Deck mutex is locked by thread (player)
* Postcondtion: Deck has 1 more card.
**/
void push_back_deck(Deck*& d, const int card) {
   d->cards.push_back(card);
}

/**
* Helper function modifies deck by popping a card into back of deck and returning
* it
* Precondition: Deck is non-empty.
* Postcondition: Deck has 1 less card.
**/
int pop_front_deck() {
   int card =  deck->cards.front();
   deck->cards.pop_front();
   return card;
}

/**
* This thread entry function lets players take top card of the deck and check
* for matching hand.
* Precondition: Players are in round and have have 0, 1, or 2 non-winning cards.
* Postcondition: Players has 2 cards, and at least 1 is different.
* @param player that is drawing from deck
**/
void* player_makes_move(void* player) {
   Player* p = (Player*)player;

   //player already has 2 cards
   pthread_mutex_lock(&mutexdeck);
   if (p->hand.size() >= 2) {
      player_discard(p, deck);
      print_hand(p);
   }
#ifdef FAKEOUT_DELAY
   usleep(rand() % 1000000);
#endif
   player_draw(p, deck);
   print_hand(p);
   pthread_mutex_lock (&mutexstatus);
   if (p->hand.at(0) ==  p->hand.at(1)) {
      pthread_cond_signal(&status_cv);
      round_status = HAS_WINNER;
      string e = "PLAYER ";
      e += to_string(p->id); e+= " WINS!";
      log_event(e);
   }
   pthread_mutex_unlock (&mutexstatus);
   pthread_mutex_unlock (&mutexdeck);

   pthread_exit(NULL);
}


/**
* Discard Player's cards.
**/
void exit_round(Player*& p) {
   int cards_in_hand = p->hand.size();
   for (int i = cards_in_hand - 1; i >= 0; i--) {
      p->hand.erase(p->hand.begin() + i);
   }
}

/**
* Logs events such as player and dealer actions into 'log.txt' file
* @param e event to log
**/
void log_event(string e) {
#ifdef COUT_EVENTS
   cout << e << endl;
#else
   ofstream fout;
   ifstream fin;
   fin.open("game.log");
   fout.open ("game.log",ios::app); // Append mode
   if (fin.is_open()) {
      fout << e << endl; // Writing data to file
   }
   fin.close();
   fout.close(); // Closing the file
 #endif
}
