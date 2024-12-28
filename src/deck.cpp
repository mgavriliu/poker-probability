#include "deck.hpp"
#include <algorithm>
#include <random>
#include <vector>
#include "card.hpp"

Deck::Deck() {
  for (int s = 0; s < 4; ++s) {
    for (int r = 2; r <= 14; ++r) {
      cards.emplace_back(static_cast<Card::Rank>(r), static_cast<Card::Suit>(s));
    }
  }
}

void Deck::shuffle() {
  static std::random_device rd;
  static std::mt19937 g(rd());  // Keep generator static for better performance
  std::shuffle(cards.begin(), cards.end(), g);
}

Card Deck::dealCard() {
  // Deal a card from the top of the deck
  Card dealtCard = cards.back();
  cards.pop_back();
  return dealtCard;
}

std::vector<Card> Deck::dealHand(int handSize) {
  std::vector<Card> hand;
  for (int i = 0; i < handSize; ++i) {
    hand.push_back(dealCard());
  }
  return hand;
}

void Deck::reset() {
  cards.clear();
  for (int s = 0; s < 4; ++s) {
    for (int r = 0; r < 13; ++r) {
      cards.emplace_back(static_cast<Card::Rank>(r), static_cast<Card::Suit>(s));
    }
  }
}