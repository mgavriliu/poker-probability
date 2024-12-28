#ifndef HAND_HPP
#define HAND_HPP

#include <string>
#include <vector>
#include "card.hpp"

enum class HandType {
    RoyalFlush = 0,
    StraightFlush,
    FourOfAKind,
    FullHouse,
    Flush,
    Straight,
    ThreeOfAKind,
    TwoPair,
    OnePair,
    HighCard,
    Count  // Used for iteration
};

class Hand {
 private:
  std::vector<uint8_t> cards;  // Changed from vector<Card> to vector<uint8_t>

 public:
  Hand() = default;
  Hand(const std::vector<Card>& cards);
  Hand(const std::vector<uint8_t>& packed_cards) : cards(packed_cards) {}
  void addCard(const Card& card);
  bool hasThreeOfAKind() const;
  bool hasOnePair() const;
  bool hasTwoPair() const;
  bool hasStraight(bool exclusive = true) const;
  bool hasFlush(bool exclusive = true) const;
  bool hasFullHouse() const;
  bool hasFourOfAKind() const;
  bool hasStraightFlush(bool exclusive = true) const;
  bool hasRoyalFlush(bool exclusive = true) const;
  HandType getHandType() const;
  std::vector<Card> getCards() const;
  void sortHand();
  std::string toString() const;

  static const char* getHandTypeName(HandType type);
};

#endif  // HAND_HPP
