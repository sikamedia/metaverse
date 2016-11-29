/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-server.
 *
 * libbitcoin-server is free software: you can redistribute it and/or
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
#include "executor.hpp"

#include <algorithm>
#include <csignal>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <bitcoin/server.hpp>
#include <bitcoin/bitcoin/utility/backtrace.hpp>

namespace libbitcoin {
namespace server {

using boost::format;
using namespace std::placeholders;
using namespace boost::system;
using namespace bc::config;
using namespace bc::database;
using namespace bc::network;

static constexpr int initialize_stop = 0;
static constexpr int directory_exists = 0;
static constexpr int directory_not_found = 2;
static constexpr auto append = std::ofstream::out | std::ofstream::app;
static const auto application_name = "bs";

std::promise<code> executor::stopping_;

executor::executor(parser& metadata, std::istream& input,
    std::ostream& output, std::ostream& error)
  : metadata_(metadata), output_(output),
    debug_file_(metadata_.configured.network.debug_file.string(), append),
    error_file_(metadata_.configured.network.error_file.string(), append)
{
    initialize_logging(debug_file_, error_file_, output, error);
    handle_stop(initialize_stop);
}

// Command line options.
// ----------------------------------------------------------------------------
// Emit directly to standard output (not the log).

void executor::do_help()
{
    const auto options = metadata_.load_options();
    printer help(options, application_name, BS_INFORMATION_MESSAGE);
    help.initialize();
    help.commandline(output_);
}

void executor::do_settings()
{
    const auto settings = metadata_.load_settings();
    printer print(settings, application_name, BS_SETTINGS_MESSAGE);
    print.initialize();
    print.settings(output_);
}

void executor::do_version()
{
    output_ << format(BS_VERSION_MESSAGE) %
        MVS_SERVER_VERSION %
        MVS_PROTOCOL_VERSION %
        MVS_NODE_VERSION %
        MVS_BLOCKCHAIN_VERSION %
        MVS_VERSION << std::endl;
}

// Emit to the log.
bool executor::do_initchain()
{
    initialize_output();
    
    boost::system::error_code ec;
    const auto& directory = metadata_.configured.database.directory;

    if (create_directories(directory, ec))
    {
        log::info(LOG_SERVER) << format(BS_INITIALIZING_CHAIN) % directory;

        // Unfortunately we are still limited to a choice of hardcoded chains.
        //const auto genesis = metadata_.configured.chain.use_testnet_rules ?
         //   chain::block::genesis_testnet() : chain::block::genesis_mainnet();
        auto genesis = consensus::miner::create_genesis_block();

        const auto result = data_base::initialize(directory, *genesis);

        log::info(LOG_SERVER) << BS_INITCHAIN_COMPLETE;
        return result;
    }

    if (ec.value() == directory_exists)
    {
        log::error(LOG_SERVER) << format(BS_INITCHAIN_EXISTS) % directory;
        return false;
    }

    log::error(LOG_SERVER) << format(BS_INITCHAIN_NEW) % directory % ec.message();
    return false;
}

// Menu selection.
// ----------------------------------------------------------------------------

bool executor::menu()
{
    const auto& config = metadata_.configured;

    if (config.help)
    {
        do_help();
        return true;
    }

    if (config.settings)
    {
        do_settings();
        return true;
    }

    if (config.version)
    {
        do_version();
        return true;
    }

    if (config.initchain)
    {
        return do_initchain();
    }

    // There are no command line arguments, just run the server.
    return run();
}

// Run.
// ----------------------------------------------------------------------------

bool executor::run()
{
    initialize_output();

    log::info(LOG_SERVER) << BS_NODE_STARTING;

    if (!verify_directory())
        return false;

    // Ensure all configured services can function.
    set_minimum_threadpool_size();

    // Now that the directory is verified we can create the node for it.
    node_ = std::make_shared<server_node>(metadata_.configured);

    // The callback may be returned on the same thread.
    node_->start(
        std::bind(&executor::handle_started,
            this, _1));

    // Wait for stop.
    stopping_.get_future().wait();

    log::info(LOG_SERVER) << BS_NODE_STOPPING;

    // Close must be called from main thread.
    if (node_->close())
        log::info(LOG_NODE) << BS_NODE_STOPPED;
    else
        log::info(LOG_NODE) << BS_NODE_STOP_FAIL;

    return true;
}

// Handle the completion of the start sequence and begin the run sequence.
void executor::handle_started(const code& ec)
{
    if (ec)
    {
        log::error(LOG_SERVER) << format(BS_NODE_START_FAIL) % ec.message();
        stop(ec);
        return;
    }

    log::info(LOG_SERVER) << BS_NODE_SEEDED;

    // This is the beginning of the stop sequence.
    node_->subscribe_stop(
        std::bind(&executor::handle_stopped,
            this, _1));

    // This is the beginning of the run sequence.
    node_->run(
        std::bind(&executor::handle_running,
            this, _1));
}

// This is the end of the run sequence.
void executor::handle_running(const code& ec)
{
    if (ec)
    {
        log::info(LOG_SERVER) << format(BS_NODE_START_FAIL) % ec.message();
        stop(ec);
        return;
    }

    log::info(LOG_SERVER) << BS_NODE_STARTED;
}

// This is the end of the stop sequence.
void executor::handle_stopped(const code& ec)
{
    stop(ec);
}

// Stop signal.
// ----------------------------------------------------------------------------

void executor::handle_stop(int code)
{
    // Reinitialize after each capture to prevent hard shutdown.
    std::signal(SIGINT, handle_stop);
    std::signal(SIGTERM, handle_stop);
    std::signal(SIGABRT, handle_stop);

    if (code == initialize_stop)
        return;

    if(SIGINT != code)
    {
    	do_backtrace("signal.out");
    }

    log::info(LOG_SERVER) << format(BS_NODE_SIGNALED) % code;
    stop(error::success);
}

void executor::stop(const code& ec)
{
    static std::once_flag stop_mutex;
    std::call_once(stop_mutex, [&](){ stopping_.set_value(ec); });
}

// Utilities.
// ----------------------------------------------------------------------------

// Set up logging.
void executor::initialize_output()
{
    log::debug(LOG_SERVER) << BS_LOG_HEADER;
    log::info(LOG_SERVER) << BS_LOG_HEADER;
    log::warning(LOG_SERVER) << BS_LOG_HEADER;
    log::error(LOG_SERVER) << BS_LOG_HEADER;
    log::fatal(LOG_SERVER) << BS_LOG_HEADER;

    const auto& file = metadata_.configured.file;

    if (file.empty())
        log::info(LOG_SERVER) << BS_USING_DEFAULT_CONFIG;
    else
        log::info(LOG_SERVER) << format(BS_USING_CONFIG_FILE) % file;
}

// Use missing directory as a sentinel indicating lack of initialization.
bool executor::verify_directory()
{
    boost::system::error_code ec;
    const auto& directory = metadata_.configured.database.directory;

    if (exists(directory, ec))
        return true;

    if (ec.value() == directory_not_found)
    {
        log::error(LOG_SERVER) << format(BS_UNINITIALIZED_CHAIN) % directory;
        return false;
    }

    const auto message = ec.message();
    log::error(LOG_SERVER) << format(BS_INITCHAIN_TRY) % directory % message;
    return false;
}

// Increase the configured minomum as required to operate the service.
void executor::set_minimum_threadpool_size()
{
    metadata_.configured.network.threads =
        std::max(metadata_.configured.network.threads,
            server_node::threads_required(metadata_.configured));
}

} // namespace server
} // namespace libbitcoin
