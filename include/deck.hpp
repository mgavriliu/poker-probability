#ifndef DECK_HPP
#define DECK_HPP

#include <random>
#include <vector>
#include "card.hpp"

class Deck {
 private:
  std::vector<Card> cards;
  size_t currentCard;

 public:
  Deck();  // Constructor initializes a standard 52-card deck

  void shuffle();                            // Shuffles the deck
  Card dealCard();                           // Deals one card from the deck
  std::vector<Card> dealHand(int handSize);  // Add this method
  void reset();                              // Resets the deck to original state

  bool isEmpty() const;           // Checks if deck is empty
  size_t remainingCards() const;  // Returns number of remaining cards
};

#endif  // DECK_HPP
