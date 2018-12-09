#ifndef HAND_H_INCLUDED
#define HAND_H_INCLUDED

#include <string>
#include <vector>
#include <memory>
#include <algorithm>

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

// TODO: Do testing, a lot

template<class Card_type>
class Hand {
public:

    Hand() = default;
    Hand(Combo max_combo_allowed)
    : m_max_combo_allowed(max_combo_allowed)
    {
        m_cards.reserve(3);
    }

    // Takes a iterator pair to the cards in the deck to be set here, an calls
    // analize_hand at the end
    template<class FWIterator>
    void set_cards(FWIterator begin, FWIterator end);

    // Erases one card from the member cards at index
    bool erase(Card& card);

    // Returns a referente to the member cards to provide better access

    // This overload exists because otherwise, it wouldn't compile
    // because of some error at "util.h", JUST FOR THAT (for now)
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

    int highest_card() const
    {
        return m_highest_card;
    }

    void reset()
    {
        m_cards.clear();
        m_combo = Combo::None;
        m_highest_card = 1;
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
    // combo, highest_cards and combo_name
    void analize_hand();

    std::vector<Card_type> m_cards{};
    Combo m_combo = Combo::None;
    int m_highest_card = 1;
    std::string m_combo_name = "";
    const Combo m_max_combo_allowed = Combo::Registro;
};

template<class Card_type>
template<class FWIterator>
void Hand<Card_type>::set_cards(FWIterator begin, FWIterator end)
{
    // Push each card in range to member cards
    std::move(begin, end, m_cards.begin());
    analize_hand();
}

template<class Card_type>
bool Hand<Card_type>::erase(Card& card)
{
    if (auto item = std::find_if(m_cards.begin(), m_cards.end(), [&](Card& other_card){
                                                                     return is_same_card(card, other_card);
                                                                 }
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
    // Sort the cards, so it's easier to analize its combo
    std::sort(m_cards.begin(), m_cards.end(), std::less<>{});

    Card& card_1 = m_cards[0];
    Card& card_2 = m_cards[1];
    Card& card_3 = m_cards[2];

    // Because is sorted, this will always be the highest

    m_highest_card = m_cards[2].value();

    // Check if extra combos are allowed
    if (m_max_combo_allowed == Combo::Casa_grande) {
        // Check if has a one as lowest card, else don't check
        if (card_1 == 1){
            // Check for Casa Grande (1, 12, 12)
            if (card_2 == 12 && card_3 == 12){
                m_combo = Combo::Casa_grande;
                m_combo_name = "Casa Grande";
                return;
            }
            // Check for Casa Chica (1, 11, 11)
            if (card_2 == 11 && card_3 == 11){
                m_combo = Combo::Casa_chica;
                m_combo_name = "Casa Chica";
                return;
            }
            // Check for Registrico (1, 10, 11)
            if (card_2 == 10 && card_3 == 11){
                m_combo = Combo::Registrico;
                m_combo_name = "Registrico";
                return;
            }
            // Check for Maguaro (1, 10, 12)
            if (card_2 == 10 && card_3 == 12){
                m_combo = Combo::Maguaro;
                m_combo_name = "Maguaro";
                return;
            }
        }
    }

    if (m_max_combo_allowed == Combo::Registro) {
        // Check for Registro (1, 11, 12)
        if (card_1 == 1 && card_2 == 11 && card_3 == 12){
            m_combo = Combo::Registro;
            m_combo_name = "Registro";
            return;
        }

        // Check for Vigia ( X, X, {X+1}) or ( {X-1}, X, X)
        else if (  (  (card_1 == card_2)
                   && (card_3 == card_1 + 1))
                || (  (card_2 == card_3)
                   && (card_1 == card_2 - 1)) )
        {
            m_combo = Combo::Vigia;
            m_combo_name = "Vigia";
            return;
        }

        // Check for Partrulla ( X, {X+1}, {X+2} )
        else if (  (card_1 + 1 == card_2)
                && (card_1 + 2 == card_3) )
        {
            m_combo = Combo::Patrulla;
            m_combo_name = "Patrulla";
            return;
        }

        // Check for Trivilin ( X, X, X )
        else if (card_1 == card_2 && card_2 == card_3)
        {
            m_combo = Combo::Trivilin;
            m_combo_name = "Trivilin";
            return;
        }

        // Check for Ronda... ( X, X, *) or ( *, X, X)
        else if (card_1 == card_2 || card_2 == card_3) {
            // The following checks only check the value of the second card
            // because if a pair exists the middle card will always be one of it

            switch(card_2.value()){
            case 12:
                // Ronda_12 (pair of 12)
                m_combo = Combo::Ronda_12;
                m_combo_name = "Ronda (12)";
                break;

            case 11:
                // Ronda_11 (pair of 11)
                m_combo = Combo::Ronda_11;
                m_combo_name = "Ronda (11)";
                break;

            case 10:
                // Ronda_10 (pair of 10)
                m_combo = Combo::Ronda_10;
                m_combo_name = "Ronda (10)";
                break;

            default:
                // Ronda (pair of [1-7])
                m_combo = Combo::Ronda;
                m_combo_name = "Ronda";
                break;
            }
            return;
        }

    // If none of the above conditions are satisfied, then hand has no combo
    // or no combos allowed (boring)
    m_combo = Combo::None;
    m_combo_name = "";
    }
}


#endif // HAND_H_INCLUDED
