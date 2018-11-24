#ifndef CARD_H_INCLUDED
#define CARD_H_INCLUDED

enum class Suit {
    None,
    Bastos,
    Copas,
    Espadas,
    Oros
};

class Card {
public:
    Card() = default;
    Card(int value, Suit suit = Suit::None)
    : m_value(value), m_suit(suit)
    {
        if (value < 1 || value == 8 || value == 9 || value > 12){
            m_value = -1; // Invalid card value
        }

    }

    int value() const
    {
        return m_value;
    }

    Suit suit() const
    {
        return m_suit;
    }

    // these both return a copy of this card with increased/decreased value
    Card operator+(int increase) const;

    Card operator-(int decrease) const;

private:
    int m_value = 1;
    Suit m_suit = Suit::None;
};


Card Card::operator+(int increase) const
{
    int this_with_inc = this->m_value + increase;
    if (this_with_inc == 8 || this_with_inc == 9){
        this_with_inc += 2; // fixes the value: 8 -> 10 ; 9 -> 11
    }
    else if (this_with_inc <= 0 || this_with_inc >= 12){
        this_with_inc = -1; // Invalid value, should make most comparisons invalid
    }
    return Card(this_with_inc, this->m_suit);
}

Card Card::operator-(int decrease) const
{
    int this_with_dec = this->m_value + decrease;
    if (this_with_dec == 8 || this_with_dec == 9){
        this_with_dec -= 2; // fixes the value: 9 -> 7 ; 8 -> 6
    }
    else if (this_with_dec <= 0 || this_with_dec >= 12){
        this_with_dec = -1; // Invalid value, should make most comparisons invalid
    }
    return Card(this_with_dec, this->m_suit);
}

bool is_same_card(const Card& lhs, const Card& rhs)
{
    return (lhs.value() == rhs.value()) && (lhs.suit() == rhs.suit());
}

// Boilerplate ahead

bool operator==(const Card& lhs, const Card& rhs)
{
    return lhs.value() == rhs.value();
}

bool operator!=(const Card& lhs, const Card& rhs)
{
    return lhs.value() != rhs.value();
}

bool operator< (const Card& lhs, const Card& rhs)
{
    return lhs.value() < rhs.value();
}

bool operator<=(const Card& lhs, const Card& rhs)
{
    return lhs.value() <= rhs.value();
}

bool operator>=(const Card& lhs, const Card& rhs)
{
    return lhs.value() >= rhs.value();
}

bool operator> (const Card& lhs, const Card& rhs)
{
    return lhs.value() > rhs.value();
}

bool operator==(const Card& lhs, int rhs)
{
    return lhs.value() == rhs;
}

bool operator!=(const Card& lhs, int rhs)
{
    return lhs.value() != rhs;
}

bool operator< (const Card& lhs, int rhs)
{
    return lhs.value() < rhs;
}

bool operator<=(const Card& lhs, int rhs)
{
    return lhs.value() <= rhs;
}

bool operator>=(const Card& lhs, int rhs)
{
    return lhs.value() >= rhs;
}

bool operator> (const Card& lhs, int rhs)
{
    return lhs.value() > rhs;
}

bool operator==(int rhs, const Card& lhs)
{
    return rhs == lhs.value();
}

bool operator!=(int rhs, const Card& lhs)
{
    return rhs != lhs.value();
}

bool operator< (int rhs, const Card& lhs)
{
    return rhs < lhs.value();
}

bool operator<=(int rhs, const Card& lhs)
{
    return rhs <= lhs.value();
}

bool operator>=(int rhs, const Card& lhs)
{
    return rhs >= lhs.value();
}

bool operator> (int rhs, const Card& lhs)
{
    return rhs > lhs.value();
}


#endif // CARD_H_INCLUDED
