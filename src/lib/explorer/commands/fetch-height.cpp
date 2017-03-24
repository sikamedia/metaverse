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

#include <metaverse/lib/explorer/commands/fetch-height.hpp>

#include <cstddef>
#include <iostream>
#include <metaverse/lib/client.hpp>
#include <metaverse/lib/explorer/callback_state.hpp>
#include <metaverse/lib/explorer/define.hpp>
#include <metaverse/lib/explorer/display.hpp>
#include <metaverse/lib/explorer/utility.hpp>


namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::client;
using namespace bc::explorer::config;

console_result fetch_height::invoke(std::ostream& output, std::ostream& error)
{
    // Bound parameters.
    const auto& server_url = get_server_url_argument();
    const auto connection = get_connection(*this);

    obelisk_client client(connection);

    // For this command only, allow command line to override server config.
    if (!server_url.empty())
    {
        if (!client.connect(server_url))
        {
            display_connection_failure(error, server_url);
            return console_result::failure;
        }
    }
    else
    {
        if (!client.connect(connection))
        {
            display_connection_failure(error, connection.server);
            return console_result::failure;
        }
    }

    callback_state state(error, output);

    auto on_done = [&state](size_t height)
    {
        state.output(height);
    };

    auto on_error = [&state](const code& error)
    {
        state.succeeded(error);
    };

    client.blockchain_fetch_last_height(on_error, on_done);
    client.wait();

    return state.get_result();
}

} //namespace commands 
} //namespace explorer 
} //namespace libbitcoin 
