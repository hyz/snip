#include <fstream>
#include <iostream>

// include headers that implement a archive in simple text format
#include <boost/asio/streambuf.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/range/iterator_range.hpp>

#include <boost/serialization/vector.hpp>

#define ARFLAG (boost::archive::no_header)

/////////////////////////////////////////////////////////////
// gps coordinate
//
// illustrates serialization for a simple type
//
class gps_position
{
private:
    friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & degrees;
        ar & minutes;
        ar & seconds;
    }
    friend std::ostream& operator<<(std::ostream& ar, gps_position const& p)
    {
        return ar << p.degrees <<":"<< p.minutes <<":"<< p.seconds;
    }
    int64_t degrees;
    int minutes;
    float seconds;
public:
    gps_position(){};
    gps_position(int d, int m, float s) :
        degrees(d), minutes(m), seconds(s)
    {}
};

void fill(boost::asio::streambuf& sbuf, gps_position const& g)
{
    std::ostream outs(&sbuf);
    boost::archive::binary_oarchive oa(outs, ARFLAG);
    oa << g;
}

struct Vec : std::vector<gps_position>
{
    int tag;
};

int main(int argc, char* const argv[])
{
    // create class instance
    std::vector<gps_position> vec;
    vec.push_back( gps_position(35, 59, 24.567f) );
    vec.push_back( gps_position(36, 59, 24.567f) );

    // save data to archive
    if (argc == 1) {
        std::cerr << boost::make_iterator_range(vec)
            << "\n asio::streambuf " << sizeof(boost::asio::streambuf)
            << "\n iarchive " << sizeof(boost::archive::binary_iarchive)
            << "\n oarchive " << sizeof(boost::archive::binary_oarchive)
            << "\n";

        boost::asio::streambuf sbuf;
        {
            std::ostream outs(&sbuf);
            boost::archive::binary_oarchive oa(outs, ARFLAG);
            oa << vec;
        }
        std::cout << &sbuf;
    } else {
        boost::asio::streambuf sbuf;
        {
            std::filebuf fbuf;
            fbuf.open(argv[1], std::ios::in);

            std::ostream outs(&sbuf);
            outs << &fbuf;
            // std::ifstream ifs(argv[1]);
        }
        {
            std::vector<gps_position> newvec;
            boost::archive::binary_iarchive ia(sbuf, ARFLAG);
            ia >> newvec;
            std::cout << boost::make_iterator_range(newvec) << "\n";
        }
    }
    return 0;
}

int main_2(int argc, char* const argv[])
{
    // create class instance
    const gps_position g(35, 59, 24.567f);

    // save data to archive
    if (argc == 1) {
        boost::asio::streambuf sbuf;
        fill(sbuf, g);
        std::cout << &sbuf;
        std::cerr << g << "\n";
    } else {
        boost::asio::streambuf sbuf;
        {
            std::filebuf fbuf;
            fbuf.open(argv[1], std::ios::in);

            std::ostream outs(&sbuf);
            outs << &fbuf;
            // std::ifstream ifs(argv[1]);
        }
        {
            gps_position newg;
            boost::archive::binary_iarchive ia(sbuf, ARFLAG);
            ia >> newg;
            std::cout << newg << "\n";
        }
    }
    return 0;
}

int main_x(int argc, char* const argv[])
{
    // create class instance
    const gps_position g(35, 59, 24.567f);

    // save data to archive
    if (argc == 1) {
        boost::archive::binary_oarchive oa(std::cout, ARFLAG);
        // write class instance to archive
        oa << g;
        std::cerr << g << "\n";
    	// archive and stream closed when destructors are called
    } else {
        // ... some time later restore the class instance to its orginal state
        gps_position newg;
        {
            // create and open an archive for input
            std::ifstream ifs(argv[1]);
            boost::archive::binary_iarchive ia(ifs, ARFLAG);
            // read class state from archive
            ia >> newg;
            // archive and stream closed when destructors are called
        }
        std::cout << newg << "\n";
    }
    return 0;
}


