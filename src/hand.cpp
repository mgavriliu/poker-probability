#include "hand.hpp"
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include "card.hpp"

Hand::Hand(const std::vector<Card>& cards) {
  this->cards.reserve(cards.size());
  for (const Card& card : cards) {
    this->cards.push_back(card.getValue());
  }
}

bool Hand::hasThreeOfAKind() const {
  int ranks[13] = {0};
  for (uint8_t card : cards) {
    if (++ranks[card >> 2] >= 3) return true;
  }
  return false;
}

bool Hand::hasOnePair() const {
  int ranks[13] = {0};
  int pairs = 0;
  for (uint8_t card : cards) {
    if (++ranks[card >> 2] == 2) pairs++;
    if (ranks[card >> 2] > 2) return false;  // Exclude higher combinations
  }
  return pairs == 1;
}

bool Hand::hasTwoPair() const {
  int ranks[13] = {0};
  int pairs = 0;
  for (uint8_t card : cards) {
    if (++ranks[card >> 2] == 2) pairs++;
    if (ranks[card >> 2] > 2) return false;  // Exclude higher combinations
  }
  return pairs == 2;
}

bool Hand::hasStraight(bool exclusive) const {
  int ranks[13] = {0};
  for (uint8_t card : cards) {
    ranks[card >> 2] = 1;
  }

  // Check for Ace-low straight (A,2,3,4,5)
  if (ranks[12] && ranks[0] && ranks[1] && ranks[2] && ranks[3]) {
    return !exclusive || !hasFlush(false);
  }

  // Find min and max ranks for regular straight check
  int min_rank = 12, max_rank = 0;
  for (int i = 0; i < 13; i++) {
    if (ranks[i]) {
      min_rank = std::min(min_rank, i);
      max_rank = std::max(max_rank, i);
    }
  }

  // Check for regular straight
  if (max_rank - min_rank == 4) {
    for (int i = min_rank; i <= max_rank; i++) {
      if (!ranks[i]) return false;
    }
    return !exclusive || !hasFlush(false);
  }
  return false;
}

bool Hand::hasFlush(bool exclusive) const {
  uint8_t suit = cards[0] & 0x3;
  for (uint8_t card : cards) {
    if ((card & 0x3) != suit) return false;
  }
  return !exclusive || !hasStraight(false);
}

bool Hand::hasFullHouse() const {
  int ranks[13] = {0};

  // Count all ranks first
  for (uint8_t card : cards) {
    ranks[card >> 2]++;
  }

  // Check for exactly three of one rank and exactly two of another
  bool hasThree = false;
  bool hasTwo = false;
  for (int count : ranks) {
    if (count == 3) {
      if (hasThree) return false;  // More than one three of a kind
      hasThree = true;
    } else if (count == 2) {
      if (hasTwo) return false;  // More than one pair
      hasTwo = true;
    } else if (count > 3) {
      return false;  // Four of a kind
    }
  }

  return hasThree && hasTwo;
}

bool Hand::hasFourOfAKind() const {
  int ranks[13] = {0};
  for (uint8_t card : cards) {
    if (++ranks[card >> 2] == 4) return true;
  }
  return false;
}

bool Hand::hasStraightFlush(bool exclusive) const { return hasFlush(false) && hasStraight(false); }

bool Hand::hasRoyalFlush(bool exclusive) const {
  if (!hasFlush(false)) return false;

  int ranks[13] = {0};
  for (uint8_t card : cards) {
    ranks[card >> 2] = 1;
  }
  return ranks[8] && ranks[9] && ranks[10] && ranks[11] && ranks[12];
}

HandType Hand::getHandType() const {
  if (hasRoyalFlush()) return HandType::RoyalFlush;
  if (hasStraightFlush()) return HandType::StraightFlush;
  if (hasFourOfAKind()) return HandType::FourOfAKind;
  if (hasFullHouse()) return HandType::FullHouse;
  if (hasFlush()) return HandType::Flush;
  if (hasStraight()) return HandType::Straight;
  if (hasThreeOfAKind()) return HandType::ThreeOfAKind;
  if (hasTwoPair()) return HandType::TwoPair;
  if (hasOnePair()) return HandType::OnePair;
  return HandType::HighCard;
}

const char* Hand::getHandTypeName(HandType type) {
  switch (type) {
    case HandType::HighCard: return "High Card";
    case HandType::OnePair: return "One Pair";
    case HandType::TwoPair: return "Two Pair";
    case HandType::ThreeOfAKind: return "Three of a Kind";
    case HandType::Straight: return "Straight";
    case HandType::Flush: return "Flush";
    case HandType::FullHouse: return "Full House";
    case HandType::FourOfAKind: return "Four of a Kind";
    case HandType::StraightFlush: return "Straight Flush";
    case HandType::RoyalFlush: return "Royal Flush";
    default: return "Unknown";
  }
}

std::vector<Card> Hand::getCards() const {
  std::vector<Card> result;
  result.reserve(cards.size());
  for (uint8_t packed : cards) {
    result.emplace_back(packed);
  }
  return result;
}

void Hand::sortHand() {
  std::sort(cards.begin(), cards.end(), [](uint8_t a, uint8_t b) { return (a >> 2) < (b >> 2); });
}

void Hand::addCard(const Card& card) { cards.push_back(card.getValue()); }

std::string Hand::toString() const {
  std::string result;
  for (size_t i = 0; i < cards.size(); ++i) {
    if (i > 0) result += ", ";
    result += Card(cards[i]).toString();
  }
  return result;
}