/**
 * server/src/hardware/protocol/loconet/iohandler/tcpiohandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_IOHANDLER_TCPIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_IOHANDLER_TCPIOHANDLER_HPP

#include "iohandler.hpp"
#include <boost/asio/ip/tcp.hpp>

namespace LocoNet {

class TCPIOHandler : public IOHandler
{
  protected:
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::endpoint m_endpoint;

    TCPIOHandler(Kernel& kernel, const std::string& hostname, uint16_t port);

  public:
    ~TCPIOHandler() override;
};

}

#endif
