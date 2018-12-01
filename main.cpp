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
#define READY 1
#define NUM_THREADS 3

#define NO_WINNER 1
#define HAS_WINNER 0

#define COUT_EVENTS
//#define DEBUG_DECK
#define DEBUG_PTHREADS
//#define DEBUG_X
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
Deck* deck = new Deck;
int round_status = NO_WINNER;

void do_nothing();
void init_deck(Deck*&);
void shuffle_deck(Deck*&, const int = 1);
void deal_round(Player* player[], const int);
void print_game_status(const Player* const player[], const int);
void print_deck(const Deck*);

void print_hand(const Player*);
void player_draw(Player*& p, Deck*& d);
void player_discard(Player*&, Deck*&);
void push_back_deck(Deck*&, const int);
int pop_front_deck();
void* player_draw(void*);

void *inc_x(void *x_void_ptr);

void log_event(const string);

int main(int argc, char *argv[]) {

   // Check the number of parameters
   if (argc < 1) {
        // Tell the user how to run the program
        std::cout << "Usage: " << argv[0] << " <seed_int> " << std::endl;
        return 1;
   }
   srand(atoi(argv[1]));

   #ifdef DEBUG_PTHREADS
   int x = 0, y = 0;

   //mine
   Player* p1 = new Player;
   Player* p2 = new Player;
   Player* p3 = new Player;
   Player* player[NUM_THREADS] = {p1,p2,p3};

   for (int n = 0; n < NUM_THREADS; n++) {
      player[n]->id = n; //set player ID
   }

   init_deck(deck);
   cout << "Deck has " << deck->cards.size() << endl;

   /* show the initial values of x and y */
   printf("x: %d, y: %d\n", x, y);

   /* this variable is our reference to the second thread */
   pthread_t inc_x_thread;

   //mine
   pthread_t threads[NUM_THREADS];
   pthread_mutex_init(&mutexdeck, NULL);
   pthread_mutex_init(&mutexstatus, NULL);

   /* create a second thread which executes inc_x(&x) */
   if (pthread_create(&inc_x_thread, NULL, inc_x, &x)) {
      fprintf(stderr, "Error creating thread\n");
      return 1;
   }
   /* increment y to 100 in the first thread */
   while(++y < 100);
   printf("y increment finished\n");

   /* wait for the second thread to finish */
   if(pthread_join(inc_x_thread, NULL)) {
      fprintf(stderr, "Error joining thread\n");
      return 2;
   }
   /* show the results - x is now 100 thanks to the second thread */
   printf("x: %d, y: %d\n", x, y);

   //mine
   //deal first hand
   deal_round(player, NUM_THREADS);
   print_game_status(player, NUM_THREADS);

   int rc;

   while (round_status) {
      for (int i = 0; i < NUM_THREADS; i++) {
         printf("[+}In main: creating thread %d\n", i);
         rc = pthread_create(&threads[i], NULL, player_draw, (void *)player[i]);
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
   print_deck(deck);
   for (int i = 0; i < NUM_THREADS; i++) {
      print_hand(player[i]);
   }
   pthread_mutex_destroy(&mutexdeck);
   pthread_exit(NULL);
#endif

#ifdef DEBUG_DECK
   init_deck(deck);
   cout << "# cards in deck: " << deck->cards.size() << endl;
   print_deck(deck);
#endif

#ifdef DEBUG_X

#endif

}

void init_deck(Deck*& d) {
   /**
   * Initialize a 52 card deck and shuffle
   * {Ace	2	3	4	5	6	7	8	9	10	Jack	Queen	King} maps to
   * {1    2   3  4	5	6	7	8	9	10	11 	12    13}
   **/

 #ifdef DEBUG_DECK
   cout << "[+]Pushing ";
#endif
   for (int i = 1; i <= 9; i++) {
      for (int j = 0; j < 1; j++) {
         d->cards.push_back(i);
      }
   }
#ifdef DEBUG_DECK
   cout << endl;
#endif
   shuffle_deck(d,2);
}

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

void print_game_status(const Player* const player[], const int num_players) {
   for (int i = 0; i < num_players; i++) {
      print_hand(player[i]);
   }
   print_deck(deck);
}

void print_deck(const Deck* d) {
   cout <<  "# cards in deck: " << d->cards.size() << ". Deck: "<< endl;

   for (int i = d->cards.size() - 1; i >= 0; i--) {
      cout <<  d->cards.at(i) << " ";
   }
   cout << endl;
}

void print_hand(const Player* p) {
   cout << "PLAYER " << p->id << ": hand ";
   for (int i = p->hand.size() - 1; i >= 0; i--) {
      cout <<  p->hand.at(i) << " ";
   }
   cout << endl;
}


/**
* this function removes top card from Deck and inserts into Player's hand
*
**/
void player_draw(Player*& p, Deck*& d) {
   string e = "";
   e += "PLAYER "; e += p->id; e+= ": draws ";
   cout << "PLAYER " << p->id << ": draws ";

   //critical section
   int card = pop_front_deck();
   e += card;
   cout << card;
   //end critical
   p->hand.push_back(card);
   log_event(e);
   cout << endl;
}

/**
* this function removes random card from Player's and inserts it into the deck
*
**/
void player_discard(Player*& p, Deck*& d) {
   string e = "";
   e += "PLAYER "; e += p->id; e += ": discards ";
   cout << "PLAYER " << p->id << ": discards ";
   int cards_in_hand = p->hand.size();
   int position = rand() % cards_in_hand;
   e += p->hand.at(position);
   cout << p->hand.at(position);

   //critical section
   push_back_deck(d, p->hand.at(position));
   p->hand.erase(p->hand.begin() + position);
   //end critical

   log_event(e);
   cout << endl;
}

/**
* this function modifies deck by pushing a card into  back of deck
*
**/
void push_back_deck(Deck*& d, const int card) {
   d->cards.push_back(card);
}

/**
* this function modifies deck by popping a card into back of deck and returning
* it
**/
int pop_front_deck() {
   int card =  deck->cards.front();
   deck->cards.pop_front();
   return card;
}

/**
* This thread entry function lets players take top card of the deck and check
* for matching hand.
* Precondition: Players have 0 or 1 card in hand. Round is in progress
* Postcondtion: there is 1 fewer card in the deck and each player that draws
* @param player that is drawing from deck
**/
void* player_draw(void* player) {
   Player* p = (Player*)player;
#ifdef FAKEOUT_DELAY
   cout << "[+]Handing " << deck->cards.front() << " to player " << p->id << endl;
   p->hand.push_back(deck->cards.front());
   usleep(rand() % 1000000);
   deck->cards.pop_front();
#else
   pthread_mutex_lock (&mutexdeck);
   cout << "[+]Handing " << deck->cards.front() << " to player " << p->id << endl;
   p->hand.push_back(deck->cards.front());
   usleep(rand() % 1000000); //debug
   deck->cards.pop_front();
   pthread_mutex_unlock (&mutexdeck);

   pthread_mutex_lock (&mutexstatus);
   if (p->hand.at(0) ==  p->hand.at(1)) {
      round_status = HAS_WINNER;
   }
   pthread_mutex_unlock (&mutexstatus);

 #endif
   pthread_exit(NULL);
}

void *do_nothing(void *null) {
   int i;
   i = 0;
   i += 0;
   pthread_exit(NULL);
}

/* this function is run by the second thread */
void *inc_x(void *x_void_ptr) {
   /* increment x to 100 */
   int *x_ptr = (int *)x_void_ptr;
   while(++(*x_ptr) < 100);

   printf("x increment finished\n");

   /* the function must return something - NULL will do */
   return NULL;
}

/**
* this function logs events such as player and dealer actions
**/
void log_event(string e) {
#ifdef COUT_EVENTS
   cout << e << endl;
#else
   ofstream fout;
   ifstream fin;
   fin.open("log.txt");
   fout.open ("log.txt",ios::app); // Append mode
   if (fin.is_open()) {
      fout << e << endl; // Writing data to file
   }
   fin.close();
   fout.close(); // Closing the file
 #endif
}
