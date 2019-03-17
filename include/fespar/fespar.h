#ifndef FESPAR_H_INCLUDED
#define FESPAR_H_INCLUDED

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>

#include "../fallengine/fallengine.h"

template<class Game_type>
class Fespar {
public:
    Fespar(Game_type& game);
    int exec_op(std::vector<std::string> ops);
    bool add_op(std::string op, std::function<int(std::vector<std::string>)> action);
    bool mod_op(std::string op, std::function<int(std::vector<std::string>)> action);
private:
    std::map<std::string, std::function<int(std::vector<std::string>)>> m_op_codes;
    Game_type& m_game;
};

template<class Game_type>
Fespar<Game_type>::Fespar(Game_type& game)
    : m_game(game)
{
    // User needs to provide the Add_player action to the list
    m_op_codes["Gop"] = [&](std::vector<std::string> args){m_game.order_players(std::stoi(args[0]));return 0;};
    m_op_codes["Gsp"] = [&](std::vector<std::string> args){m_game.shuffle_players();return 0;};
    m_op_codes["Gi"] = [&](std::vector<std::string> args){m_game.init_game();return 0;};
    m_op_codes["Grp"] = [&](std::vector<std::string> data){game.remove_player(std::stoi(data[0]));return 0;}; 
    m_op_codes["Psp"] = [&](std::vector<std::string> args){
        try {
            auto& target = m_game.get_player(std::stoi(args[0]));
            auto& partner = m_game.get_player(std::stoi(args[1])); 
            target.set_partner(partner);
            partner.set_partner(target);
            return 0;
        }
        catch (const Player_not_found_exception& e){
            return 1;
        }
        catch (const std::invalid_argument& e){
            return 2;
        }
        catch (const std::out_of_range& e){
            return 3;
        }
    };
    m_op_codes["psh"] = [&](std::vector<std::string> args){
        try {
            auto& current = m_game.current_player();
            if (current.select(current.get_cards()[std::stoi(args[0])])){
                return 0;
            }
            else {
                return 1;
            }
        }
        catch (const std::invalid_argument& e){
            return 2;
        }
        catch (const std::out_of_range& e){
            return 3;
        }
    };
    m_op_codes["pst"] = [&](std::vector<std::string> args){
        try {
            auto& current = m_game.current_player();
            auto& target_card = m_game.get_table_cards()[std::stoi(args[0])];
            if (current.select(target_card)){
                return 0;
            }
            else {
                return 1;
            }
        }
        catch (const std::invalid_argument& e){
            return 2;
        }
        catch (const std::out_of_range& e){
            return 3;
        }
    };
    m_op_codes["psu"] = [&](std::vector<std::string> args){m_game.current_player().undo_select();return 0;};
    m_op_codes["psc"] = [&](std::vector<std::string> args){m_game.current_player().reset_selection();return 0;};
    m_op_codes["gs"] = [&](std::vector<std::string> args){
        if (args.size() < 1){
            m_game.step(0);
        }
        else {
            m_game.step(std::stoi(args[0]));
        }
        return 0;
    };
}


template<class Game_type>
int Fespar<Game_type>::exec_op(std::vector<std::string> ops)
{
    if (ops.size() == 0){return -2;};
    auto& opcode = ops[0];
    if (m_op_codes.find(opcode) != m_op_codes.end()){
        return m_op_codes[opcode](std::vector<std::string>(ops.begin()+1, ops.end()));
    }
    else {
        std::cout << "OP not found\n";
        return -1;
    }
    // if exists in map execute and return the result
    // else, return -1 (reserved)
    // values:
    // 0 success (mostly)
    // -1 non existent
    // 1.. function defined
}

template<class Game_type>
bool Fespar<Game_type>::add_op(std::string op, std::function<int(std::vector<std::string>)> action)
{
    return (m_op_codes.insert(std::pair(op, action))).second;
}

template<class Game_type>
bool Fespar<Game_type>::mod_op(std::string op, std::function<int (std::vector<std::string>)> action)
{
    if (m_op_codes.find(op) != m_op_codes.end()){
        m_op_codes[op] = action;
        return true;
    }
    else {
        return false;
    }
}

#endif // FESPAR_H_INCLUDED
