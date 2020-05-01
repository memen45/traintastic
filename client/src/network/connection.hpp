/**
 * client/src/network/client.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_CONNECTION_HPP
#define TRAINTASTIC_CLIENT_NETWORK_CONNECTION_HPP

#include <QObject>
#include <QAbstractSocket>
#include <QMap>
#include <QUuid>
#include <message.hpp>
#include "handle.hpp"
#include "objectptr.hpp"
#include "tablemodelptr.hpp"

class QTcpSocket;
class Property;
class ObjectProperty;
class Method;

class Connection : public QObject, public QEnableSharedFromThis<Connection>
{
  Q_OBJECT

  public:
    enum class State
    {
      Disconnected = 0,
      Disconnecting = 1,
      Connected = 2,
      Connecting = 3,
      SocketError = 4,
      ErrorAuthenticationFailed = 5,
      ErrorNewSessionFailed = 6,
    };

    using SocketError = QAbstractSocket::SocketError;

  protected:
    QTcpSocket* m_socket;
    State m_state;
    QString m_username;
    QByteArray m_password;
    struct
    {
      qint64 offset = 0;
      Message::Header header;
      std::shared_ptr<Message> message;
    } m_readBuffer;
    QMap<uint16_t, std::function<void(const std::shared_ptr<Message>&)>> m_requestCallback;
    QUuid m_sessionUUID;
    ObjectPtr m_traintastic;
    ObjectProperty* m_worldProperty;
    int m_worldRequestId;
    ObjectPtr m_world;
    QMap<Handle, QWeakPointer<Object>> m_objects;
    QMap<Handle, TableModel*> m_tableModels;

    void setState(State state);
    void processMessage(const std::shared_ptr<Message> message);
    void send(std::unique_ptr<Message>& message);
    void send(std::unique_ptr<Message>& message, std::function<void(const std::shared_ptr<Message>&)> callback);

    ObjectPtr readObject(const Message &message);
    TableModelPtr readTableModel(const Message& message);

    void getWorld();
    void setWorld(const ObjectPtr& world);

  protected slots:
    void socketConnected();
    void socketDisconnected();
    void socketError(QAbstractSocket::SocketError);
    void socketReadyRead();

  public:
    static const quint16 defaultPort = 5740;
    static constexpr int invalidRequestId = -1;

    //static Client* instance;

    Connection();
    ~Connection();

    inline bool isConnected() const { return m_state == State::Connected; }
    bool isDisconnected() const;
    State state() const { return m_state; }
    SocketError error() const;
    QString errorString() const;

    void connectToHost(const QUrl& url, const QString& username, const QString& password);
    void disconnectFromHost();

    void cancelRequest(int requestId);

    const ObjectPtr& traintastic() const { return m_traintastic; }
    const ObjectPtr& world() const { return m_world; }

    //[[nodiscard]] int createObject(const QString& classId, const QString& id, std::function<void(const ObjectPtr&, Message::ErrorCode)> callback);
    [[nodiscard]] int getObject(const QString& id, std::function<void(const ObjectPtr&, Message::ErrorCode)> callback);
    void releaseObject(Object* object);

    void setPropertyBool(Property& property, bool value);
    void setPropertyInt64(Property& property, int64_t value);
    void setPropertyDouble(Property& property, double value);
    void setPropertyString(Property& property, const QString& value);

    void callMethod(Method& method);
    [[nodiscard]] int callMethod(Method& method, std::function<void(const ObjectPtr&, Message::ErrorCode)> callback);

    [[nodiscard]] int getTableModel(const ObjectPtr& object, std::function<void(const TableModelPtr&, Message::ErrorCode)> callback);
    void releaseTableModel(TableModel* tableModel);
    void setTableModelRegion(TableModel* tableModel, int columnMin, int columnMax, int rowMin, int rowMax);

  signals:
    void stateChanged();
    void worldChanged();
};

#endif