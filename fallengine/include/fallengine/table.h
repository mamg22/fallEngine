#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

template<class Card_type>
class Table;

#include <algorithm>
#include <utility>
#include <array>

#include "player.h"

template<class Card_type>
class Table {
public:

    Table()
    {
        m_deck.reserve(40);
        m_table_cards.reserve(12);

    }

    template<class UniformRandomGen>
    void shuffle_deck(UniformRandomGen&& generator);

    // Deletes the given card and returns true, false if doesn't exist
    bool take_card(Card_type& card);

    // Tries to place the card, true if placed, else if it exists on the table
    bool place_card(Card_type& card);

    // Takes a range of cards
    // if the range contains one card, it is laid on the table
    // else if the range contains one card, it takes every card
    // returns 4 if there has been "mesa limpia" (clean table, empty) else 0
    //         ^~~~ because is the bonus of this special move
    template<class FWIterator>
    int play_cards(FWIterator begin, FWIterator end);

    Card_type* last_card_placed() const
    {
        return m_last_card_placed;
    }

    // deal to a player
    template<class Player_type>
    void deal_to(Player_type& player);

    // deal to each player in a range
    template<class FWIterator>
    void deal(FWIterator begin, FWIterator end);

    // Deals the cards for a first round, while counting 1..4 (or 4..1) if
    // count_from_4, if a card repeats, ignore and try the next, if a card's
    // value equals the current count
    int init_round(bool count_from_4);

    // Clears the table, and returns the amount of cards removed
    int clear_table();

    bool is_deck_empty()
    {
        return m_deck.empty();
    }

    void reset_state();

    void reset_last_placed()
    {
        m_last_card_placed = nullptr;
    }

    template <class Iterator>
    void set_deck(Iterator begin, Iterator end);

    const std::vector<Card_type>& get_table_cards() const
    {
        return m_table_cards;
    }

private:
    Card_type* m_last_card_placed = nullptr;
    std::vector<Card_type> m_deck = {};
    std::vector<Card_type> m_table_cards = {};
};

template<class Card_type>
template<class FWIterator>
void Table<Card_type>::deal(FWIterator begin, FWIterator end)
{
    for (; begin != end; begin++){
        deal_to(*begin);
    }
}

template<class Card_type>
template<class UniformRandomGen>
void Table<Card_type>::shuffle_deck(UniformRandomGen&& generator)
{
    std::shuffle(m_deck.begin(), m_deck.end(), std::forward<UniformRandomGen>(generator));
}

template<class Card_type>
template<class FWIterator>
int Table<Card_type>::play_cards(FWIterator begin, FWIterator end)
{
    if (auto cards_played = std::distance(begin, end); cards_played >= 2){
        auto laid_select = begin + 1;
        for (; laid_select != end; laid_select++){
            take_card(*laid_select);
        }
        m_last_card_placed = nullptr;
        if (!m_table_cards.empty()){
            return 0;
        }
        else {
            return 4;
        }
    }
    else if (cards_played == 1){
        m_table_cards.emplace_back(std::move(*begin));
        m_last_card_placed = m_table_cards.end();
        return 0;
    }
    else {
        return 0;
    }
}

template<class Card_type>
template <class Iterator>
void Table<Card_type>::set_deck(Iterator begin, Iterator end)
{
    m_deck.reserve(40);
    std::copy(begin, end, m_deck.begin());
}

template<class Card_type>
template<class Player_type>
void Table<Card_type>::deal_to(Player_type& player)
{
    player.set_cards(m_deck.begin(), m_deck.begin() + 3);
    m_deck.erase(m_deck.begin(), m_deck.begin() + 3);
}


template<class Card_type>
bool Table<Card_type>::take_card(Card_type& card)
{
    if (auto taken_card = std::find_if(m_table_cards.begin(), m_table_cards.end(), [&](Card_type& laid_card){
                                                                                     return is_same_card(card, laid_card);
                                                                                 });
        taken_card != m_table_cards.end())
    {
        m_table_cards.erase(taken_card);
        return true;
    }
    else {
        return false;
    }
}

template<class Card_type>
bool Table<Card_type>::place_card(Card_type& card)
{
    if (auto card_found = std::find_if(m_table_cards.begin(), m_table_cards.end(), [&card](Card_type& laid_card){
                                                                                     return is_same_card(card, laid_card);
                                                                                 });
        card_found == m_table_cards.end())
    {
        m_table_cards.emplace_back(std::move(card));
        return true;
    }
    else {
        return false;
    }

}

template<class Card_type>
void Table<Card_type>::reset_state()
{
    m_last_card_placed = nullptr;
    m_table_cards.clear();
    m_deck.clear();
}

template<class Card_type>
int Table<Card_type>::init_round(bool count_from_4)
{



    // Idea for this to compile: instead of array, use a single int, if counting from 4, then set it to -4, else 1
    // when is required its value, pass it to abs() before
    // Also add an int to limit the increments up to 4 or just compare when it reaches a limit:
    // 0 when counting from 4, 5 if not

    const std::array<int, 4> counts = {0, 0, 0, 0};
    typename std::array<int, 4>::iterator count;
    typename std::array<int, 4>::iterator end;

    if (!count_from_4){
        counts = {1, 2, 3, 4};
    }
    else {
        counts = {4, 3, 2, 1};
    }

    count = counts.begin();
    end = counts.end();

    int match_bonus = 0;

    for (auto& card : m_deck){
        if (count != end){
            if (std::find(m_table_cards.begin(), m_table_cards.end(), card) == m_table_cards.end())
            {
                if (card == *count){
                    match_bonus += *count;
                }
                ++count;
                m_table_cards.emplace_back(std::move(card));
            }
        }
        else {
            break;
        }
    }
    return match_bonus;
}

template<class Card_type>
int Table<Card_type>::clear_table()
{
    int cards_removed = m_table_cards.size();
    m_table_cards.clear();
    return cards_removed;
}


#endif // TABLE_H_INCLUDED
