/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-explorer.
 *
 * libbitcoin-explorer is free software: you can redistribute it and/or
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
#include <metaverse/lib/explorer/commands/tx-decode.hpp>

#include <iostream>
#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/explorer/define.hpp>
#include <metaverse/lib/explorer/prop_tree.hpp>
#include <metaverse/lib/explorer/utility.hpp>


namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

console_result tx_decode::invoke(std::ostream& output, std::ostream& error)
{
    // Bound parameters.
    const auto& encoding = get_format_option();
    const auto& transaction = get_transaction_argument();

    // This enables json-style array formatting.
    const auto json = encoding == encoding_engine::json;

    write_stream(output, prop_tree(transaction, json), encoding);

    return console_result::okay;
}

} //namespace commands 
} //namespace explorer 
} //namespace libbitcoin 
