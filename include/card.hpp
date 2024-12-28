#ifndef CARD_HPP
#define CARD_HPP

#include <cstdint>
#include <string>

class Card {
private:
    uint8_t value;  // rank << 2 | suit

public:
    enum class Suit { Hearts, Diamonds, Clubs, Spades };
    enum class Rank { Two, Three, Four, Five, Six, Seven, Eight, Nine, Ten, Jack, Queen, King, Ace };

    Card() : value(0) {}
    Card(Rank rank, Suit suit) : value((static_cast<uint8_t>(rank) << 2) | static_cast<uint8_t>(suit)) {}
    Card(uint8_t packed) : value(packed) {}

    Rank getRank() const { return static_cast<Rank>(value >> 2); }
    Suit getSuit() const { return static_cast<Suit>(value & 0x3); }
    uint8_t getValue() const { return value; }

    std::string toString() const;
};

#endif // CARD_HPP
