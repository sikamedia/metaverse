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
#include <metaverse/lib/explorer/commands/ek-address.hpp>

#include <algorithm>
#include <iostream>
#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/explorer/define.hpp>


namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::wallet;

console_result ek_address::invoke(std::ostream& output, std::ostream& error)
{
    const auto uncompressed = get_uncompressed_option();
    const auto version = get_version_option();
    const auto& token = get_token_argument();
    const data_chunk& seed = get_seed_argument();

    if (seed.size() < ek_seed_size)
    {
        error << BX_EK_ADDRESS_SHORT_SEED << std::flush;
        return console_result::failure;
    }
    
    ek_seed bytes;
    std::copy(seed.begin(), seed.begin() + ek_seed_size, bytes.begin());
    const auto compressed = !uncompressed;

    ec_compressed point;
    encrypted_private unused;
    create_key_pair(unused, point, token, bytes, version, compressed);
    const payment_address address({ point, compressed }, version);
    
    output << address << std::flush;
    return console_result::okay;
}

} //namespace commands 
} //namespace explorer 
} //namespace libbitcoin 
