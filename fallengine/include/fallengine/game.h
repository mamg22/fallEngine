#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED


#include <memory>
#include <vector>
#include <algorithm>
#include <utility>
#include <numeric>
#include <functional>
#include <array>
#include <type_traits>

#include "card.h"
#include "table.h"
#include "player.h"

template<bool Teamed, class Card_type, class Player_type, class Table_type>
class Game {
public:
    Game(Combo max_combo_allowed = Combo::Registro)
        : m_max_combo_allowed(max_combo_allowed)
    {
        // fill cards from which table should copy
        m_cards.reserve(40);
        for (auto& val : {1, 2, 3, 4, 5, 6, 7, 10, 11, 12}){
            for (auto& suit : {Suit::Bastos, Suit::Copas, Suit::Espadas,
                               Suit::Oros}){
                m_cards.emplace_back(val, suit);
            }
        }
        m_players.reserve(4);
    }


    virtual ~Game() = default;

    enum class State {
        Caida, // Player picked the card the previous player just placed, bonus points
        Waiting_for_init, // No cards left in either player's hands nor table, new round should begin, call step(bool)
        Table_clear, // Table cleared by player move, bonus points
        Winner_found, // A winner has been found in this player's move, round begin (first deal) or round end (card count)
        Last_deal, // This is the last deal of cards in the deck, Caida is disabled in last deal
    };

    // The game state should be indexed via enum class State members;
    using Game_state = std::array<bool, 5>;

    using Player_ptr = typename std::vector<Player_type>::iterator;

    bool init_game();

    template<class... Args>
    void add_player(Args... args);

    Game_state step(bool count_from_4);

    void count_cards();

    std::vector<std::reference_wrapper<Player_type>> find_winners() const;

    void order_players(Player_ptr first_player);

    template <class UniformRandomGen>
    void shuffle_players(UniformRandomGen&& generator);

    template <class UniformRandomGen>
    void shuffle_deck(UniformRandomGen&& generator);

    Player_type& current_player() const
    {
        return *m_current_player;
    }

    Player_type& dealer() const
    {
        return *m_dealer;
    }

    Player_type& player_with_best_combo() const
    {
        return *m_best_combo_player;
    }

    const std::vector<Player_type>& get_players() const
    {
        return m_players;
    }

private:
    auto rotate_player(Player_ptr player, int step) const;
    void set_best_combo_player();

    std::vector<Card_type> m_cards = {};
    Table_type m_table;
    std::vector<Player_type> m_players = {};

    // Various player pointers (Player vector iterators really), required for basic game functionality
    Player_ptr m_current_player = nullptr;
    Player_ptr m_dealer = nullptr;
    Player_ptr m_best_combo_player = nullptr;
    Player_ptr m_last_grab_player = nullptr;

    bool m_is_playing = false;

    Game_state m_last_game_state = {};

    int m_id_count = 0; // Counter for the player IDs

    Combo m_max_combo_allowed = Combo::Registro;
};

//{ Game methods

template<bool Teamed, class Card_type, class Player_type, class Table_type>
auto Game<Teamed, Card_type, Player_type, Table_type>::rotate_player(Player_ptr player, int step) const
{
    if (step >= 0){
        while (step--){
            if (++player == m_players.end()){
                player = m_players.begin();
            }
        }
    }
    else {
        while (step++){
            if (--player == m_players.rend()){
                player = m_players.rbegin();
            }
        }
    }
    return player;
}

template<bool Teamed, class Card_type, class Player_type, class Table_type>
template <class UniformRandomGen>
void Game<Teamed, Card_type, Player_type, Table_type>::shuffle_players(UniformRandomGen&& generator)
{
    std::shuffle(m_players.begin(), m_players.end(), std::forward<UniformRandomGen>(generator));
}

template<bool Teamed, class Card_type, class Player_type, class Table_type>
void Game<Teamed, Card_type, Player_type, Table_type>::order_players(Player_ptr new_first)
{
    if constexpr (Teamed){ // Non teamed
        std::rotate(m_players.begin(), new_first, m_players.end());
    }
    else { // teamed
        std::iter_swap(m_players.begin(), new_first);
        std::iter_swap(m_players.begin() + 2, &(new_first->get_partner()));
    }

}

template<bool Teamed, class Card_type, class Player_type, class Table_type>
template <class UniformRandomGen>
void Game<Teamed, Card_type, Player_type, Table_type>::shuffle_deck(UniformRandomGen&& generator)
{
    m_table.shuffle_deck(std::forward<UniformRandomGen>(generator));
}

template<bool Teamed, class Card_type, class Player_type, class Table_type>
std::vector<std::reference_wrapper<Player_type>> Game<Teamed, Card_type, Player_type, Table_type>::find_winners() const
{
    std::vector<std::reference_wrapper<Player_type>> ret{};
    for (auto& player : m_players){
        if (player.is_winner()){
            ret.emplace_back(player);
        }
    }
    return ret;
}

template<bool Teamed, class Card_type, class Player_type, class Table_type>
bool Game<Teamed, Card_type, Player_type, Table_type>::init_game()
{
    constexpr int cards_in_deck_after_deal = 36;
    // only if is valid player count: 2+ players, and the remaining (36) cards are divided evenly between them
    if ((m_players.size() >= 2) && ((cards_in_deck_after_deal / m_players.size()) % 3 == 0)){
        m_table.set_deck(m_cards.begin(), m_cards.end());
        // poner de alguna forma el next_round
        // ^~ nope, init_game valida el inicio, despues solo se llama a step
        m_is_playing = true;
        m_last_game_state[State::Waiting_for_init] = true;
        return true;
    }
    else {
        return false;
    }
}

template<bool Teamed, class Card_type, class Player_type, class Table_type>
template<class... Args>
void Game<Teamed, Card_type, Player_type, Table_type>::add_player(Args... args)
{
    m_players.emplace_back(m_id_count++, m_table, m_max_combo_allowed, std::forward<Args...>(args...));
}

template<bool Teamed, class Card_type, class Player_type, class Table_type>
typename Game<Teamed, Card_type, Player_type, Table_type>::Game_state Game<Teamed, Card_type, Player_type, Table_type>::step(bool count_from_4)
{
    Game_state ret{};
    constexpr int cards_per_deck = 40;
    // New code based on Game_state return
    if (!(m_last_game_state[State::Waiting_for_init])){
        // play cards, branch if caida happened
        if (m_current_player->play_cards(!(m_table.is_deck_empty()))){
            ret[State::Caida] = true;
        }
        m_current_player = rotate_player(m_current_player, 1);

        int players_with_cards = 0;
        for (auto& player : m_players){
            if (player.get_cards().empty()){
                ++players_with_cards;
            }
        }
        if (players_with_cards == 0){
            if (!m_table.is_deck_empty()){
                for (auto& player : m_players){
                player.reset_state();
                }
                m_table.deal(m_players.begin(), m_players.end());

                // set the player with the greatest combo, only it will get the bonus
                // from his combo
                for (auto& player : m_players){
                    set_best_combo_player(player);
                }

                // in a new deal, last_placed is invalidated
                m_table.reset_last_placed();

                m_best_combo_player->increase_score(m_best_combo_player->get_combo());
            }
            else {
                count_cards();
                ret[State::Waiting_for_init] = true;
            }
        }
        else if (players_with_cards == 1 && m_table.is_deck_empty()){
            // winner_found = next_round(count_from_4);
        }
        if (m_table.get_laid_cards().empty()){
            ret[State::Table_clear] = true;
        }
    }
    else {
        for (auto& player : m_players){
            if (player.is_winner()){
                ret[State::Winner_found] = true;
            }
        }

        if (!(ret[State::Winner_found])){
            for (auto& player : m_players){
                player.reset_state();
            }
            m_dealer = rotate_player(m_dealer, 1);

            // crear baraja y repartir con first deal
            m_table.set_deck(m_cards.begin(), m_cards.end());

            if (int match_bonus = m_table.init_round(count_from_4); match_bonus > 0){
                m_dealer->increase_score(match_bonus);
            }
            else {
                rotate_player(m_dealer, 1)->increase_score(1);
            }

            m_current_player = rotate_player(m_dealer, 1);

            m_best_combo_player = nullptr;
            m_last_grab_player = nullptr;

            m_table.deal(m_players.begin(), m_players.end());

        }
    }
    return ret;
}

template<bool Teamed, class Card_type, class Player_type, class Table_type>
void Game<Teamed, Card_type, Player_type, Table_type>::count_cards()
{
    constexpr int cards_per_deck = 40;
    const bool three_players = (m_players.size() == 3);

    for (auto& player : m_players){
        // if the player isn't the dealer and the game has other than 3 players then do the usual sum
        if (!(three_players && (player.id() == m_dealer->id()))){
            player.count_cards(cards_per_deck / m_players.size());
        }
        else { // Count to 14, it is the dealer in a round of 3, so it should count 14 rather than 13, because rules
            player.count_cards(14);
        }
    }
}

template<bool Teamed, class Card_type, class Player_type, class Table_type>
void Game<Teamed, Card_type, Player_type, Table_type>::set_best_combo_player()
{
    for (auto& player : m_players){
        if (m_best_combo_player){ // if not null/empty
            if (player.get_combo() == m_best_combo_player->get_combo()){
                // if they both share the same combo, the one with the
                // greatest card gets the points
                for (auto player_hand_it = player.get_hand().rbegin(), best_hand_it = m_best_combo_player->get_hand().rbegin();
                     player_hand_it != player.get_hand().rend(); ++player_hand_it, ++best_hand_it){
                    if (player_hand_it->value() > best_hand_it->value()){
                        m_best_combo_player = &player;
                        break;
                    }
                }
            }
            else if (player.get_combo() > m_best_combo_player->get_combo()){
                m_best_combo_player = &player;
            }
        }
        else {
            m_best_combo_player = &player;
        }
    }

}

//} End of Game methods



#endif // GAME_H_INCLUDED
