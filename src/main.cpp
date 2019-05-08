#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <sstream>
#include <array>

#include "../include/fallengine/fallengine.hpp"
#include "../include/fespar/fespar.hpp"

/* TODO: Find a way to record a game and replay it given a file, because repeating it is annoying (see notes)
 */

template<class Game_type>
void report_round(int& round, Game_type& game)
{
    if (!game.is_playing()) {return;};
    try {game.current_player();} catch (falleng::Player_not_found_exception& e){return;}
    std::cout << "Round: " << round++ << "\n===============================\n" <<
    "CP: " << game.current_player().id() << " DP: " << game.dealer().id() << " BC: " << game.best_combo_p().id() << "\n\n";
}


template<class Game_type>
void print_state(Game_type& game)
{
    if (!game.is_playing()) {return;};
    try {game.current_player();} catch (falleng::Player_not_found_exception& e){return;}
    std::cout << "Table: ";
    for (auto& card : game.get_table_cards()){
        std::cout << card.get_color() << card.value() << ' ' << static_cast<char>(static_cast<int>(card.suit())+0x40) << " \t" ;
    }
    std::cout << "\n\n";
    for (auto& player : game.get_players()){
        std::cout << "Player: " << player.id() << "\t S: " << player.get_score() << "\t CCnt: " << player.get_cards_accumulated()
         << "\tName: " << player.get_name() << "\t Combo:" << player.get_combo_name() << '\n';
        for (auto& card : player.get_cards()){
            std::cout << card.value() << ' ' << static_cast<char>(static_cast<int>(card.suit())+0x40) << " \t";
        }

        std::cout << "\n\n";
    }
    std::cout << "CP-Sel: ";
    for (auto& card: game.current_player().get_selection()){
        std::cout << card.get().value() << ' ' << static_cast<char>(static_cast<int>(card.get().suit())+0x40) << " \t";
    }
    std::cout << "\n\n";
}

std::vector<std::string> split(const std::string& s, char delimiter)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}

bool y_n_ask(std::string prompt)
{
    char choice = 'n';
    std::cout << prompt << " [y/n]: ";
    std::cin >> choice;
    return choice == 'y' || choice == 'Y';
}

class Neo_card : public falleng::Card {
public:
    using falleng::Card::Card;
    std::string& get_color()
    {
        return m_color;
    }
private:
    std::string m_color = "\033[32m";
};

template<class Card_type>
class Neo_table : public falleng::Table<Card_type> {
public:
    using falleng::Table<Card_type>::Table;
    void set_color(std::string& color)
    {
        m_color = color;
    }
    std::string& get_color()
    {
        return m_color;
    }
private:
    std::string m_color;
};

template <class Card_type>
class Neo_player : public falleng::Base_player<Card_type, Neo_player<Card_type>> {
public:
    Neo_player(bool teamed, int id, std::string name)
        : falleng::Base_player<Card_type, Neo_player>(teamed, id), m_name(name)
    {}

    const std::string& get_name()
    {
        return m_name;
    }
private:
    std::string m_name;
};


template<class Card_type, class Player_type, class Table_type>
class Neo_game : public falleng::Game<Card_type, Player_type, Table_type> {
public:
    Neo_game(bool teamed, falleng::Combo max_combo_allowed, const std::string& color)
        : falleng::Game<Card_type, Player_type, Table_type>(teamed, max_combo_allowed)
        , m_color(color)
    {}
    
    Neo_game(bool teamed, std::array<bool, 12> max_combo_allowed, const std::string& color)
        : falleng::Game<Card_type, Player_type, Table_type>(teamed, max_combo_allowed)
        , m_color(color)
    {}

    std::string& get_color()
    {
        return m_color;
    }
private:
    std::string m_color = "\033[34m";
};

int main()
{
    int round = 0;
    std::cout << "Seed: ";
    int seed = 0;
    std::cin >> seed;
    std::cout << '\n';
    std::mt19937 eng(seed);
    
    bool teamed = y_n_ask("Teamed");
    bool enable_trivilin = y_n_ask("Use Trivilin");
    bool enable_extra_combos = y_n_ask("Use extra combos");

    std::array<bool, 12> combos = {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0};
    if (!enable_trivilin){combos[4] = false;}
    if (enable_extra_combos){
        combos[8] = true;
        combos[9] = true;
        combos[10] = true;
        combos[11] = true;
    }
    
    Neo_game<Neo_card, Neo_player<Neo_card>, Neo_table<Neo_card>> game(teamed, combos, "\033[36m");
    falleng::fespar::Fespar<decltype(game), decltype(eng)> fes(game, eng);
    
    fes.add_op("Gap", [&](std::vector<std::string>& args){args.empty() ? game.add_player("None") : game.add_player(args[0]); return 0;});

    std::string line;
    std::string arg;
    std::vector<std::string> arguments;
    report_round(round, game);
    print_state(game);
    std::cin.ignore(256, '\n');
    // TODO: open argv[1] as file, then pass argv[2] as (w)record or (r)replay

    while (std::getline(std::cin, line, '\n')){
        std::istringstream sline(line);
        while (sline >> arg){
            arguments.push_back(arg);
        }
        try {
            fes.exec_op(arguments);
        }
        catch (falleng::Op_not_valid_currently_exception& e){
            std::cout << '\n' << e.what() << '\n';
        }
        if (game.get_last_state().winner_found) break;
        arguments.clear();
        // Clear the screen
        if (game.is_playing()){
            std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
        }
        else {
            std::cout << '\n';
        }
        report_round(round, game);
        print_state(game);
        
    }

    std::cout << "\n\nWINNER:" << game.find_winners()[0].get().id();
}


