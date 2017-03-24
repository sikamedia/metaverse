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

#include <metaverse/lib/explorer/commands/wif-to-public.hpp>

#include <iostream>
#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/explorer/define.hpp>


namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::wallet;

console_result wif_to_public::invoke(std::ostream& output, std::ostream& error)
{
    // Bound parameters.
    const auto& secret = get_wif_argument();

    output << ec_public(secret) << std::flush;
    return console_result::okay;
}

} //namespace commands 
} //namespace explorer 
} //namespace libbitcoin 
