#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>

#include "include/fallEngine.h"

// TODO: fix Table::deal, doesn't deal (also meant changing Table::deal_to and Hand::set_cards)

void report_round(int& round, auto& game)
{
    std::cout << "Round: " << round++ << "===============================\n" <<
    "CP: " << game.current_player().id() << " DP: " << game.dealer().id() << '\n' ;
}

int main()
{
    int round = 0;
    std::mt19937 eng(1234567890);
    Game<false, Card, Player<false, Card>, Table<Card>, decltype(eng)> game(eng);
    game.add_player();
    game.add_player();
    game.add_player();
    game.add_player();
    std::cout << 'b' << game.get_table_cards().size();
    game.init_game();
    std::cout << 'i' << game.get_table_cards().size();
    game.step();
    report_round(round, game);
    std::cout << 's' << game.get_table_cards().size();
    std::cout << '\n';

    for (auto& card : game.get_table_cards()){
        std::cout << card.value() << ' ' << static_cast<int>(card.suit()) << '\n';
    }
    for (auto& player : game.get_players()){
        std::cout << "Player: " << player.id() << '\n';
        for (auto& card : player.get_cards()){
            std::cout << card.value() << ' ' << static_cast<int>(card.suit()) << '\n';
        }
    }

    game.current_player().select(game.current_player().get_cards()[0]);
    game.step();

    report_round(round, game);


    for (auto& card : game.get_table_cards()){
        std::cout << card.value() << ' ' << static_cast<int>(card.suit()) << '\n';
    }
    for (auto& player : game.get_players()){
        std::cout << "Player: " << player.id() << '\n';
        for (auto& card : player.get_cards()){
            std::cout << card.value() << ' ' << static_cast<int>(card.suit()) << '\n';
        }
    }
    // TODO: now test the player's methods, and continue the game, and make the interpeter for the small scripting
    // language that allows game automation (defined in fallengine/fescript/commands.txt)
}
