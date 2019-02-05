#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <sstream>

#include "../include/fallengine.h"
#include "../include/fespar/fespar.h"

// TODO: Reparar Table::take_card, que eleimina cosas basado en la posicion y no en la carte en si (Semilla 99, eliminar 2, 3. elimina 2, 7)

template<class Game_type>
void report_round(int& round, Game_type& game)
{
    if (!game.is_playing()) {std::cout << "!!!"; return;};
    try {game.current_player();} catch (Player_not_found_exception& e){return;}
    std::cout << "Round: " << round++ << "\n===============================\n" <<
    "CP: " << game.current_player().id() << " DP: " << game.dealer().id() << " BC: " << game.best_combo_p().id() << "\n\n";
}

template<class Game_type>
void print_state(Game_type& game)
{
    if (!game.is_playing()) {std::cout << "!!!"; return;};
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

int main()
{
    int round = 0;
    std::cout << "SEED: ";
    int seed = 0;
    std::cin >> seed;
    std::cout << '\n';
    std::mt19937 eng(seed);
    Game<false, Card, Player<false, Card>, Table<Card>, decltype(eng)> game(eng, Combo::Casa_grande);
    Fespar<decltype(game)> fes(game);
    
    std::string line;
    std::string arg;
    std::vector<std::string> arguments;
    report_round(round, game);
    print_state(game);
    std::cin.ignore(256, '\n');

    
    while (std::getline(std::cin, line, '\n')){
        
        //auto spli = split(line, ' ');
        //std::cout << spli.size();
        //for (auto& str: spli){std::cout << str;}
        std::istringstream sline(line);
        while (sline >> arg){
            arguments.push_back(arg);
        }
        std::cout << arguments.size();
        fes.exec_op(arguments);
        arguments.clear();
        report_round(round, game);
        print_state(game);
        //std::cin.ignore(256, '\n');
        //if (!std::cin) throw 1;
    }
    /*
    std::cout << 'b' << game.get_table_cards().size();
    game.init_game();
    std::cout << 'i' << game.get_table_cards().size();
    game.step();
    std::cout << 's' << game.get_table_cards().size();
    std::cout << '\n';
    report_round(round, game);

    print_state(game);

    game.current_player().select(game.current_player().get_cards()[0]);
    game.current_player().reset_selection();
    game.current_player().select(game.current_player().get_cards()[1]);
    game.step();

    report_round(round, game);
    print_state(game);

    game.current_player().select(game.current_player().get_cards()[2]);
    game.step();

    report_round(round, game);
    print_state(game);

    // TODO: now test the player's methods, and continue the game, and make the interpeter for the small scripting
    // language that allows game automation (defined in fallengine/fescript/commands.txt
*/
}


