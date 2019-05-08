#ifndef FALLENGINE_FALLENG_GAME_HPP_INCLUDED
#define FALLENGINE_FALLENG_GAME_HPP_INCLUDED


#include <memory>
#include <vector>
#include <algorithm>
#include <utility>
#include <numeric>
#include <functional>
#include <array>
#include <iostream>

#include "card.hpp"
#include "table.hpp"
#include "player.hpp"

namespace falleng {

    class Player_not_found_exception : public std::exception {
    public:
        virtual const char * what() const noexcept 
        {
            return "The requested player with the given ID in not in the current players, please check the available player IDs";
        }
    };

    class Op_not_valid_currently_exception : public std::exception {
    public:
        Op_not_valid_currently_exception(const char * func_name, bool game_running_allowed) 
        {
            text = std::string("Called function ") + func_name + " while the game was " + (game_running_allowed ? "not " : "") + "running";
        }

        virtual const char * what() const noexcept {
            return text.c_str();
        }
    private:
        std::string text;
    };

    template<class Card_type, class Player_type, class Table_type>
    class Game {
    public:
        Game(bool teamed, Combo max_combo_allowed)
            : m_is_teamed(teamed)
        {
            fill_cards();
            std::fill_n(m_allowed_combos.begin(), static_cast<int>(max_combo_allowed), true);
        }
        Game(bool teamed, std::array<bool, 12>& allowed_combos)
            : m_is_teamed(teamed), m_allowed_combos(allowed_combos)
        {
            fill_cards();
        }

        struct State {
            bool caida = false; // Player picked the card the previous player just placed, bonus points
            bool waiting_next_round = false; // No cards left in neither player's hands or table, new round should begin,
            bool table_clear = false;  // Table cleared by player move, bonus points
            bool winner_found = false;  // A winner has been found in this player's move, round begin (first deal) or round end (card count)
            bool last_deal = false; // This is the last deal of cards in the deck, Caida is disabled in last deal
        };

        using Player_ptr = typename std::vector<Player_type>::iterator;
        using Reverse_player_ptr = typename std::vector<Player_type>::reverse_iterator;

        bool init_game();

        template<class... Args>
        void add_player(Args... args);

        bool remove_player(int id);

        template<class Uniform_random_engine>
        State step(Uniform_random_engine&& random_engine, bool count_from_4 = false); // count_from_4 only used when Waiting_next_round was returned before

        std::vector<std::reference_wrapper<Player_type>> find_winners();

        void order_players(int new_first_id);

        template<class Uniform_random_engine>
        void shuffle_players(Uniform_random_engine&& random_engine);

        Player_type& current_player()
        {
            if (m_current_player){
                return *m_current_player;
            }
            else {
                throw Player_not_found_exception();
            }
        }

        Player_type& dealer() const
        {
            if (m_dealer){
                return *m_dealer;
            }
            else {
                throw Player_not_found_exception();
            }   
        }

        Player_type& best_combo_p() const
        {
            if (m_best_combo_player){
                return *m_best_combo_player;
            }
            else {
                throw Player_not_found_exception();
            }   
        }

        std::vector<Player_type>& get_players()
        {
            return m_players;
        }

        Player_type& get_player(int id)
        {
            if (auto player = std::find_if(m_players.begin(), m_players.end(), [&](Player_type& player){
                                                                                   return player.id() == id;
                                                                               }); player  != m_players.end()){
                return *player;
            }
            else {
                throw Player_not_found_exception();
            }
        }

        std::vector<Card_type>& get_table_cards()
        {
            return m_table.get_table_cards();
        }
        
        Table_type& get_table()
        {
            return m_table;
        }

        bool is_playing()
        {
            return m_is_playing;
        }   

        State get_last_state()
        {
            return m_last_game_state;
        }
    private:
        auto rotate_player(Player_type* player, int step);
        void set_best_combo_player();
        void count_cards();
        void fill_cards();
        Player_type* to_player_ptr(Player_ptr iter)
        {
            return &(*iter);
        }
        Player_type* to_player_ptr(Reverse_player_ptr r_iter)
        {
            return &(*r_iter);
        }

        std::vector<Card_type> m_cards;
        Table_type m_table;
        std::vector<Player_type> m_players;

        // Various player pointers, required for basic game functionality 
        Player_type* m_current_player = nullptr;
        Player_type* m_dealer = nullptr;
        Player_type* m_best_combo_player = nullptr;
        Player_type* m_last_grab_player = nullptr;

        bool m_is_playing = false;
        bool m_is_teamed;
        bool m_just_inited = false;

        int m_id_count = 0; // Counter for the player IDs

        State m_last_game_state = {};

        std::array<bool, 12> m_allowed_combos;

    };


    // Game methods

    template<class Card_type, class Player_type, class Table_type>
    auto Game<Card_type, Player_type, Table_type>::rotate_player(Player_type* player, int step)
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

    template<class Card_type, class Player_type, class Table_type>
    template<class Uniform_random_engine>
    void Game<Card_type, Player_type, Table_type>::shuffle_players(Uniform_random_engine&& random_engine)
    {
        if (m_is_playing){throw Op_not_valid_currently_exception("shuffle_players", false);}
        std::shuffle(m_players.begin(), m_players.end(), std::forward<Uniform_random_engine>(random_engine));
    }

    template<class Card_type, class Player_type, class Table_type>
    void Game<Card_type, Player_type, Table_type>::order_players(int new_first_id)
    {
        if (m_is_playing){throw Op_not_valid_currently_exception("order_players", false);}
        auto new_first = std::find_if(m_players.begin(), m_players.end(), [&](Player_type& player){
                                                                             return player.id() == new_first_id;
                                                                         });
        if (m_is_teamed){ // Non teamed
            std::rotate(m_players.begin(), new_first, m_players.end());
        }
        else { // teamed
            std::swap(*m_players.begin(), *new_first);
            std::swap(*(m_players.begin() + 2), new_first->get_partner());
        }

    }

    template<class Card_type, class Player_type, class Table_type>
    std::vector<std::reference_wrapper<Player_type>> Game<Card_type, Player_type, Table_type>::find_winners() 
    {
        if (!m_is_playing){throw Op_not_valid_currently_exception("find_winners", true);}
        std::vector<std::reference_wrapper<Player_type>> ret{};
        for (auto& player : m_players){
            if (player.is_winner()){
                ret.emplace_back(player);
            }
        }
        return ret;
    }

    template<class Card_type, class Player_type, class Table_type>
    bool Game<Card_type, Player_type, Table_type>::init_game()
    {
        if (m_is_playing){throw Op_not_valid_currently_exception("init_game", false);}
        constexpr int cards_in_deck_after_deal = 36;
        // only if is valid player count: 2+ players, and the remaining (36) cards are divided evenly between them
        // or in two Teams of two players each
        if (  (m_is_teamed && m_players.size() == 4)
           || (!m_is_teamed && m_players.size() >= 2 && ((cards_in_deck_after_deal / m_players.size()) % 3 == 0))){
            if (m_is_teamed){
                for (auto& player: m_players){
                    try {
                        player.get_partner();
                    }
                    catch (Null_partner_exception& e){
                        // Cannot init the game if a team player has no partner
                        return false;
                    }
                order_players(m_players[0].id());
                }
            }
            m_is_playing = true;
            m_just_inited = true;
            m_dealer = to_player_ptr(m_players.begin());
            m_current_player = rotate_player(m_dealer, 1);
            m_last_game_state.waiting_next_round = true;
            m_best_combo_player = to_player_ptr(m_players.begin());
            return true;
        }
        else {
            return false;
        }
    }

    template<class Card_type, class Player_type, class Table_type>
    template<class... Args>
    void Game<Card_type, Player_type, Table_type>::add_player(Args... args)
    {
        if (m_is_playing){throw Op_not_valid_currently_exception("add_player", false);}
        m_players.push_back(Player_type(m_is_teamed, m_id_count++, std::forward<Args>(args)...));
    }

    template<class Card_type, class Player_type, class Table_type>
    bool Game<Card_type, Player_type, Table_type>::remove_player(int id)
    {
        if (m_is_playing){throw Op_not_valid_currently_exception("remove_player", false);}
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

    template<class Card_type, class Player_type, class Table_type>
    template<class Uniform_random_engine>
    typename Game<Card_type, Player_type, Table_type>::State
    Game<Card_type, Player_type, Table_type>::step(Uniform_random_engine&& random_engine, bool count_from_4)
    {
        if (!m_is_playing){throw Op_not_valid_currently_exception("step", true);}
        State ret;
        if (!(m_last_game_state.waiting_next_round || m_last_game_state.winner_found)){
            // play cards, branch if caida happened
            auto[caida, valid] = m_current_player->play_cards(m_table,!(m_table.is_deck_empty()));

            ret.caida = caida;
            if (valid){
                m_current_player = rotate_player(m_current_player, 1);
            }


            int players_with_cards = 0;
            for (auto& player : m_players){
                if (!player.get_cards().empty()){
                    ++players_with_cards;
                }
            }
            if (players_with_cards == 0){
                m_best_combo_player->increase_score(m_best_combo_player->get_combo());
                if (!m_table.is_deck_empty()){
                    for (auto& player : m_players){
                        player.reset_hand();
                    }
                    m_table.deal(m_allowed_combos, m_players.begin(), m_players.end());
                    

                    for (auto& player: m_players){
                        auto& cards = player.get_cards();
                        if (cards[0] == 12 && cards[1] == 12 && cards[2] == 12){
                            // If a player, after a deal, has three 12's, wins
                            player.increase_score(24);
                            ret.waiting_next_round = true;
                            break;
                        }
                    }
                    // set the player with the greatest combo, only it will get the bonus
                    // from his combo
                    set_best_combo_player();
                    // in a new deal, last_placed is invalidated
                    m_table.reset_last_placed();

                }
                else {
                    count_cards();
                    ret.waiting_next_round = true;
                }
            }
            if (m_table.get_table_cards().empty()){
                ret.table_clear = true;
            }
            for (auto& player : m_players){
                if (player.is_winner()){
                    ret.winner_found = true;
                    break; // No longer need to loop
                }
            }
        }
        else {
            for (auto& player : m_players){
                if (player.is_winner()){
                    ret.winner_found = true;
                    break; // No longer need to loop
                }
            }

            if (!(ret.winner_found)){
                for (auto& player : m_players){
                    player.reset_state();
                }
                if (!m_just_inited){
                    m_dealer = rotate_player(m_dealer, 1);
                    m_current_player = rotate_player(m_dealer, 1);
                }
                else {
                    m_just_inited = false;
                }

                m_table.set_deck(m_cards.begin(), m_cards.end());


                m_table.shuffle_deck(std::forward<Uniform_random_engine>(random_engine));

                if (int match_bonus = m_table.init_round(count_from_4); match_bonus > 0){
                    m_dealer->increase_score(match_bonus);
                }
                else {
                    rotate_player(m_dealer, 1)->increase_score(1);
                }


                m_table.deal(m_allowed_combos, m_players.begin(), m_players.end());

                for (auto& player: m_players){
                    auto& cards = player.get_cards();
                    if (cards[0] == 12 && cards[1] == 12 && cards[2] == 12){
                        // If a player, after a deal, has three 12's, wins
                        player.increase_score(24);
                        break;
                    }
                }

                set_best_combo_player();
                m_last_grab_player = nullptr;
                
                
                for (auto& player: m_players){
                    if (player.is_winner()){
                        ret.winner_found = true;
                        break;
                    }
                }

                if (!ret.winner_found){
                    ret.waiting_next_round = false;
                }
                else {
                    ret.waiting_next_round = true;
                }
                
            }

        }
        m_last_game_state = ret;
        return ret;
    }

    template<class Card_type, class Player_type, class Table_type>
    void Game<Card_type, Player_type, Table_type>::count_cards()
    {
        
        constexpr int cards_per_deck = 40;
        const bool three_players = (m_players.size() == 3);
        int players_counted = 0;
        
        for (auto& player : m_players){
            // if the player isn't the dealer and the game has other than 3 players then do the usual sum
            if (m_is_teamed){
                if (players_counted == 2) return;
                player.count_cards(20);
                ++players_counted;
            }
            else if (!(three_players && (player.id() == m_dealer->id()))){
                player.count_cards(cards_per_deck / m_players.size());
            }
            else { // Count to 14, it is the dealer in a round of 3, so it should count 14 rather than 13, because rules
                player.count_cards(14);
            }
        }
    }

    template<class Card_type, class Player_type, class Table_type>
    void Game<Card_type, Player_type, Table_type>::set_best_combo_player()
    {
        int player_count = m_players.size();
        for (auto current = rotate_player(m_dealer, -1);player_count > 0; current = rotate_player(current, -1)){
            auto& player = *current;
            
            if (!m_best_combo_player){
                m_best_combo_player = &player;
                --player_count;
                continue;
            }
            auto& bc_player = *m_best_combo_player;
            if (player.get_combo() > bc_player.get_combo()){
                m_best_combo_player = current;
            }
            else if (player.get_combo() == bc_player.get_combo()){
                if (player.get_combo() == Combo::Ronda && player.get_cards()[1] > bc_player.get_cards()[1]){
                    // Ronda includes values from range [1-7], so it can be hard to determine which one is the best
                    m_best_combo_player = current;
                    --player_count;
                    continue;
                }

                for (auto player_hand_it = player.get_cards().rbegin(), best_hand_it = m_best_combo_player->get_cards().rbegin();
                     player_hand_it != player.get_cards().rend(); ++player_hand_it, ++best_hand_it){
                    if (player_hand_it->value() > best_hand_it->value()){
                        m_best_combo_player = &player;
                        break;
                    }
                }
            }
            --player_count;
        }
    }

    template<class Card_type, class Player_type, class Table_type>
    void Game<Card_type, Player_type, Table_type>::fill_cards()
    {
        // fill cards from which table should copy
        for (auto& val : {1, 2, 3, 4, 5, 6, 7, 10, 11, 12}){
            for (auto& suit : {Suit::Bastos, Suit::Copas, Suit::Espadas, Suit::Oros}){
                m_cards.emplace_back(val, suit);
            }
        }
    }
    // End of Game methods

} // namespace falleng

#endif // FALLENGINE_FALLENG_GAME_HPP_INCLUDED
