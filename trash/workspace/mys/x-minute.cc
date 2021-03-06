#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/qi_repeat.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/vector.hpp>
//#include <boost/fusion/include/io.hpp>
//#include <boost/fusion/adapted/boost_array.hpp>
//#include <boost/spirit/include/phoenix.hpp>
//namespace phoenix = boost::phoenix;
#include <boost/array.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <string>

#define ERR_EXIT(...) err_exit_(__LINE__, "%d: " __VA_ARGS__)
template <typename... Args> void err_exit_(int lin_, char const* fmt, Args... a)
{
    fprintf(stderr, fmt, lin_, a...);
    exit(127);
}
#define ERR_MSG(...) err_msg_(__LINE__, "%d: " __VA_ARGS__)
template <typename... Args> void err_msg_(int lin_, char const* fmt, Args... a)
{
    fprintf(stderr, fmt, lin_, a...);
    //fflush(stderr);
}

namespace filesystem = boost::filesystem;
namespace gregorian = boost::gregorian;
namespace fusion = boost::fusion;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
template <typename T,size_t N> using array = boost::array<T,N>;

namespace boost { namespace spirit { namespace traits
{
    template<>
    struct transform_attribute<boost::gregorian::date, fusion::vector<int, int, int>, qi::domain>
    {
        typedef fusion::vector<int, int, int> date_parts;
        typedef date_parts type;

        static date_parts pre(boost::gregorian::date) { 
            return date_parts(); 
        }
        static void post(boost::gregorian::date& d, date_parts const& v) {
            d = boost::gregorian::date(fusion::at_c<0>(v), fusion::at_c<1>(v), fusion::at_c<2>(v));
        }
        static void fail(boost::gregorian::date&) {}
    };
    //template <typename T, size_t N> struct is_container<array<T, N>, void> : mpl::false_ { };
}}}
//typedef gregorian::date::ymd_type ymd_type;
//BOOST_FUSION_ADAPT_STRUCT(ymd_type, (ymd_type::year_type,year)(ymd_type::month_type,month)(ymd_type::day_type,day))

struct Xline {
    long amount[2];
    unsigned volume;
    short val[2];;
    unsigned short tp[2];;
};

struct Xrec {
    int sec, price, vol;
};

struct Oclh { array<int,2> oc = {}, lh = {}; };

struct Av {
    long volume = 0;
    long amount = 0;
    Av& operator+=(Av const& lhs) {
        amount += lhs.amount;
        volume += lhs.volume;
        return *this;
    }
    Av operator+(Av const& lhs) {
        Av x = *this;
        return (x+=lhs);
    }
};
//BOOST_FUSION_ADAPT_STRUCT(Av, (long,volume)(long,amount))
struct Pv {
    int price = 0;
    int volume = 0;
};
//struct RecBS : Av { char bsflag; }; BOOST_FUSION_ADAPT_STRUCT(RecBS, (float,amount)(char,bsflag)(float,volume))

gregorian::date _date(std::string const& s) // ./20151221
{
    static const qi::int_parser<unsigned,10,4,4> _4digit = {};
    static const qi::int_parser<unsigned,10,2,2> _2digit = {};
    using qi::lit;
    using ascii::char_;

    unsigned y, m, d;
    auto it = s.cbegin();
    if (!qi::parse(it, s.cend()
            , -lit('/') >> *qi::omit[+(char_-'/') >> '/']
                 >> _4digit >> _2digit >> _2digit
            , y, m, d))
        ERR_EXIT("%s: not-a-date", s.c_str());
    return gregorian::date(y,m,d);
}

typedef unsigned code_t;
inline code_t make_code(int szsh, int numb) { return ((szsh<<24)|numb); }
inline int numb(code_t v_) { return v_&0x0ffffff; }
inline int szsh(code_t v_) { return (v_>>24)&0xff; }
static code_t _code(std::string const& s)
{
    static const qi::int_parser<int,10,6,6> _6digit = {};
    using qi::char_;
    using qi::lit;
    struct SZSH_ : qi::symbols<char, int> { SZSH_() { add ("SZ",0) ("SH",1); } } SZSH;

    int x, y;
    auto it = s.cbegin();
    if (!qi::parse(it, s.cend()
            , -lit('/') >> *qi::omit[+(char_-'/') >> '/']
                 >> SZSH >> _6digit // >>'.'>>qi::no_case[lit("csv")|"txt"]
            , x, y))
        ERR_EXIT("%s: not-a-code", s.c_str());
    return make_code(x,y);
}

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
//#include <boost/multi_index/random_access_index.hpp>
//#include <boost/multi_index/ordered_index.hpp>
using namespace boost::multi_index;

struct Vols : std::vector<long>
{
    int code = 0;
    long volume = 0;
};

struct Main : boost::multi_index::multi_index_container< Vols, indexed_by <
              sequenced<>
            , hashed_unique<member<Vols,int,&Vols::code>> >>
{
    iterator setdefault(int code) {
        auto & idc = this->get<1>();
        Vols tmp;
        tmp.code = code;
        return project<0>(idc.insert(tmp).first);
    }

    Main(int argc, char* const argv[])
    {}
    //~Main();
    int run(int argc, char* const argv[]);

    void step1(code_t code, std::string const& path, gregorian::date);
    template <typename F> int step2(F read, code_t code, array<int,2>&oc, array<int,2>&lh);
};

int main(int argc, char* const argv[])
{
    try {
        Main a(argc, argv);
        return a.run(argc, argv);
    } catch (std::exception const& e) {
        fprintf(stderr, "%s\n", e.what());
    }
    return 1;
}

int Main::run(int argc, char* const argv[])
{
    if (argc != 2) {
        ERR_EXIT("%s: argc>1", argv[0]);
    }
    filesystem::path path(argv[1]);

    if (!filesystem::is_directory(path)) {
        if (!filesystem::is_regular_file(path))
            ERR_EXIT("%s: is_directory|is_regular_file", argv[1]);
        if (argc != 3) {
            ERR_EXIT("%s: argc>2", argv[0]);
        }
        auto && p = path.generic_string();
        step1(_code(p), p, _date(argv[2]));
        return 0;
    }

    gregorian::date date = _date(path.generic_string());
    for (auto& di : filesystem::directory_iterator(path)) {
        if (!filesystem::is_regular_file(di.path()))
            continue;
        auto && p = di.path().generic_string();
        step1(_code(p), p, date);
    }
    return 0;
}

void Main::step1(code_t code, std::string const& path, gregorian::date)
{
    //if (code.numb()!=807) return; // Debug
    if (FILE* fp = fopen(path.c_str(), "r")) {
        std::unique_ptr<FILE,decltype(&fclose)> xclose(fp, fclose);
        auto xcsv = [fp,&path](Xrec& xr)/*(int& sec, int& price, int& vol)*/ {
            //static const qi::int_parser<int, 10, 2, 2> _2digit = {};
            //qi::rule<char*, int> rule_sec = _2digit[qi::_val=3600*qi::_1] >> _2digit[qi::_val+=60*qi::_1] >> _2digit[qi::_val+=qi::_1] ;
            using qi::int_;
            using qi::float_;
            using qi::char_;
            for (;;) {
                char linebuf[256];
                if (!fgets(linebuf, sizeof(linebuf), fp)) {
                    if (!feof(fp))
                        ERR_EXIT("fgets");
                    break;
                }
                float fprice;
                int sec;
                signed char c;

                char* pos = linebuf;
                if (!qi::phrase_parse(pos, &linebuf[sizeof(linebuf)]
                            , int_ >> float_ >> char_ >> int_, ',', sec, fprice, c, xr.vol) /*&& pos == end*/) {
                    ERR_MSG("qi::parse %s %s", path.c_str(), pos);
                    continue;
                }
                xr.sec = sec/10000*3600 + sec/100%100*60 + sec%100;
                xr.price = int(fprice*100);
                return int('J') - c; //sec; //60*(sec/10000) + sec/100;
            }
            return 0;
        };
        auto xtxt = [fp,&path](Xrec& xr)/*(int& sec, int& price, int& vol)*/ {
            //static const qi::int_parser<int, 10, 2, 2> _2digit = {};
            //qi::rule<char*, int> rule_sec = _2digit[qi::_val=3600*qi::_1] >> _2digit[qi::_val+=60*qi::_1] >> _2digit[qi::_val+=qi::_1] ;
            using qi::int_;
            using qi::char_;
            for (;;) {
                char linebuf[256];
                if (!fgets(linebuf, sizeof(linebuf), fp)) {
                    if (!feof(fp))
                        ERR_EXIT("fgets");
                    break; //return 0;
                }
                int sec, svol;
                char* pos = linebuf;
                if (!qi::phrase_parse(pos, &linebuf[sizeof(linebuf)]
                            , int_ >> int_ >> int_, qi::space, sec, xr.price, svol) /*&& pos == end*/) {
                    ERR_MSG("qi::parse %s %s", path.c_str(), pos);
                    continue;
                }
                xr.sec = sec/10000*3600 + sec/100%100*60 + sec%100;
                xr.vol = abs(svol);
                return svol;
            }
            return 0;
        };
        //std::unordered_map<int,Oclh> oclh;
        Oclh oclh = {};
        if (boost::algorithm::iends_with(path, ".txt")) // if (path.extension() == ".txt")
            step2(xtxt, code, oclh.oc, oclh.lh);
        else
            step2(xcsv, code, oclh.oc, oclh.lh);
    }
}

template <typename F> int Main::step2(F read, code_t code, array<int,2>&oc, array<int,2>&lh)
{
    static const auto Idx = [](int m) {
        return (m < 60*12+30)
            ? std::min(std::max(m-(60*9+30), 0), 60*2)
            : std::min(std::max(m-(60*13  ), 0), 60*2) + (60*2+1);
    };
    std::vector<array<Av,2>> vols(60*4+2);
    Xrec xr; //int sec, price, vol;
    while (int bsf = read(xr)) {
        Av& av = vols[Idx(xr.sec/60)][bsf>0]; // += av; //vols[Idx(xt/100)][bsf>0] += av;
        av.amount += xr.price * xr.vol /100;
        av.volume += xr.vol;

        //if (oc[0] == 0)
        //    oc[0] = oc[1] = lh[0] = lh[1] = price;
        //oc[1] = price;
        //if (price < lh[0])
        //    lh[0] = price;
        //else if (price > lh[1])
        //    lh[1] = price;
    }

    fprintf(stdout, "%06d %d", numb(code), szsh(code));
    for (auto& v : vols) {
        fprintf(stdout, "\t%ld %ld %ld %ld"
                , v[0].volume, v[0].amount, v[1].volume, v[1].amount);
    }
    fprintf(stdout, "\n");

    return int(vols.size());
}

