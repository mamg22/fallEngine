#include "include/fallengine.h"
#include "include/tests_main.h"

/*
   This main should do:
   Unit test on every function
   Some integration tests
   A scripted test of all components based on a fixed random seed and scripted movements from the players
     note: make a playable game first, record the movements, and put them in a file

 */

SCENARIO("Cards can be compared", "[Card][Comparison]"){
    GIVEN("A card (value= 5) and two integers (3 and 5)"){
        {
            Card card(5);
            WHEN("Card is compared with 3"){
                THEN("Only >, >= and != should return true"){
                    REQUIRE_FALSE(card_a == 3);
                    REQUIRE(card_a != 3);
                    REQUIRE_FALSE(card_a < 3);
                    REQUIRE_FALSE(card_a <= 3);
                    REQUIRE(card_a >= 3);
                    REQUIRE(card_a > 3);
                }
            }
            WHEN("Card is compared with 5"){
                THEN("Only ==, >= and <= should return true"){
                    REQUIRE(card_a == 5);
                    REQUIRE_FALSE(card_a != 5);
                    REQUIRE(card_a < 5);
                    REQUIRE_FALSE(card_a <= 5);
                    REQUIRE(card_a >= 5);
                    REQUIRE_FALSE(card_a > 5);
                }
            }
        }
    }
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
}

SCENARIO("Cards can be added/substracted by an integer", "[Card]")
    GIVEN("A set of cards (a = 1, b = 7, c = 10, d = 12) and same suit"){
        {
            Card card_a(1), card_b(7), card_c(10), card_d(12);
            WHEN("Added with integers 1, 2, 3"){
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
                    REQUIRE(card_c + 1 == Card(11));
                    REQUIRE(card_c + 2 == Card(12));
                    REQUIRE(card_c + 3 == Card(-1));
                }
                THEN("d should return cards with values -1, -1, -1 (invalid or overflowed)"){
                    REQUIRE(card_d + 1 == Card(-1));
                    REQUIRE(card_d + 2 == Card(-1));
                    REQUIRE(card_d + 3 == Card(-1));
                }
            }
            WHEN("Substracted integers 1, 2, 3"){
                THEN("a should return cards with values -1, -1, -1 (invalid or underflowed)"){
                    REQUIRE(card_a - 1 == Card(-1));
                    REQUIRE(card_a - 2 == Card(-1));
                    REQUIRE(card_a - 3 == Card(-1));
                }
                THEN("b should return cards with values 6, 5, 4"){
                    REQUIRE(card_b - 1 == Card(6));
                    REQUIRE(card_b - 2 == Card(5));
                    REQUIRE(card_b - 3 == Card(4));
                }
                THEN("c should return cards with values 7, 6, 5"){
                    REQUIRE(card_c - 1 == Card(7));
                    REQUIRE(card_c - 2 == Card(6));
                    REQUIRE(card_c - 3 == Card(5));
                }
                THEN("d should return cards with values 11, 10, 7"){
                    REQUIRE(card_d - 1 == Card(11));
                    REQUIRE(card_d - 2 == Card(10));
                    REQUIRE(card_d - 3 == Card(7));
                }
            }
        }
    }
}
