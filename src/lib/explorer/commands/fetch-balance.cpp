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

#include <metaverse/lib/explorer/commands/fetch-balance.hpp>

#include <iostream>
#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/client.hpp>
#include <metaverse/lib/explorer/callback_state.hpp>
#include <metaverse/lib/explorer/define.hpp>
#include <metaverse/lib/explorer/display.hpp>
#include <metaverse/lib/explorer/prop_tree.hpp>


namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::chain;
using namespace bc::client;
using namespace bc::explorer::config;

console_result fetch_balance::invoke(std::ostream& output, std::ostream& error)
{
    // Bound parameters.
    const auto& encoding = get_format_option();
    const auto& address = get_payment_address_argument();
    const auto connection = get_connection(*this);

    obelisk_client client(connection);

    if (!client.connect(connection))
    {
        display_connection_failure(error, connection.server);
        return console_result::failure;
    }

    callback_state state(error, output, encoding);

    auto on_done = [&state, &address](const history::list& rows)
    {
        // This override summarizes the history response as balance.
        state.output(prop_tree(rows, address));
    };

    auto on_error = [&state](const code& error)
    {
        state.succeeded(error);
    };

    // The v3 client API works with and normalizes either server API.
    //// client.address_fetch_history(on_error, on_done, address);
    client.address_fetch_history2(on_error, on_done, address);
    client.wait();

    return state.get_result();
}

} //namespace commands 
} //namespace explorer 
} //namespace libbitcoin 
