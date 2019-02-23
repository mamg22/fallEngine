#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <sstream>
#include <array>

#include "../include/fallengine/fallengine.h"
#include "../include/fespar/fespar.h"

/* TODO: Add some behavior in Game to disallow some behavior when the game is and is not running (such as adding players after the game
 * begun or similar
 * 
 * Find a way to record a game and replay it given a file, because repeating it is annoying (see notes)
 */
template<class Game_type>
void report_round(int& round, Game_type& game)
{
    if (!game.is_playing()) {return;};
    try {game.current_player();} catch (Player_not_found_exception& e){return;}
    std::cout << "Round: " << round++ << "\n===============================\n" <<
    "CP: " << game.current_player().id() << " DP: " << game.dealer().id() << " BC: " << game.best_combo_p().id() << "\n\n";
}

template<class Game_type>
void print_state(Game_type& game)
{
    if (!game.is_playing()) {return;};
    try {game.current_player();} catch (Player_not_found_exception& e){return;}
    std::cout << "Table: ";
    for (auto& card : game.get_table_cards()){
        std::cout << card.value() << ' ' << static_cast<char>(static_cast<int>(card.suit())+0x40) << " \t" ;
    }
    std::cout << "\n\n";
    for (auto& player : game.get_players()){
        std::cout << "Player: " << player.id() << "\t S: " << player.get_score() << "\t Combo:" << player.get_combo_name() << '\n';
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

bool y_n_ask(auto prompt)
{
    char choice = 'n';
    std::cout << prompt << " [y/n]: ";
    std::cin >> choice;
    if (choice == 'y' || choice == 'Y'){return true;}
    return false;
}

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
    
    Game<Card, Player<Card>, Table<Card>, decltype(eng)> game(eng, teamed, combos);
    Fespar<decltype(game)> fes(game);

    std::string line;
    std::string arg;
    std::vector<std::string> arguments;
    report_round(round, game);
    print_state(game);
    std::cin.ignore(256, '\n');
    // open argv[1] as file, then pass argv[2] as (w)record or (r)replay
    

    while (std::getline(std::cin, line, '\n')){
        // write line to file
        std::istringstream sline(line);
        while (sline >> arg){
            arguments.push_back(arg);
        }
        try {
            fes.exec_op(arguments);
        }
        catch (Op_not_valid_currently_exception& e){
            std::cout << '\n' << e.what() << '\n';
        }
        if (game.get_last_state().winner_found) break;
        arguments.clear();
        // Clear the screen
        if (game.is_playing()){
            std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
        }
        else {
            std::cout << '\n';
        }
        report_round(round, game);
        print_state(game);
        
    }

    std::cout << "\n\nWINNER:" << game.find_winners()[0].get().id();
    // TODO: now test the player's methods, and continue the game, and make the interpeter for the small scripting
    // language that allows game automation (defined in fallengine/fescript/commands.txt
}


