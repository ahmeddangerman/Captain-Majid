#include "captain/defensive_move_inspection.hpp"
#include "captain/shot_content.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc,char** argv) {
    if(argc==3 && std::string(argv[1])=="--batch") {
        std::ifstream input(argv[2],std::ios::binary);
        if(!input)return 2;
        const auto content=captain::load_shot_content(input);
        unsigned id,type,level,action,variant,subtype,stamina;
        while(std::cin>>id>>type>>level>>action>>variant>>subtype>>stamina) {
            if(id>255 || type>255 || level>255 || action>255 || variant>255 || subtype>255 || stamina>65535)return 2;
            captain::FieldPlayerRecord player{static_cast<std::uint8_t>(id),static_cast<std::uint8_t>(type),
                static_cast<std::uint8_t>(stamina),static_cast<std::uint8_t>(stamina>>8),static_cast<std::uint8_t>(level)};
            try {
                const auto result=captain::inspect_defensive_move(player,stamina,action,variant,subtype,content.field_players,content.defense);
                std::cout<<result.cost<<' '<<result.allowed<<' '<<unsigned(result.effect)<<'\n';
            } catch(const std::exception&) {std::cout<<"unsupported\n";}
        }
        return std::cin.eof()?0:2;
    }
    if(argc!=1)return 2;
    captain::FieldPlayerProfiles profiles{};profiles.skills.resize(1);
    captain::FieldDefenseContent defense{};
    defense.skill_selectors[2][1]=1;defense.moves[2].push_back({0,0,50,0});
    defense.moves[2].push_back({0,0,180,0});
    captain::FieldPlayerRecord player{1,1,179,0,0};
    try {
        const auto special=captain::inspect_defensive_move(player,179,2,1,1,profiles,defense);
        const auto ordinary=captain::inspect_defensive_move(player,179,2,1,0,profiles,defense);
        if(special.cost!=180 || special.allowed || ordinary.cost!=50 || !ordinary.allowed)throw std::runtime_error("wrong move decision");
        bool rejected=false;try {captain::inspect_defensive_move(player,180,2,1,1,profiles,defense);} catch(const std::invalid_argument&) {rejected=true;}
        if(!rejected)throw std::runtime_error("inconsistent raw stamina accepted");
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
    std::cout<<"Defensive move inspection checks passed\n";
}
