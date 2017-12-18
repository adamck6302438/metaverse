/**
 * Copyright (c) 2016-2017 mvs developers 
 *
 * This file is part of metaverse-explorer.
 *
 * metaverse-explorer is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/getasset.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getasset *************************/

console_result getasset::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);
    
    if (argument_.symbol.size() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw asset_symbol_length_exception{"asset symbol exceed length ?"};

    // 1. first search asset in blockchain
    // std::shared_ptr<std::vector<asset_detail>> 
    auto sh_vec = blockchain.get_issued_assets();
    auto sh_local_vec = blockchain.get_local_assets();
#ifdef MVS_DEBUG
    const auto action = [&](asset_detail& elem)
    {
        log::info("getasset blockchain") << elem.to_string();
    };
    std::for_each(sh_vec->begin(), sh_vec->end(), action);
#endif
    //if(sh_vec.empty() && sh_local_vec.empty()) // not found any asset
        //throw asset_notfound_exception{"no asset found ?"};
#ifdef MVS_DEBUG
    const auto lc_action = [&](asset_detail& elem)
    {
        log::info("getasset local") << elem.to_string();
    };
    std::for_each(sh_local_vec->begin(), sh_local_vec->end(), lc_action);
#endif
    
    Json::Value aroot;
    Json::Value assets;
    // add blockchain assets
    for (auto& elem: *sh_vec) {
        if( elem.get_symbol().compare(argument_.symbol) != 0 )// not request asset symbol
            continue;
        Json::Value asset_data;
        asset_data["symbol"] = elem.get_symbol();
        asset_data["maximum_supply"] += elem.get_maximum_supply();
        asset_data["decimal_number"] += elem.get_decimal_number();
        asset_data["issuer"] = elem.get_issuer();
        asset_data["address"] = elem.get_address();
        asset_data["description"] = elem.get_description();
        asset_data["status"] = "issued";
        assets.append(asset_data);
        
        aroot["assets"] = assets;
        output << aroot.toStyledString();
        return console_result::okay;
    }
    
    // add local assets
    for (auto& elem: *sh_local_vec) {
        if( elem.get_symbol().compare(argument_.symbol) != 0 )// not request asset symbol
            continue;
        Json::Value asset_data;
        asset_data["symbol"] = elem.get_symbol();
        asset_data["maximum_supply"] += elem.get_maximum_supply();
        asset_data["decimal_number"] += elem.get_decimal_number();
        asset_data["issuer"] = elem.get_issuer();
        asset_data["address"] = elem.get_address();
        asset_data["description"] = elem.get_description();
        asset_data["status"] = "unissued";
        assets.append(asset_data);
        
        aroot["assets"] = assets;
        output << aroot.toStyledString();
        return console_result::okay;
    }
    
    aroot["assets"] = assets;
    output << aroot.toStyledString();
    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

