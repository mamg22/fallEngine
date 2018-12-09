#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED


#include <memory>
#include <vector>
#include <algorithm>
#include <utility>
#include <numeric>
#include <functional>
#include <array>

#include "card.h"
#include "table.h"
#include "player.h"


template<bool Teamed, class Card_type, class Player_type, class Table_type, class Uniform_random_engine>
class Game {
public:
    explicit Game(Uniform_random_engine& random_engine, Combo max_combo_allowed = Combo::Registro)
        : m_random_engine(random_engine), m_max_combo_allowed(max_combo_allowed)
    {
        // fill cards from which table should copy
        for (auto& val : {1, 2, 3, 4, 5, 6, 7, 10, 11, 12}){
            for (auto& suit : {Suit::Bastos, Suit::Copas, Suit::Espadas,
                               Suit::Oros}){
                m_cards.emplace_back(val, suit);
            }
        }
    }


    virtual ~Game() = default;


    using Game_state = std::array<bool, 5>;
    /*
        0 = Caida, // Player picked the card the previous player just placed, bonus points
        1 = Waiting_next_round, // No cards left in neither player's hands or table, new round should begin,
        2 = Table_clear, // Table cleared by player move, bonus points
        3 = Winner_found, // A winner has been found in this player's move, round begin (first deal) or round end (card count)
        4 = Last_deal, // This is the last deal of cards in the deck, Caida is disabled in last deal
     */
    using Player_ptr = typename std::vector<Player_type>::iterator;

    bool init_game();

    template<class... Args>
    void add_player(Args... args);

    bool remove_player(int id);

    Game_state step(bool count_from_4 = false); // count_from_4 only used when Waiting_next_round was returned before

    std::vector<std::reference_wrapper<Player_type>> find_winners() const;

    void order_players(int new_first_id);

    void shuffle_players();

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

    const std::vector<Card_type>& get_table_cards() const
    {
        return m_table.get_table_cards();
    }

private:
    auto rotate_player(Player_type* player, int step);
    void set_best_combo_player();
    void count_cards();
    Player_type* to_player_ptr(auto iter)
    {
        return &(*iter);
    }

    std::vector<Card_type> m_cards = {};
    Table_type m_table;
    std::vector<Player_type> m_players = {};

    // Various player pointers, required for basic game functionality // don't jusge me
    Player_type* m_current_player = nullptr;
    Player_type* m_dealer = nullptr;
    Player_type* m_best_combo_player = nullptr;
    Player_type* m_last_grab_player = nullptr;

    Uniform_random_engine& m_random_engine; // User must supply an random engine

    bool m_is_playing = false;

    int m_id_count = 0; // Counter for the player IDs

    Game_state m_last_game_state = {};

    Combo m_max_combo_allowed = Combo::Registro;
};


//{ Game methods

template<bool Teamed, class Card_type, class Player_type, class Table_type, class Uniform_random_engine>
auto Game<Teamed, Card_type, Player_type, Table_type, Uniform_random_engine>::rotate_player(Player_type* player, int step)
{
    if (step >= 0){
        while (step--){
            if (++player == to_player_ptr(m_players.end())){
                player = to_player_ptr(m_players.begin());
            }
        }
    }
    else {
        while (step++){
            if (--player == to_player_ptr(m_players.rend())){
                player = to_player_ptr(m_players.rbegin());
            }
        }
    }
    return player;
}

template<bool Teamed, class Card_type, class Player_type, class Table_type, class Uniform_random_engine>
void Game<Teamed, Card_type, Player_type, Table_type, Uniform_random_engine>::shuffle_players()
{
    std::shuffle(m_players.begin(), m_players.end(), std::forward<decltype(m_random_engine)>(m_random_engine));
}

template<bool Teamed, class Card_type, class Player_type, class Table_type, class Uniform_random_engine>
void Game<Teamed, Card_type, Player_type, Table_type, Uniform_random_engine>::order_players(int new_first_id)
{
    auto new_first = std::find_if(m_players.begin(), m_players.end(), [&](Player_type& player){
                                                                         return player.id() == new_first_id;
                                                                     });
    if constexpr (Teamed){ // Non teamed
        std::rotate(m_players.begin(), new_first, m_players.end());
    }
    else { // teamed
        swap(*m_players.begin(), *new_first);
        swap(*(m_players.begin() + 2), new_first->get_partner());
    }

}

template<bool Teamed, class Card_type, class Player_type, class Table_type, class Uniform_random_engine>
std::vector<std::reference_wrapper<Player_type>> Game<Teamed, Card_type, Player_type, Table_type, Uniform_random_engine>::find_winners() const
{
    std::vector<std::reference_wrapper<Player_type>> ret{};
    for (auto& player : m_players){
        if (player.is_winner()){
            ret.emplace_back(player);
        }
    }
    return ret;
}

template<bool Teamed, class Card_type, class Player_type, class Table_type, class Uniform_random_engine>
bool Game<Teamed, Card_type, Player_type, Table_type, Uniform_random_engine>::init_game()
{
    constexpr int cards_in_deck_after_deal = 36;
    // only if is valid player count: 2+ players, and the remaining (36) cards are divided evenly between them
    if ((m_players.size() >= 2) && ((cards_in_deck_after_deal / m_players.size()) % 3 == 0)){
        m_is_playing = true;
        m_dealer = to_player_ptr(m_players.end());
        m_current_player = rotate_player(m_dealer, 1);
        return true;
    }
    else {
        return false;
    }
}

template<bool Teamed, class Card_type, class Player_type, class Table_type, class Uniform_random_engine>
template<class... Args>
void Game<Teamed, Card_type, Player_type, Table_type, Uniform_random_engine>::add_player(Args... args)
{
    m_players.emplace_back(m_id_count++, m_table, m_max_combo_allowed, std::forward<Args>(args)...);
}

template<bool Teamed, class Card_type, class Player_type, class Table_type, class Uniform_random_engine>
bool Game<Teamed, Card_type, Player_type, Table_type, Uniform_random_engine>::remove_player(int id)
{
    if (auto player = std::find_if(m_players.begin(), m_players.end(), [&](Player_type& plyr){
                                                                           return id == plyr.id();
                                                                        }); player != m_players.end()){
        m_players.erase(player);
        return true;
    }
    else {
        return false;
    }
}

template<bool Teamed, class Card_type, class Player_type, class Table_type, class Uniform_random_engine>
typename Game<Teamed, Card_type, Player_type, Table_type, Uniform_random_engine>::Game_state
Game<Teamed, Card_type, Player_type, Table_type, Uniform_random_engine>::step(bool count_from_4)
{
    Game_state ret{};
    // New code based on Game_state return
    if (!(m_last_game_state[1])){
        // play cards, branch if caida happened
        if (m_current_player->play_cards(!(m_table.is_deck_empty()))){
            ret[0] = true;
        }
        m_current_player = rotate_player(m_current_player, 1);

        int players_with_cards = 0;
        for (auto& player : m_players){
            if (!player.get_cards().empty()){
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
                set_best_combo_player();

                // in a new deal, last_placed is invalidated
                m_table.reset_last_placed();

                m_best_combo_player->increase_score(m_best_combo_player->get_combo());
            }
            else {
                count_cards();
                ret[1] = true;
            }
        }
        else if (players_with_cards == 1 && m_table.is_deck_empty()){
            // winner_found = next_round(count_from_4);
        }
        if (m_table.get_table_cards().empty()){
            ret[2] = true;
        }
    }
    else {
        for (auto& player : m_players){
            if (player.is_winner()){
                ret[3] = true;
                break; // No longer need to loop
            }
        }

        if (!(ret[3])){
            for (auto& player : m_players){
                player.reset_state();
            }
            m_dealer = rotate_player(m_dealer, 1);

            m_table.set_deck(m_cards.begin(), m_cards.end());

            m_table.shuffle_deck(std::forward<decltype(m_random_engine)>(m_random_engine));

            if (int match_bonus = m_table.init_round(count_from_4); match_bonus > 0){
                m_dealer->increase_score(match_bonus);
            }
            else {
                rotate_player(m_dealer, 1)->increase_score(1);
            }

            m_current_player = rotate_player(m_dealer, 1);

            m_best_combo_player = nullptr;
            m_last_grab_player = nullptr;

        }
        ret[1] = false;
    }
    m_last_game_state = ret;
    return ret;
}

template<bool Teamed, class Card_type, class Player_type, class Table_type, class Uniform_random_engine>
void Game<Teamed, Card_type, Player_type, Table_type, Uniform_random_engine>::count_cards()
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

template<bool Teamed, class Card_type, class Player_type, class Table_type, class Uniform_random_engine>
void Game<Teamed, Card_type, Player_type, Table_type, Uniform_random_engine>::set_best_combo_player()
{
    for (auto& player : m_players){
        if (m_best_combo_player){ // if not null/empty
            if (player.get_combo() == m_best_combo_player->get_combo()){
                // if they both share the same combo, the one with the
                // greatest card gets the points
                for (auto player_hand_it = player.get_cards().rbegin(), best_hand_it = m_best_combo_player->get_cards().rbegin();
                     player_hand_it != player.get_cards().rend(); ++player_hand_it, ++best_hand_it){
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
