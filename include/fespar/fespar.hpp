#ifndef FALLENGINE_FESPAR_FESPAR_HPP_INCLUDED
#define FALLENGINE_FESPAR_FESPAR_HPP_INCLUDED

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>

#include "../fallengine/fallengine.hpp"

namespace falleng::fespar {

    template<class Game_type, class Uniform_random_engine>
    class Fespar {
    public:
        Fespar(Game_type& game, Uniform_random_engine& random_engine);
        int exec_op(std::vector<std::string>& ops);
        bool add_op(const std::string& op, std::function<int(std::vector<std::string>&)> action);
        bool mod_op(const std::string& op, std::function<int(std::vector<std::string>&)> action);
        void useargs(std::vector<std::string>& args){args.size();}
    private:
        std::map<std::string, std::function<int(std::vector<std::string>&)>> m_op_codes;
        Game_type& m_game;
        Uniform_random_engine& m_randeng;
    };

    template<class Game_type, class Uniform_random_engine>
    Fespar<Game_type, Uniform_random_engine>::Fespar(Game_type& game, Uniform_random_engine& random_engine)
        : m_game(game), m_randeng(random_engine)
    {
        // User needs to provide the Add_player action to the list
        m_op_codes["Gsp"] = [&](std::vector<std::string>& args){game.shuffle_players(m_randeng);useargs(args);return 0;};
        m_op_codes["Gop"] = [&](std::vector<std::string>& args){m_game.order_players(std::stoi(args[0]));return 0;};
        m_op_codes["Gi"] = [&](std::vector<std::string>& args){m_game.init_game();useargs(args);return 0;};
        m_op_codes["Grp"] = [&](std::vector<std::string>& data){game.remove_player(std::stoi(data[0]));return 0;}; 
        m_op_codes["Psp"] = [&](std::vector<std::string>& args){
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
        m_op_codes["psh"] = [&](std::vector<std::string>& args){
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
        m_op_codes["pst"] = [&](std::vector<std::string>& args){
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
        m_op_codes["psu"] = [&](std::vector<std::string>& args){m_game.current_player().undo_select();useargs(args);return 0;};
        m_op_codes["psc"] = [&](std::vector<std::string>& args){m_game.current_player().reset_selection();useargs(args);return 0;};
        m_op_codes["gs"] = [&](std::vector<std::string>& args){
            if (args.size() < 1){
                m_game.step(m_randeng, 0);
            }
            else {
                m_game.step(m_randeng, std::stoi(args[0]));
            }
            return 0;
        };
    }


    template<class Game_type, class Uniform_random_engine>
    int Fespar<Game_type, Uniform_random_engine>::exec_op(std::vector<std::string>& ops)
    {
        if (ops.size() == 0){return -2;};
        auto& opcode = ops[0];
        if (m_op_codes.find(opcode) != m_op_codes.end()){
            std::vector<std::string> args(ops.begin()+1, ops.end());
            return m_op_codes[opcode](args);
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

    template<class Game_type, class Uniform_random_engine>
    bool Fespar<Game_type, Uniform_random_engine>::add_op(const std::string& op, std::function<int(std::vector<std::string>&)> action)
    {
        return (m_op_codes.insert(std::pair(op, action))).second;
    }

    template<class Game_type, class Uniform_random_engine>
    bool Fespar<Game_type, Uniform_random_engine>::mod_op(const std::string& op, std::function<int (std::vector<std::string>&)> action)
    {
        if (m_op_codes.find(op) != m_op_codes.end()){
            m_op_codes[op] = action;
            return true;
        }
        else {
            return false;
        }
    }

} // namespace falleng::fespar

#endif // FALLENGINE_FESPAR_FESPAR_HPP_INCLUDED
