#include <iostream>
#include <fstream>
#include <random>

#include "include/fallEngine.h"

// testing the functions so the errors come out

int main()
{
    std::mt19937 eng(1234567890);
    Game<false, Card, Player<false, Card>, Table<Card>, decltype(eng)> game(eng);
    game.add_player();
    game.add_player();
    game.add_player();
    game.add_player();
    game.init_game();
    for (auto& player : game.get_players()){
        std::cout << player.id() << ' ';
    }
    std::cout << '\n';
    for (auto& card : game.get_table_cards()){
        std::cout << card.value() << ' ' << static_cast<int>(card.suit()) << '\n';
    }
    game.step();

}
