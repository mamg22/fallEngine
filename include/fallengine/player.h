#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include <string>
#include <deque>
#include <memory>
#include <functional>
#include <algorithm>
#include <exception>
#include <utility>

#include "hand.h"
#include "table.h"

class Null_partner_exception : public std::exception {
public:
    virtual const char * what() const noexcept {
        return "Used a player's partner when it has not been set (null partner), partner is requiered for "
               "Teamed Players, not required for Non-teamed Players";
    }
};

template<class Card_type, class This_classname>
class Base_player {
public:
    Base_player(bool teamed, int id, Table<Card_type>& current_table, Combo max_combo = Combo::Registro)
        : m_current_table(current_table), m_hand(max_combo), m_id(id), m_teamed(teamed)
    {
    }
    Base_player(bool teamed, int id, Table<Card_type>& current_table, std::array<bool, 12> allowed_combos)
        : m_current_table(current_table), m_hand(allowed_combos), m_id(id), m_teamed(teamed) 
    {
    }

    std::vector<Card_type>& get_cards()
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
    Card_type& next_selection() const // First card selected
    {
        return m_selection.front();
    }
    Card_type& last_selection() const // Last card selected
    {
        return m_selection.back();
    }
    const std::deque<std::reference_wrapper<Card_type>>& get_selection() const
    {
        return m_selection;
    }

    // Plays the current selection, true if (special move) "caida" has happened,
    // else false
    // with_caida shall be false if the deck is empty, hence end of round
    // Plays the current selection, returns:
    // First: true if special move "caida" happened, else false
    // Second: false if the played cards weren't valid (e.g. no cards selected,
    //         or selected a own card that is on the table, but didn't select the other, else true
    std::pair<bool, bool> play_cards(bool caida_enabled = true);
    
    void reset_hand();
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

    void set_partner(This_classname& partner)
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

    const std::string& get_combo_name() const
    {
        return m_hand.combo_name();
    }

    int id() const
    {
        return m_id;
    }
    
    This_classname& get_partner()
    {
        if (m_teamed){
            if (m_partner){
                return *m_partner;
            }
            else {
                throw Null_partner_exception();
            }
        }
        else {
            return *static_cast<This_classname*>(this);
        }
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
    bool m_teamed;

    This_classname* m_partner = nullptr;
};


template<class Card_type, class This_classname>
template<class FWIterator>
void Base_player<Card_type, This_classname>::set_cards(FWIterator begin, FWIterator end)
{
    m_hand.set_cards(begin, end);
}

template<class Card_type, class This_classname>
bool Base_player<Card_type, This_classname>::select(Card_type& card)
{
    Card dummy(-1);
    Card_type* prev_select = &dummy;
    if (m_selection.size() > 0){
        prev_select = &last_selection();
    }

    auto hand_begin = m_hand.get_cards().begin();
    auto hand_end = m_hand.get_cards().end();

    bool in_hand = (std::find_if(hand_begin, hand_end, [&](Card& other){
                                                           return is_same_card(card, other);
                                                       }) != hand_end);
    // Only add if:
    // first card selected AND own hand only
    // second card selected AND same value as last selected AND not in hand
    // 3rd, 4th, ..., card selected AND same as last selected but increased by 1
    // AND not in own hand

    if (  (  (m_selection.size() >= 2) && (card == *prev_select + 1) && (!in_hand))
       || (  (m_selection.size() == 1) && (card == *prev_select) && (!in_hand))
       || (  (m_selection.size() == 0) && (in_hand)) )
    {
        m_selection.emplace_back(card);
        return true;
    }
    else {
        return false;
    }
}

template<class Card_type, class This_classname>
void Base_player<Card_type, This_classname>::reset_hand()
{
    m_selection.clear();
    m_hand.reset();
}
template<class Card_type, class This_classname>
void Base_player<Card_type, This_classname>::reset_state()
{
    m_cards_accumulated = 0;
    reset_hand();
}

template<class Card_type, class This_classname>
void Base_player<Card_type, This_classname>::count_cards(int base)
{
    if (m_teamed){
        increase_score(std::max((m_cards_accumulated + m_partner->get_cards_accumulated()) - base, 0));
    }
    else {
        increase_score(std::max(m_cards_accumulated - base, 0));
    }
}

template<class Card_type, class This_classname>
std::pair<bool, bool> Base_player<Card_type, This_classname>::play_cards(bool caida_enabled)
{
    auto select_beg = m_selection.begin();
    auto select_end = m_selection.end();

    std::vector<Card> selection_cp(select_beg, select_end);

    auto& table_cards = m_current_table.get().get_table_cards();

    bool has_caida = false;

    if (caida_enabled && (m_current_table.get().last_card_placed()) && (*(m_current_table.get().last_card_placed()) == m_selection.front().get()))
    {
        has_caida = true;
        if ((1 <= m_selection.front().get()) && (m_selection.front().get() <= 7)){
            increase_score(1);
        }
        else {
            // just for cards in range 10..12
            increase_score(m_selection.front().get().value() - 8);
        }
    }
    // Don't play if just selected one (own) card, but its already on the table
    if (m_selection.size() == 1 && std::find(table_cards.begin(), table_cards.end(), *select_beg) != table_cards.end()){
        //CLEAR SELECT
        return {false, false};
    }

    if (select_beg == select_end){
        //CLEAR
        return {false, false};
    }

    if (m_current_table.get().play_cards(selection_cp.begin(), selection_cp.end())){
        increase_score(4);
    }

    if (m_selection.size() >= 2)
    {
        m_cards_accumulated += m_selection.size(); // No. of cards collected
    }

    // This checks for a "caida", where if the previous player placed a card
    // and this player takes it with his own card of the same value, then it
    // gives extra points
    m_hand.erase(select_beg->get());
    m_selection.clear();
    return {has_caida, true};
}

template<class Card_type, class This_classname>
void Base_player<Card_type, This_classname>::increase_score(int increment)
{
    if (!m_teamed){
        m_score += increment;
    }
    else {
        m_score += increment;
        if (m_partner){
            m_partner->increase_score_this_only(increment);
        }
        else {
            // Not initialized partner
            throw Null_partner_exception();
        }
    }
}

template<class Card_type>
class Player : public Base_player<Card_type, Player<Card_type>> {
public:
    using Base_player<Card_type, Player>::Base_player;
};

#endif // PLAYER_H_INCLUDED
