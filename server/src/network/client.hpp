/**
 * server/src/network/client.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_NETWORK_CLIENT_HPP
#define TRAINTASTIC_SERVER_NETWORK_CLIENT_HPP

#include <memory>
#include <queue>
#include <boost/asio.hpp>
#include "../core/objectptr.hpp"
#include <traintastic/network/message.hpp>

class Server;
class Session;

class Client : public std::enable_shared_from_this<Client>
{
  friend class Session;

  protected:
    using ObjectHandle = uint32_t;

    Server& m_server;
    boost::asio::ip::tcp::socket m_socket;
    const std::string m_id;
    struct
    {
      Message::Header header;
      std::shared_ptr<Message> message;
    } m_readBuffer;
    std::mutex m_writeQueueMutex;
    std::queue<std::unique_ptr<Message>> m_writeQueue;
    bool m_authenticated;
    std::shared_ptr<Session> m_session;

    void doReadHeader();
    void doReadData();
    void doWrite();

    void processMessage(const std::shared_ptr<Message> message);
    void sendMessage(std::unique_ptr<Message> message);

    void connectionLost();

  public:
    Client(Server& server, boost::asio::ip::tcp::socket socket);
    virtual ~Client();

    void disconnect();
};

#endif
