#ifndef __APPLE_PUSH_H__
#define __APPLE_PUSH_H__

#include <iostream>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/bind.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/format.hpp>
#include "log.h"
#include "push.h"

class ios_push_client : boost::noncopyable
{
private:
    friend class ios_push_mgr;
    struct config
    {
        std::string passwd;
        std::string kfn;
        std::string cfn;
        push_mgr::channel ch;
        boost::asio::ip::tcp::endpoint ep;

        std::string getpwd(std::size_t max_length, 
                boost::asio::ssl::context::password_purpose purpose) const
        {
            return passwd;
        }

        friend std::ostream& operator<<(std::ostream& out, config const& cf)
        {
            return out << cf.ch <<"/"<< cf.ep;
        }
    }conf_;

public:
    typedef unsigned int UInt;
    typedef ios_push_client this_type;
    struct stage { enum type {
            error = 0,
            connecting = 1, handshaking,
            idle,
            writing,
        };
    };

    ios_push_client(boost::asio::io_service& io_service, const config& conf)
        : conf_(conf), timer_(io_service)
    {
        stage_ = stage::connecting;
        timed_connect(boost::system::error_code());
    }

public:
    void write_msg();

private:
    std::string buf_;

private:
    void handle_write(boost::system::error_code const& ec, UInt id, push_mgr::s_push_msg m)
    {
        if( stage::writing != stage_ ) { 
            LOG_I<<"Illegal stage:"<<stage_;
            return; 
        }
        if ( ec ) {
            return handle_error(ec);
        }

        del_id( id );
        push_mgr::instance().del_one_msg( m, conf_.ch);
        LOG_I << id << "ok" << m.userid << m.msgid;

        stage_ = stage::idle;
        write_msg();
    }

    void handle_read(boost::system::error_code const & ec)
    {
        LOG_I << stage_ << conf_ << ec << ec.message();
        if (ec) {
            return handle_error(ec);
        }

        LOG_I << &sbuf_;
        boost::asio::async_read(ssl_->socket(), sbuf_
                , boost::asio::transfer_at_least(32)
                , boost::bind(&this_type::handle_read, this, boost::asio::placeholders::error));
    }

    void handle_error(boost::system::error_code const & ec)
    {
        LOG_I << ec << ec.message() << stage_;

        if (ec == boost::asio::error::operation_aborted) {
            return;
        }

        if (stage_ == stage::connecting) { return; }

        stage_ = stage::connecting;
        timed_connect(boost::system::error_code());
        //timer_.expires_from_now(boost::posix_time::seconds(1));
        //timer_.async_wait(boost::bind(&ios_push_client::timed_connect, this, boost::asio::placeholders::error));
    }

    void handle_handshake(const boost::system::error_code& ec)
    {
        LOG_I << ec << ec.message() << stage_;
        if (ec) {
            timer_.expires_from_now(boost::posix_time::seconds(10));
            timer_.async_wait(boost::bind(&ios_push_client::timed_connect, 
                        this, boost::asio::placeholders::error));
            return;
        }

        stage_ = stage::idle;
        write_msg();

        boost::asio::async_read(ssl_->socket(), sbuf_
                , boost::asio::transfer_at_least(32)
                , boost::bind(&this_type::handle_read, this, boost::asio::placeholders::error));
    }

    void handle_connect(boost::system::error_code const & ec)
    {
        LOG_I << ec << ec.message() << stage_;
        if (ec) {
            timer_.expires_from_now(boost::posix_time::seconds(5));
            timer_.async_wait(boost::bind(&ios_push_client::timed_connect, 
                        this, boost::asio::placeholders::error));
            return;
        }

        // stage_ = stage::handshaking;
        ssl_->socket().async_handshake(boost::asio::ssl::stream_base::client,
                boost::bind(&this_type::handle_handshake, this, boost::asio::placeholders::error));
    }

    void timed_connect(boost::system::error_code ec)
    {
        if (ec) {
            LOG_I << ec << ec.message() << stage_ << conf_.ch;
            return;
        }

        LOG_I << stage_ << "channel:" << conf_.ch << "connecting ...";

        if (stage_ != stage::connecting) { return; }

        ssl_.reset(new wrapssl(timer_.get_io_service(), conf_));
        ssl_->lowest_layer().async_connect(conf_.ep, 
                boost::bind(&this_type::handle_connect, this, boost::asio::placeholders::error));
    }

private:
    stage::type stage_;

    struct wrapssl : boost::noncopyable
    {
        explicit wrapssl(boost::asio::io_service& io_service, const config& conf)
            : context_(boost::asio::ssl::context::sslv3)
              , socket_(io_service, init_context(context_, conf))
        {}

        boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::lowest_layer_type & lowest_layer() 
        { return socket_.lowest_layer(); }

        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> & socket() { return socket_; }

    private:
        boost::asio::ssl::context context_;
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;

        static boost::asio::ssl::context& init_context(boost::asio::ssl::context& ctx, const config& conf);
    };

    boost::shared_ptr<wrapssl> ssl_;
    boost::asio::deadline_timer timer_;

    boost::asio::streambuf sbuf_;

public:
    UInt get_id();
    void del_id(UInt id);

    static std::string unhex(const std::string & hs);
    template <typename Int> static std::string& pkint(std::string & pkg, Int x);
    static std::string & pkstr(std::string & pkg, const std::string & s);
    std::string apack(std::string const & tok, int id, std::string const & aps);
};

class ios_push_mgr
{
    ios_push_mgr( boost::asio::io_service& io_service, 
            const ios_push_client::config& enter, const ios_push_client::config& rel)
        : enterprise_( io_service, enter), release_( io_service, rel )

    {
    }

    public:
    static void init( boost::asio::io_service& io_service, const boost::property_tree::ptree& ini );

    static ios_push_mgr& inst(boost::asio::io_service* io_service=NULL,
            const ios_push_client::config* enter = NULL, const ios_push_client::config* rel=NULL);


    static boost::asio::ip::tcp::endpoint resolv(boost::asio::io_service & io_service, 
            const std::string& h, const std::string& p);

    void push( push_mgr::channel ch )
    {
        if ( ch == push_mgr::IOS_RELEASE ) {
            release_.write_msg();
        } else if ( ch == push_mgr::IOS_ENTERPRISE ) {
            enterprise_.write_msg();
        } else {
            LOG_I << "Invalid channel:"<< ch;
        }
    }

    static std::string enter_bid;
    static std::string rel_bid;
    static const std::string& enterBid(){ return enter_bid;}
    static const std::string& relBid(){ return rel_bid;}
    private:
    ios_push_client enterprise_, release_;
};

#endif
