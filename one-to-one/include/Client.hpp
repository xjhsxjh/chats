#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Message.hpp"
#include "MessageIO.hpp"

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/asio.hpp>
#include <string>

/*!
 * \brief The ClientTCP class is tcp client implementation
 * route:
 *  - connect (call to Connect)
 *  - exchange data
 *  - disconnect
 */
class ClientTCP : virtual public MessageIOTCP {
public:
    /*!
     * \brief ClientTCP
     * \param _app_cv conditional to notify user about new message with
     * \param _io_service io_service to use
     * \param _connection_cv conditional to notify user about connection with
     */
    ClientTCP(boost::weak_ptr<boost::condition_variable> _app_cv,
              boost::shared_ptr<boost::asio::io_service> &_io_service,
              boost::weak_ptr<boost::condition_variable> _connection_cv);

    /*!
     * \brief calls to async_connect
     * \param _addr address
     * \param _port port
     */
    void Connect(std::string _addr,
                 unsigned short _port);
    /*!
     * \brief disconnects (shutdown and close the socket)
     */
    void Disconnect();

    /*!
     * \brief API for application to asynchronously send message
     * (should check if connected)
     */
    void SendMsg(MessagePtr _msg);

    /*!
     * \brief API to start receiving messages
     */
    void StartReceiver();

protected:
    /********** functions ************/
    /*!
     * \brief callback of async_connect
     * \param err
     * \param it
     */
    void _OnConnect(const boost::system::error_code &err);

    /********** variables ************/
    /// conditional to notify user about connection with
    boost::weak_ptr<boost::condition_variable> m_connection_cv;
    /// remote endpoint to connect to
    boost::asio::ip::tcp::endpoint m_remote_ep;
    /// io_service
    boost::shared_ptr<boost::asio::io_service> m_io_service;
    /// flags if is in connected or not
    bool m_connected = false;
};

/*!
 * \brief The ClientUDP class is udp client implementation
 * route:
 *  - set whom to talk with
 *  - exchange data
 */
class ClientUDP : virtual public MessageIOUDP {
};

#endif // CLIENT_HPP
