#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>

#define private public
#include "include/fallEngine.h"

// Todo: Find WHY it doesn't deal to player nor the table, not even calling step many times works

int main()
{
    std::cout << std::hex;
    std::mt19937 eng(1234567890);
    Game<false, Card, Player<false, Card>, Table<Card>, decltype(eng)> game(eng);
    game.add_player();
    game.add_player();
    game.add_player();
    // if dealt correctly, table shoudl have 4 cards in it
    std::cout << game.get_table_cards().size();
    game.init_game();
    std::cout << game.get_table_cards().size();
    game.step();
    std::cout << game.get_table_cards().size();
        game.step();
    std::cout << game.get_table_cards().size();
        game.step();
    std::cout << game.get_table_cards().size();
    // Should return 3
    std::cout << game.current_player().get_cards().size();
}
