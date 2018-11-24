#include <array>

#include "include/fallengine.h"
#include "include/tests_main.h"

/*
   This main should do:
   Unit test on every function
   Some integration tests
   A scripted test of all components based on a fixed random seed and scripted movements from the players, where:

   TODO:
   Escribir tests para TODO, basado en Catch2 (aprenderlo a usar)
   Escribir documentacion para TODO, basado en doxygen (ditto)

   Añadir la comparacion con ints de cards

 */

SCENARIO("Cards can be compared"){
    GIVEN("Two cards with same value (1) and suit"){
        {
            Card card_a(1), card_b(1); // Uses default argument for suit
            WHEN("Compared using build-in operators"){
                THEN("Only ==, <= and >= should return true"){
                    REQUIRE(card_a == card_b);
                    REQUIRE_FALSE(card_a != card_b);
                    REQUIRE_FALSE(card_a < card_b);
                    REQUIRE(card_a <= card_b);
                    REQUIRE(card_a >= card_b);
                    REQUIRE_FALSE(card_a > card_b);
                }
            }
            WHEN("Compared using is_same_card()"){
                THEN("It should return true"){
                    REQUIRE(is_same_card(card_a, card_b));
                }
            }
        }
    }
    GIVEN("Two cards with different values (a = 1, b = 2) and same suit"){
        {
            Card card_a(1), card_b(2);
            WHEN("Compared using built-in operators"){
                THEN("Only !=, < and <= should return true (asuming is written as a OPERATOR b)"){
                    REQUIRE_FALSE(card_a == card_b);
                    REQUIRE(card_a != card_b);
                    REQUIRE(card_a < card_b);
                    REQUIRE(card_a <= card_b);
                    REQUIRE_FALSE(card_a >= card_b);
                    REQUIRE_FALSE(card_a > card_b);
                }
            }
            WHEN("Compared using is_same_card"){
                THEN("It should return false"){
                    REQUIRE_FALSE(is_same_card(card_a, card_b));
                }
            }
        }
    }
    GIVEN("A set of cards (a = 1, b = 7, c = 10, d = 12) and same suit"){
        {
            Card card_a(1), card_b(7), card_c(10), card_d(12);
            WHEN("Added with ints 1, 2, 3"){
                THEN("a should return cards with values 2, 3, 4"){
                    REQUIRE(card_a + 1 == Card(2));
                    REQUIRE(card_a + 2 == Card(3));
                    REQUIRE(card_a + 3 == Card(4));
                }
                THEN("b should return cards with values 10, 11, 12"){
                    REQUIRE(card_b + 1 == Card(10));
                    REQUIRE(card_b + 2 == Card(11));
                    REQUIRE(card_b + 3 == Card(12));
                }
                THEN("c should return cards with values 11, 12, -1 (invalid or overflowed)"){
                    REQUIRE(card_a + 1 == Card(11));
                    REQUIRE(card_a + 2 == Card(12));
                    REQUIRE(card_a + 3 == Card(-1));
                }
                THEN("d should return cards with values -1, -1, -1 (invalid or overflowed)"){
                    REQUIRE(card_a + 1 == Card(-1));
                    REQUIRE(card_a + 2 == Card(-1));
                    REQUIRE(card_a + 3 == Card(-1));
                }
            }
            WHEN("Substracted ints 1, 2, 3"){
                // TODO: the same as above but with -
            }
        }
    }
}

TEST_CASE("Card operations tested") {
    REQUIRE((Card(1) == Card(1)));
    REQUIRE((Card(7) + 1 == Card(10)));
}
