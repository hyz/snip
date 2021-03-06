#include <stdio.h>
#include <array>
#include <vector>
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <string>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_repeat.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/vector.hpp>
//#include <boost/fusion/include/io.hpp>
//#include <boost/fusion/adapted/boost_array.hpp>
//#include <boost/spirit/include/phoenix.hpp>
//#include <boost/spirit/include/phoenix_core.hpp>
//#include <boost/spirit/include/phoenix_operator.hpp>
//#include <boost/spirit/include/phoenix_object.hpp>
//#include <boost/spirit/include/phoenix_core.hpp>
//#include <boost/spirit/include/phoenix_operator.hpp>
//#include <boost/container/static_vector.hpp>
#include <boost/function_output_iterator.hpp>
#include <boost/range/iterator_range.hpp>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
//#include <boost/filesystem/fstream.hpp>
//#include <boost/format.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
//#include <boost/multi_index/random_access_index.hpp>
//#include <boost/multi_index/ordered_index.hpp>

#define ERR_EXIT(...) err_exit_(__LINE__, "%d: " __VA_ARGS__)
template <typename... Args> void err_exit_(int lin_, char const* fmt, Args... a) {
    fprintf(stderr, fmt, lin_, a...);
    exit(127);
}
#define ERR_MSG(...) err_msg_(__LINE__, "%d: " __VA_ARGS__)
template <typename... Args> void err_msg_(int lin_, char const* fmt, Args... a) {
    fprintf(stderr, fmt, lin_, a...);
    //fflush(stderr);
}


//namespace phoenix = boost::phoenix;
namespace filesystem = boost::filesystem;
namespace gregorian = boost::gregorian;
namespace fusion = boost::fusion;
namespace qi = boost::spirit::qi;
//namespace ascii = boost::spirit::ascii;
template <typename T,size_t N> using array = std::array<T,N>;

using namespace boost::multi_index;

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

struct Av {
    long volume = 0;
    long amount = 0;

    Av& operator+=(Av const& lhs) {
        amount += lhs.amount;
        volume += lhs.volume;
        return *this;
    }
    Av operator+(Av const& lhs) const {
        Av x = *this;
        return (x+=lhs);
    }
    Av& operator-=(Av const& lhs) {
        amount -= lhs.amount;
        volume -= lhs.volume;
        return *this;
    }
    Av operator-(Av const& lhs) const {
        Av x = *this;
        return (x-=lhs);
    }
};
BOOST_FUSION_ADAPT_STRUCT(Av, (long,volume)(long,amount))

typedef fusion::vector<Av,Av> Avsb;
auto First  = [](auto&& x) -> auto& { return fusion::at_c<0>(x); };
auto Second = [](auto&& x) -> auto& { return fusion::at_c<1>(x); };

static const auto Sum = [](Avsb const& avsb) { return fusion::at_c<0>(avsb) + fusion::at_c<1>(avsb); };
static const auto Plus = [](Avsb const& x, Avsb const& y) {
    Av const a = fusion::at_c<0>(x) + fusion::at_c<0>(y);
    Av const b = fusion::at_c<1>(x) + fusion::at_c<1>(y);
    return fusion::make_vector(a, b);
};
//[](auto&& x) -> decltype(fusion::at_c<0>(x))& { return fusion::at_c<0>(x); };

static auto Lohi = [](auto& rng) {
    auto cheap = [](auto&x, auto&y) {
        Av a = Sum(x);
        Av b = Sum(y);
        return (a.amount/a.volume) < (b.amount/b.volume);
    };
    return std::minmax_element(std::begin(rng), std::end(rng), cheap);
};

inline int Price(Av const& v) {
    if (!v.volume)
        return 1;
    return int(v.amount*100/v.volume);
};
inline int Price(Avsb const& x) { return Price(Sum(x)); };

//struct RecBS : Av { char bsflag; }; BOOST_FUSION_ADAPT_STRUCT(RecBS, (float,amount)(char,bsflag)(float,volume))

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

gregorian::date _date(std::string const& s) // ./20151221
{
    static const qi::int_parser<unsigned,10,4,4> _4digit = {};
    static const qi::int_parser<unsigned,10,2,2> _2digit = {};
    using qi::lit;
    using qi::char_;

    unsigned y, m, d;
    auto it = s.cbegin();
    if (!qi::parse(it, s.cend()
            , -lit('/') >> *qi::omit[+(char_-'/') >> '/']
                 >> _4digit >> _2digit >> _2digit
            , y, m, d))
        ERR_EXIT("%s: not-a-date", s.c_str());
    return gregorian::date(y,m,d);
}

struct SInfo
{
    long gbx, gbtotal;
    int eov;
};
BOOST_FUSION_ADAPT_STRUCT(SInfo, (long,gbx)(long,gbtotal)(int,eov))

struct Elem : SInfo
{
    int code = 0;

    std::vector<Avsb> all = {};
    std::vector<Avsb> vless = {};
    std::vector<Avsb> vmass = {};
    std::vector<Avsb> voths = {};

    Avsb sum = {}; //Av av = {}; //long volume = 0; long amount = 0;
    array<int,2> lohi = {};

    int n_day() const { return int(all.size()); }
};

struct Main : boost::multi_index::multi_index_container< Elem, indexed_by <
              sequenced<>
            , hashed_unique<member<Elem,int,&Elem::code>> >>
{
    Main(int argc, char* const argv[]);
    int run(int argc, char* const argv[]);

    gregorian::date date;
    struct init_;
};

struct Main::init_ : std::unordered_map<int,SInfo> , boost::noncopyable
{
    Main* m_ = 0;
    Elem tmp_ = {};
    init_(Main* p, int argc, char* const argv[]);

    void loadsi(char const* dir, char const* fn);
    Elem* address(int code);
    void prep(char* fn, int xbeg, int xend); //(filesystem::path const& path);
    template <typename F> void proc1 (F read);
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

void Main::init_::loadsi(char const* dir, char const* fn)
{
    char ph[128];
    snprintf(ph,sizeof(ph), "%s/%s", dir, fn);
    if (FILE* fp = fopen(ph, "r")) {
        std::unique_ptr<FILE,decltype(&fclose)> xclose(fp, fclose);
        using qi::long_; //using qi::_val; using qi::_1;
        using qi::int_;
        qi::rule<char*, SInfo(), qi::space_type> R_
            = long_ >> long_ >> qi::omit[long_]>>qi::omit[long_] >> int_;

        char linebuf[1024];
        while (fgets(linebuf, sizeof(linebuf), fp)) {
            int szsh=0, code;
            SInfo si = {};
            char* pos = linebuf;
            if (!qi::phrase_parse(pos, &linebuf[sizeof(linebuf)]
                        , int_ >> int_ >> R_
                        , qi::space, code, szsh, si)) {
                ERR_EXIT("qi::parse: %s %s", fn, pos);
            }
            if (si.gbx > 0)
                this->emplace(make_code(szsh,code), si);
            else
                fprintf(stderr, "%d\n", code);
        }
    } else
        ERR_EXIT("fopen: %s", fn);
}

Elem* Main::init_::address(int code) //-> Main::iterator
{
    auto itr = find(code);
    if (itr == end()) {
        return 0;
    }
    auto & idc = m_->get<1>();

    tmp_.code = code;
    auto p = idc.insert(tmp_);
    Elem& el = const_cast<Elem&>(*p.first);
    if (p.second) {
        SInfo& si = el;
        si = itr->second;
    }
    return &el;
}

Main::init_::init_(Main* p, int argc, char* const argv[])
    : m_(p)
{
    if (argc < 4) {
        ERR_EXIT("%s argc: %d", argv[0], argc);
    }
    loadsi(getenv("HOME"), "._sinfo");

    //if (filesystem::is_directory(argv[1])) {
    //    for (auto& di : filesystem::directory_iterator(argv[1])) {
    //        if (!filesystem::is_regular_file(di.path()))
    //            continue;
    //        auto & p = di.path();
    //        prep( p);
    //        m_->date = std::min(m_->date, _date(p.generic_string()));
    //    }
    //} else for (int i=1; i<argc; ++i) {
    //    if (!filesystem::is_regular_file(argv[i]))
    //        continue; // ERR_EXIT("%s: is_directory|is_regular_file", argv[i]);
    //    prep( argv[i]);
    //    m_->date = std::min(m_->date, _date(argv[i]));
    //}
    //m_->date = _date(argv[1]);

    static const auto erase = [](char* s0, char x) {
        char* e=s0+strlen(s0);
        for (char *p,*s=s0; (p=strchr(s, x)); --e)
            s = (char*)memmove(p, p+1, strlen(p+1));
        *e = '\0';
        return s0;
    };
    static const auto index = [](int xt) {
        int m = xt/100*60 + xt%100;
        return (m < 60*13-30)
            ? std::min(std::max(m-(60*9+30), 0), 60*2)
            : 60*2+1+std::min(std::max(m-60*13, 0), 60*4);
    };
    prep(argv[1], index(atoi(erase(argv[2],':'))), index(atoi(erase(argv[3],':'))));
}

void Main::init_::prep(char* fn, int xbeg, int xend)
{
    ERR_MSG("info: %d,%d\n", xbeg, xend);
    if (FILE* fp = fopen(fn, "r")) {
        std::unique_ptr<FILE,decltype(&fclose)> xclose(fp, fclose);
        typedef boost::iterator_range<std::vector<Avsb>::iterator> rng_t;
        auto reader = [fp,xbeg,xend](rng_t& rng, std::vector<Avsb>& vec) {
            vec.reserve(60*4+2); //using qi::long_; //using qi::_val; using qi::_1;

            qi::rule<char*, Av(), qi::space_type> R_Av = qi::long_ >> qi::long_;
            //qi::rule<char*, Avsb(), qi::space_type> R_ =  R_Av >> R_Av;

            char linebuf[(60*4+8)*64];
            if (!fgets(linebuf, sizeof(linebuf), fp)) {
                if (!feof(fp))
                    ERR_EXIT("fgets");
                return 0u;
            }
            int szsh=0, code = 0;
            char*const end = &linebuf[sizeof(linebuf)];
            char* pos = linebuf;
            if (qi::phrase_parse(pos,end, qi::int_ >> qi::int_, qi::space, code, szsh)) {
                int i=0, xb=0, xe=0;
                Avsb avsb;
                while (qi::phrase_parse(pos,end, R_Av>>R_Av, qi::space, avsb)) {
                    if (Sum(avsb).volume > 0) {
                        vec.push_back(avsb);
                    }
                    if (i == xbeg)
                        xb =xe= vec.size()-1;
                    else if (i == xend)
                        xe = vec.size();
                    ++i;
                }
                rng = boost::make_iterator_range(vec.begin()+xb, vec.begin()+xe);
            } else
                ERR_EXIT("qi::parse: %s", pos);
            return make_code(szsh,code);
        };
        proc1(reader);
    }
}

struct XClear { template<typename T> void operator()(T*v)const{v->clear();} };

constexpr int Wn=10000;
constexpr int Yi=Wn * Wn;

template <typename F> void Main::init_::proc1(F read)
{
    auto Chr = [](auto& rng) {
        auto v = Lohi(rng);
        auto it = (v.first < v.second) ? v.first : v.second;
        auto last = std::prev(std::end(rng));
        return (Price(*last)-Price(*it))*1000/Price(*it);
    };
    //Avsb avsb[2] = {};
    std::vector<Avsb> vec;
    boost::iterator_range<std::vector<Avsb>::iterator> rng;
    while (code_t code = read(rng, vec)) {
        std::unique_ptr<decltype(vec),XClear> xclear(&vec);
        if (vec.size() < 100 || boost::size(rng)<10)
            continue;
        Elem* vss = this->address(code);
        if (!vss) {
            ERR_MSG("%d address-fail\n", code);
            continue;
        }

        Avsb avsb0 = std::accumulate(std::begin(vec), std::end(vec), Avsb{}, Plus);
        //avsb[0] = Plus(avsb[0], avsb0);
        Avsb avsb1 = std::accumulate(std::begin(rng), std::end(rng), Avsb{}, Plus);
        //avsb[1] = Plus(avsb[1], avsb1);

        Av b0 = Second(avsb0) - First(avsb0);
        Av b1 = Second(avsb1) - First(avsb1);
        Av s0 = Second(avsb0) + First(avsb0);
        Av s1 = Second(avsb1) + First(avsb1);
        long lsz = vss->gbx*Price(avsb0)/100;

        printf("%06d", numb(code));

        printf("\t%6.2f %5.2f %03ld", lsz/double(Yi), s0.amount/double(Yi), 1000*s0.amount/lsz);

        printf("\t% 5.2f % 5.2f", b1.amount/double(Yi), b0.amount/double(Yi));
        printf("\t% .3ld % .3ld % .3ld", b1.amount*1000/s0.amount, b1.amount*1000/s1.amount, b0.amount*1000/s0.amount);
        printf("\t% .3d % .3d", Chr(rng), Chr(vec));
        printf("\n");
    }

    //for (auto& _sum : avsb) {
    //    Av buy = Second(_sum);
    //    Av sell = First(_sum);
    //    Av total = buy + sell;
    //    Av b = buy - sell;
    //    fprintf(stderr,"\t% 8.2f % 8.2f"
    //            , total.amount/double(Yi), b.amount/double(Yi));
    //    //fprintf(stderr,"\t%6.2f %6.2f %03ld" , total.volume/double(Y_), b.volume/double(Y_), labs(b.volume*100)/(total.volume/10));
    //}
    //fprintf(stderr, "\n");

    //gregorian::date::ymd_type ymd = date.year_month_day();
    //fprintf(stderr,"%04d%02d%02d", int(ymd.year), int(ymd.month), int(ymd.day));
}

Main::Main(int argc, char* const argv[])
    : date(gregorian::day_clock::local_day())
{
    init_ ini(this, argc, argv);
    (void)ini;
}

int Main::run(int argc, char* const argv[])
{
    return 0;
}

