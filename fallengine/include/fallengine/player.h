#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include <string>
#include <deque>
#include <memory>
#include <functional>
#include <algorithm>
#include <exception>

template<bool Teamed, class Card_type>
class Player;

#include "hand.h"
#include "table.h"

class Null_partner_exception : public std::exception {
public:
    virtual const char * what() const noexcept {
        return "Used a player's partner when it has not been set (null partner), partner is requiered for"
               "Teamed Players, not required for Non-teamed Players";
    }
};

template<bool Teamed, class Card_type>
class Player {
public:
    Player(int id, Table<Card_type>& current_table, Combo max_combo = Combo::Registro)
        : m_current_table(current_table), m_hand(max_combo), m_id(id)
    {
    }
    Player(Player<Teamed, Card_type>&& other) = default;
    Player& operator=(Player<Teamed, Card_type>&& other) = default;
    Player(Player<Teamed, Card_type>& other) = default;
    Player& operator=(Player<Teamed, Card_type>& other) = default;

    const std::vector<Card_type>& get_cards() const
    {
        return m_hand.get_cards();
    }

    // Select card, true if can select, else false
    bool select(Card_type& card);

    void pop_next() // Pops the next card selected
    {
        m_selection.pop_front();
    }
    void undo_select() // Pops the last card selected (undo)
    {
        m_selection.pop_back();
    }
    void reset_selection() // Clear selection
    {
        m_selection.clear();
    }
    const Card_type& next_selection() const // First card selected
    {
        return *m_selection.front();
    }
    const Card_type& last_selection() const // Last card selected
    {
        return *m_selection.back();
    }
    const std::deque<std::reference_wrapper<const Card_type>>& get_selection() const
    {
        return m_selection;
    }

    // Plays the current selection, true if (special move) "caida" has happened,
    // else false
    // with_caida shall be false if the deck is empty, hence end of round
    bool play_cards(bool caida_enabled = true);

    void reset_state();

    void count_cards(int base);

    void increase_score(int increment);
    void increase_score(Combo combo)
    {
        increase_score(static_cast<int>(combo));
    }


    // Forwards to (member hand).set_cards, accepts a iterator pair of the deck
    // from which the cards will be extracted
    template<class FWIterator>
    void set_cards(FWIterator begin, FWIterator end);

    void set_partner(Player<Teamed, Card_type>& partner)
    {
        m_partner = &partner;
    }

    bool is_winner() const
    {
        return m_score >= 24;
    }

    int get_score() const
    {
        return m_score;
    }

    int get_cards_accumulated() const
    {
        return m_cards_accumulated;
    }

    Combo get_combo() const
    {
        return m_hand.combo();
    }

    std::string get_combo_name() const
    {
        return m_hand.combo_name();
    }

    int get_highest_card() const
    {
        return m_hand.highest_card();
    }

    int id() const
    {
        return m_id;
    }

    Player<Teamed, Card_type>& get_partner()
    {
        if constexpr (Teamed){
            if (m_partner){
                return *m_partner;
            }
            else {
                throw Null_partner_exception();
            }
        }
        else {
            return *this;
        }
    }

    friend void swap(Player<Teamed, Card_type>& lhs, Player<Teamed, Card_type>& rhs)
    {
        std::swap(lhs.m_score, rhs.m_score);
        std::swap(lhs.m_cards_accumulated, rhs.m_cards_accumulated);
        std::swap(lhs.m_id, rhs.m_id);
        std::swap(lhs.m_partner, rhs.m_partner);
    }
protected:
    void increase_score_this_only(int increase)
    {
        m_score += increase;
    }
    void increase_score_this_only(Combo combo)
    {
        m_score += static_cast<int>(combo);
    }

private:
    std::reference_wrapper<Table<Card_type>> m_current_table;
    std::deque<std::reference_wrapper<Card_type>> m_selection;
    Hand<Card_type> m_hand;

    int m_score = 0;
    int m_cards_accumulated = 0;
    int m_id = 0;

    Player<Teamed, Card_type>* m_partner = nullptr;
};

template<bool Teamed, class Card_type>
void swap(Player<Teamed, Card_type>&& a, Player<Teamed, Card_type>&& b)
{
    a.swap(b);
}

template<bool Teamed, class Card_type>
template<class FWIterator>
void Player<Teamed, Card_type>::set_cards(FWIterator begin, FWIterator end)
{
    m_hand.set_cards(begin, end);
}

template<bool Teamed, class Card_type>
bool Player<Teamed, Card_type>::select(Card_type& card)
{
    auto& prev_select = next_selection();

    auto hand_begin = m_hand.get_cards().begin();
    auto hand_end = m_hand.get_cards().end();

    // bool in_hand = (find_same_card(card, hand_begin, hand_end)== hand_end);
    bool in_hand = (std::find_if(hand_begin, hand_end, is_same_card) != hand_end);
    // Only add if:
    // first card selected AND own hand only
    // second card selected AND same value as last selected AND not in hand
    // 3rd, 4th, ..., card selected AND same as last selected but increased by 1
    // AND not in own hand

    if (  (  (m_selection.size() >= 2) && (card == prev_select + 1) && (!in_hand))
       || (  (m_selection.size() == 1) && (card == prev_select) && (!in_hand))
       || (  (m_selection.size() == 0) && (in_hand)) )
    {
        m_selection.emplace_back(card);
        return true;
    }
    else {
        return false;
    }
}

template<bool Teamed, class Card_type>
void Player<Teamed, Card_type>::reset_state()
{
    m_selection.clear();
    m_hand.reset();
    m_score = 0;
    m_cards_accumulated = 0;
}


template<bool Teamed, class Card_type>
void Player<Teamed, Card_type>::count_cards(int base)
{
    increase_score(std::max(m_cards_accumulated - base, 0));
}

template<bool Teamed, class Card_type>
bool Player<Teamed, Card_type>::play_cards(bool caida_enabled)
{
    auto select_beg = m_selection.begin();
    auto select_end = m_selection.end();

    if (select_beg == select_end){
        return false;
    }

    increase_score(m_current_table.get().play_cards(select_beg, select_end));

    if (m_selection.size() >= 2)
    {
        m_cards_accumulated += m_selection.size(); // No. of cards collected
    }

    bool has_caida = false;

    // This checks for a "caida", where if the previous player placed a card
    // and this player takes it with his own card of the same value, then it
    // gives extra points
    if (caida_enabled && (m_current_table.get().last_card_placed()) && (*(m_current_table.get().last_card_placed()) == select_beg->get()))
    {
        has_caida = true;
        if ((1 <= select_beg->get()) && (select_beg->get() >= 7)){
            increase_score(1);
        }
        else {
            // just for cards in range 10..12
            increase_score(select_beg->get().value() - 8);
        }
    }
    m_hand.erase(select_beg->get());
    m_selection.clear();
    return has_caida;
}

template<bool Teamed, class Card_type>
void Player<Teamed, Card_type>::increase_score(int increment)
{
    if constexpr(!Teamed){
        m_score += increment;
    }
    else {
        m_score += increment;
        if (m_partner){
            m_partner->increase_score_this_only(increment);
        }
        else {
            // THROW AN EXCEPTION BECAUSE THE USER OF THIS LIBRARY WON'T INIT THE PARTNER
            throw Null_partner_exception();
        }
    }
}



#endif // PLAYER_H_INCLUDED
