#include "Server.hpp"
#include "MessageIO.hpp"
#include "Message.hpp"
#include "thread-pool.hpp"

#include <boost/thread.hpp>

#undef _ParentClass
#define _ParentClass MessageIOTCP

Server::Server(boost::weak_ptr<boost::condition_variable> _app_cv,
               boost::shared_ptr<boost::asio::io_service> &_io_service,
               boost::weak_ptr<boost::condition_variable> _connection_cv,
               boost::shared_ptr<ThreadPool> &_thread_pool,
               unsigned short _port) :
    MessageIO(_app_cv),
    _ParentClass(_app_cv, _io_service),
    m_connection_cv(_connection_cv),
    m_connection_acceptor(),//new boost::asio::ip::tcp::acceptor(*_io_service)),
    m_thread_pool(_thread_pool),
    m_port(_port),
    m_listen_ep(boost::asio::ip::tcp::v4(), _port)
{
}

Server::~Server()
{
    Disconnect();
}

void
Server::Listen()
{
    /// setup socket for listening

    if (!m_connection_acceptor.get()) {
        // initialize connection acceptor
        m_connection_acceptor.reset(
                new boost::asio::ip::tcp::acceptor(*m_io_service,
                                                   m_listen_ep,
                                                   true));
    }

    m_connection_acceptor->async_accept(
                m_socket, // new connection will be accepted into this socket
                m_listen_ep, // listening endpoint
                boost::bind(&Server::_OnConnection,
                            this,
                            _1));
    m_listening = true;
}

void
Server::_OnConnection(const boost::system::error_code &err)
{
    m_listening = false;
    printf("[Server] some kind of connection here\n");
    /// TODO do smth on error
    if (!err) {
        boost::unique_lock<boost::mutex> _scoped(m_connected_mutex);
        m_connected = true;
        _scoped.unlock();
        if (auto _cv = m_connection_cv.lock()) {
            _cv->notify_all();
        }
    }
}

void
Server::Disconnect()
{
    boost::unique_lock<boost::mutex> _scoped(m_connected_mutex);
    if (!m_connected) return;
    m_connected = false;
    _scoped.unlock();

    m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    m_socket.close();
}

bool
Server::StartReceiver()
{
    boost::unique_lock<boost::mutex> _scoped(m_connected_mutex);
    if (!m_connected) return false;
    return _ParentClass::StartReceiver();
}

void
Server::SendMsg(MessagePtr _msg)
{
    boost::unique_lock<boost::mutex> _scoped(m_connected_mutex);
    if (m_connected)
        _ParentClass::SendMsg(_msg);
}

boost::asio::ip::tcp::endpoint
Server::Remote()
{
    boost::unique_lock<boost::mutex> _scoped(m_connected_mutex);
    if (m_connected)
        return m_socket.remote_endpoint();
    return boost::asio::ip::tcp::endpoint();
}

bool
Server::IsConnected() const
{
    boost::unique_lock<boost::mutex> _scoped(m_connected_mutex);
    return m_connected;
}
