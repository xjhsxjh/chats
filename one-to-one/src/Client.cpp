#include "Client.hpp"
#include "MessageIO.hpp"
#include "Message.hpp"
#include <string>

/************* TCP client class implementation **************/
#undef _ParentClass
#define _ParentClass MessageIOTCP

ClientTCP::ClientTCP(boost::weak_ptr<boost::condition_variable> _app_cv,
                     boost::shared_ptr<boost::asio::io_service> &_io_service,
                     boost::weak_ptr<boost::condition_variable> _connection_cv) :
    MessageIO(_app_cv),
    _ParentClass(_app_cv, _io_service),
    m_connection_cv(_connection_cv),
    m_io_service(_io_service)
{
}

ClientTCP::~ClientTCP()
{
    Disconnect();
}

void
ClientTCP::Connect(std::string _addr, unsigned short _port)
{
    if (m_connected) {
        // disconnect then
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        m_socket.close();
    }

    boost::asio::ip::tcp::resolver _r(*m_io_service);
    boost::asio::ip::tcp::resolver::query _q(_addr, std::to_string(_port));
    boost::asio::ip::tcp::resolver::iterator _it = _r.resolve(_q);

    m_remote_ep = *_it;

    printf("Connecting to %s : %u\n",
           m_remote_ep.address().to_string().c_str(),
           m_remote_ep.port());
    m_socket.async_connect(m_remote_ep,
                           boost::bind(&ClientTCP::_OnConnect,
                                       this,
                                       _1));
}

void
ClientTCP::_OnConnect(const boost::system::error_code &err)
{
    /// TODO do smth on error
    printf("[Client] _OnClient, error message: %d = %s\n",
           err.value(),
           err.message().c_str());

    if (!err) {
        printf("[Client]\t connected!\n");
        boost::unique_lock<boost::mutex> _scoped(m_connected_mutex);
        m_connected = true;
        _scoped.unlock();
        if (auto _cv = m_connection_cv.lock()) {
            printf("[Client] Notifying!\n");
            _cv->notify_all();
        }
    }
}

void
ClientTCP::Disconnect()
{
    boost::unique_lock<boost::mutex> _scoped(m_connected_mutex);
    if (!m_connected) return;
    m_connected = false;
    _scoped.unlock();
    m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    m_socket.close();
}

bool
ClientTCP::StartReceiver()
{
    boost::unique_lock<boost::mutex> _scoped(m_connected_mutex);
    if (!m_connected) return false;
    return _ParentClass::StartReceiver();
}

void
ClientTCP::SendMsg(MessagePtr _msg)
{
    boost::unique_lock<boost::mutex> _scoped(m_connected_mutex);
    if (m_connected)
        _ParentClass::SendMsg(_msg);
}

bool
ClientTCP::IsConnected()
{
    boost::unique_lock<boost::mutex> _scoped(m_connected_mutex);

    return m_connected;
}

/************* UDP client class implementation **************/
#undef _ParentClass
#define _ParentClass MessageIOUDP

ClientUDP::ClientUDP(boost::weak_ptr<boost::condition_variable> _app_cv,
                     boost::shared_ptr<boost::asio::io_service> &_io_service,
                     boost::weak_ptr<boost::condition_variable> _connection_cv) :
    MessageIO(_app_cv),
    _ParentClass(_app_cv, _io_service),
    m_io_service(_io_service)
{
}

void
ClientUDP::_ShutdownSocket()
{
    m_socket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
    m_socket.close();
}

void
ClientUDP::SetRemote(std::string _address, unsigned short _port)
{
    if (m_remote_set) {
        _ShutdownSocket();
    }

    boost::asio::ip::udp::resolver _r(*m_io_service);
    boost::asio::ip::udp::resolver::query _q(_address, std::to_string(_port));
    boost::asio::ip::udp::resolver::iterator _it = _r.resolve(_q);

    m_remote = _it->endpoint();

    m_remote_set = true;
}

bool ClientUDP::StartReceiver()
{
    if (!m_remote_set) return false;
    return _ParentClass::StartReceiver();
}

void
ClientUDP::SendMsg(MessagePtr _msg)
{
    if (!m_remote_set) return;
    _ParentClass::SendMsg(_msg);
}
