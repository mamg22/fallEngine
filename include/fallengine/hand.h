#ifndef HAND_H_INCLUDED
#define HAND_H_INCLUDED

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <array>

#include "card.h"

enum class Combo {
    None,
    Ronda,
    Ronda_10,
    Ronda_11,
    Ronda_12,
    Trivilin,
    Patrulla,
    Vigia,
    Registro,
    Maguaro,
    Registrico,
    Casa_chica,
    Casa_grande,
};

template<class Card_type>
class Hand {
public:



    Hand(Combo max_combo_allowed = Combo::Registro)
    {
        // Allow combos below and including the specified
        std::fill_n(m_allowed_combos.begin(), static_cast<int>(max_combo_allowed), true);
    }

    explicit Hand(std::array<bool, 12> allowed_combos)
    : m_allowed_combos(allowed_combos)
    {
    }

    Hand<Card_type>& operator=(Hand<Card_type>&& other) = default;
    Hand(Hand<Card_type>& other) = default;
    Hand<Card_type>& operator=(Hand<Card_type>& other) = default;
    Hand(Hand<Card_type>&& other) = default;
    // Takes a iterator pair to the cards in the deck to be set here, an calls
    // analize_hand at the end
    template<class FWIterator>
    void set_cards(FWIterator begin, FWIterator end);

    // Erases one card from the member cards
    bool erase(Card& card);

    // Returns a referente to the member cards to provide better access

    std::vector<Card_type>& get_cards()
    {
        return m_cards;
    }

    const std::vector<Card_type>& get_cards() const
    {
        return m_cards;
    }

    const Card_type& get_card(int index) const
    {
        return m_cards[index];
    }

    void reset()
    {
        m_cards.clear();
        m_combo = Combo::None;
        m_combo_name = "";
    }

    Combo combo() const
    {
        return m_combo;
    }

    const std::string& combo_name() const
    {
        return m_combo_name;
    }
private:
    // Private method that is for especific use by set_cards, sets members
    // combo and combo_name
    void analize_hand();
    bool is_allowed(Combo combo);

    std::vector<Card_type> m_cards = {};
    Combo m_combo = Combo::None;
    std::string m_combo_name = "";
    std::array<bool, 12 /*number of combos*/> m_allowed_combos;
};

template<class Card_type>
template<class FWIterator>
void Hand<Card_type>::set_cards(FWIterator begin, FWIterator end)
{
    for (;begin != end; begin++){
        m_cards.push_back(*begin);
    }
    analize_hand();
}

template<class Card_type>
bool Hand<Card_type>::is_allowed(Combo combo)
{
    return m_allowed_combos[static_cast<int>(combo) - 1];
}

template<class Card_type>
bool Hand<Card_type>::erase(Card& card)
{
    if (auto item = std::find_if(m_cards.begin(), m_cards.end(), [&](Card& other_card){
                                                                     return is_same_card(card, other_card);});
        item != m_cards.end())
    {
        m_cards.erase(item);
        return true;
    }
    else {
        return false;
    }
}

template<class Card_type>
void Hand<Card_type>::analize_hand()
{
    // Sort the cards, so it's easier to analize its combo and display
    std::sort(m_cards.begin(), m_cards.end(), std::less<>{});

    Card& card_1 = m_cards[0];
    Card& card_2 = m_cards[1];
    Card& card_3 = m_cards[2];

    // Check for Casa Grande (1, 12, 12)
    if (is_allowed(Combo::Casa_grande) && card_1 == 1 && card_2 == 12 && card_3 == 12){
        m_combo = Combo::Casa_grande;
        m_combo_name = "Casa Grande";
        return;
    }
    // Check for Casa Chica (1, 11, 11)
    else if (is_allowed(Combo::Casa_chica) && card_1 == 1 && card_2 == 11 && card_3 == 11){
        m_combo = Combo::Casa_chica;
        m_combo_name = "Casa Chica";
        return;
    }
    // Check for Registrico (1, 10, 11)
    else if (is_allowed(Combo::Registrico) && card_1 == 1 && card_2 == 10 && card_3 == 11){
        m_combo = Combo::Registrico;
        m_combo_name = "Registrico";
        return;
    }
    // Check for Maguaro (1, 10, 12)
    else if (is_allowed(Combo::Maguaro) && card_1 == 1 && card_2 == 10 && card_3 == 12){
        m_combo = Combo::Maguaro;
        m_combo_name = "Maguaro";
        return;
    }
    // Check for Registro (1, 11, 12)
    else if (is_allowed(Combo::Registro) && card_1 == 1 && card_2 == 11 && card_3 == 12){
        m_combo = Combo::Registro;
        m_combo_name = "Registro";
        return;
    }
    // Check for Vigia ( X, X, {X+1}) or ( X, {X+1}, {X+1})
    else if (is_allowed(Combo::Vigia) && (  (card_1 == card_2 && card_3 == card_1 + 1)
                                         || (card_2 == card_3 && card_1 + 1 == card_2) )){
        m_combo = Combo::Vigia;
        m_combo_name = "Vigia";
        return;
    }
    // Check for Partrulla ( X, {X+1}, {X+2} )
    else if (is_allowed(Combo::Patrulla) && (card_1 + 1 == card_2 && card_1 + 2 == card_3))
    {
        m_combo = Combo::Patrulla;
        m_combo_name = "Patrulla";
        return;
    }
    // Check for Trivilin ( X, X, X )
    else if (is_allowed(Combo::Trivilin) && card_1 == card_2 && card_2 == card_3)
    {
        m_combo = Combo::Trivilin;
        m_combo_name = "Trivilin";
        return;
    }

    // Check for Ronda... ( X, X, *) or ( *, X, X)
    else if (card_1 == card_2 || card_2 == card_3) {
        // The following checks only check the value of the second card
        // because if a pair exists the middle card will always be one of it

        if (is_allowed(Combo::Ronda_12) && card_2.value() == 12){
            // Ronda_12 (pair of 12)
            m_combo = Combo::Ronda_12;
        }
        else if (is_allowed(Combo::Ronda_11) && card_2.value() == 11){
            // Ronda_11 (pair of 11)
            m_combo = Combo::Ronda_11;
        }
        else if (is_allowed(Combo::Ronda_10) && card_2.value() == 10){
            // Ronda_10 (pair of 10)
            m_combo = Combo::Ronda_10;
        }
        else if (is_allowed(Combo::Ronda)){
            // Ronda (pair of [1-7])
            m_combo = Combo::Ronda;
        }
        m_combo_name = "Ronda";
        return;
    }

    // If none of the above conditions are satisfied, then hand has no combo or no combos allowed (boring)
    m_combo = Combo::None;
    m_combo_name = "";
    return;
}


#endif // HAND_H_INCLUDED
