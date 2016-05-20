#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <array>
#include <vector>
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <string>
#include <boost/signals2/detail/null_output_iterator.hpp>
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
#include <boost/format.hpp>

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
    fputc('\n', stderr);
    //fflush(stderr);
}
template <typename Iter>
static boost::iterator_range<Iter> c_trim(Iter h, Iter end, const char* cs)
{
    while (end > h && strchr(cs, *(end-1)))
        --end;
    while (h < end && strchr(cs, *h))
        ++h;
    return boost::make_iterator_range(h,end);
}
static boost::iterator_range<char*> c_trim(char* s, const char* cs) {
    return c_trim(s, s + strlen(s), cs);
}

template <typename... V> struct Path_join ;
template <typename T, typename... V>
struct Path_join<T,V...> { static void concat(char*beg,char*end, T const& a, V const&... v) {
    Path_join<typename std::decay<T>::type>::concat(beg,end, a);
    Path_join<typename std::decay<V>::type...>::concat(beg,end, v...);
}};
template <> struct Path_join<char*> { static void concat(char*beg,char*end, char const* src) {
    char* p = beg + strlen(beg); // char* src = a;
    if (p != beg && p != end)
        *p++ = '/';
    while (p != end && (*p++ = *src++))
        ;
    c_trim(beg, "/\\").back() = '\0';
}}; 
template <> struct Path_join<char const*> { static void concat(char*beg,char*end, char const* src) {
    Path_join<char*>::concat(beg,end, src);
}};
template <> struct Path_join<std::string> { static void concat(char*beg,char*end, std::string const& src) {
    Path_join<char*>::concat(beg,end, src.c_str());
}};
template <> struct Path_join<boost::format> { static void concat(char*beg,char*end, boost::format const& src) {
    auto s = str(src);
    Path_join<char*>::concat(beg,end, s.c_str());
}};

template <int L=128>
struct joinp : std::array<char,L> {
    template <typename ...V>
    joinp(V&&... v) {
        this->front() = this->back() = '\0';
        char* end = &this->back(); //+1;
        char* beg = c_str();
        Path_join<typename std::decay<V>::type...>::concat(beg,end, v...);
    }
    char* c_str() const { return const_cast<char*>(this->data()); }
};

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
    Av& operator/=(int x) {
        volume /= x;
        amount /= x;
        return *this;
    }
    Av operator/(int x) const {
        Av tmp = *this;
        return (tmp /= x);
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

template <typename It, typename Iter, typename Cmp> // boost::make_function_output_iterator
auto Ma(unsigned n, It it, It end, Iter iter, Cmp&& cmp) // -> array<decltype(*it),2>
{
    typename It::value_type a{}; // decltype(*it) a{};
    std::vector<It> ar(n);
    unsigned x=0;
    for (; x < ar.size() && it != end; ++x, ++it) {
        a += *it;
        ar[x] = it;
    }
    auto b = a/ar.size();
    array<decltype(b),2> ret={{b,b}};

    if (x == ar.size()) {
        *iter++ = b;

        for (; it != end; ++it) {
            unsigned i = x++ % n;
            a -= *ar[i];
            a += *it;
            ar[i] = it;
            *iter++ = b = a/n;
            if (cmp(b, ret[0])) {
                ret[0] = b;
            } else if (!cmp(b, ret[1])) {
                ret[1] = b;
            }
        }
    }
    return ret;
}

struct SInfo {
    long gbx, gbtotal;
    int eov;
};
BOOST_FUSION_ADAPT_STRUCT(SInfo, (long,gbx)(long,gbtotal)(int,eov))

struct Unit : Av {
    array<int,2> oc = {}, lohi = {};
};

struct Elem : SInfo, Unit, std::vector<Unit> {
    int code = 0;
    //void extend(Elem const& o) {}
};

struct Main : boost::multi_index::multi_index_container< Elem, indexed_by <
              sequenced<>
            , hashed_unique<member<Elem,int,&Elem::code>> >> // , boost::noncopyable
{
    Main(int argc, char* const argv[]);
    int run(int argc, char* const argv[]);

    gregorian::date date;
    struct initializer;
};

struct Main::initializer : std::unordered_map<int,SInfo>, boost::noncopyable
{
    Main* a_ = 0;
    int igntail_ = 0;
    initializer(Main* p, int argc, char* const argv[]);

    void loadsi(char const* dir, char const* fn);
    Elem* add(int code, Elem&& d);
    void prep(char const* path);
    template <typename F> void fun(F read);
};

int main(int argc, char* const argv[])
{
    BOOST_STATIC_ASSERT(sizeof(long)==8);
    try {
        Main a(argc, argv);
        return a.run(argc, argv);
    } catch (std::exception const& e) {
        fprintf(stderr, "%s\n", e.what());
    }
    return 1;
}

void Main::initializer::loadsi(char const* dir, char const* fn)
{
    if (FILE* fp = fopen(joinp<>(dir,fn).c_str(), "r")) {
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
                ERR_MSG("%d gbx %ld", code, si.gbx);
        }
    } else
        ERR_EXIT("fopen: %s", fn);
}

Elem* Main::initializer::add(int code, Elem&& el)
{
    //auto & idc = a_->get<1>();
    //auto it = idc.find(code);
    //if (it != idc.end()) {
    //    ERR_EXIT("add: %d", numb(code));
    //}
    auto si = this->find(code);
    if (si == this->end()) {
        ERR_MSG("%d: find", numb(code));
        return 0;
    }
    static_cast<SInfo&>(el) = si->second;
    el.code = code;
    auto p = a_->push_back( std::move(el) );
    if (!p.second)
        ERR_EXIT("%d: push_back", numb(code));
    return &const_cast<Elem&>(*p.first);
}

Main::initializer::initializer(Main* p, int argc, char* const argv[])
    : a_(p)
{
    if (argc < 2) {
        ERR_EXIT("%s argc: %d", argv[0], argc);
    }
    
    loadsi(getenv("HOME"), "_/_sinfo");

    while ( getopt(argc, argv, "b:") != -1) {
        igntail_ = atoi(optarg);
    }

    if (filesystem::is_directory(argv[optind])) {
        for (auto& di : filesystem::directory_iterator(argv[optind])) {
            if (!filesystem::is_regular_file(di.path()))
                continue;
            auto & p = di.path();
            prep( p.generic_string().c_str() );
            //a_->date = std::min(a_->date, _date(p.generic_string()));
        }
    } else for (int i=optind; i<argc; ++i) {
        if (!filesystem::is_regular_file(argv[i]))
            continue; // ERR_EXIT("%s: is_directory|is_regular_file", argv[i]);
        prep( argv[i] );
        //a_->date = std::min(a_->date, _date(argv[i]));
    }
}

void Main::initializer::prep(char const* path)
{
    if (FILE* fp = fopen(path, "r")) {
        auto read = [fp](std::vector<Unit>& vec, Unit& sa) {
            // vec.reserve(60*4+2); //using qi::long_; //using qi::_val; using qi::_1;
            qi::rule<char*, Av(), qi::space_type> R_Av = qi::long_ >> qi::long_;
            //qi::rule<char*, Avsb(), qi::space_type> R_ =  R_Av >> R_Av;

            char linebuf[1024*16]; //[(60*4+15)*16+256];
            if (!fgets(linebuf, sizeof(linebuf), fp)) {
                if (!feof(fp))
                    ERR_EXIT("feof");
                return 0u;
            }
            int szsh=0, code = 0;
            char*const end = &linebuf[sizeof(linebuf)];
            char* pos = linebuf;
            if (qi::phrase_parse(pos,end, qi::int_ >> qi::int_, qi::space, code, szsh)) {
                Unit un = {};
                Av & av = un;

                using qi::int_;
                while (qi::phrase_parse(pos,end, R_Av>>int_>>int_>>int_>>int_, qi::space
                            , av, un.oc[0], un.oc[1], un.lohi[0], un.lohi[1])) {
                    if (av.volume) {
                        if (vec.empty()) {
                            sa = un;
                        } else { 
                            if (sa.lohi[0] > un.lohi[0])
                                sa.lohi[0] = un.lohi[0];
                            if (sa.lohi[1] < un.lohi[1])
                                sa.lohi[1] = un.lohi[1];
                            sa.oc[1] = un.oc[1];
                            static_cast<Av&>(sa) += av;
                        }
                        vec.push_back(un);
                    } else {
                        ERR_MSG("%d vol %ld", code, av.volume);
                    }
                }
            } else
                ERR_EXIT("qi::parse: %s", pos);
            return make_code(szsh,code);
        };
        std::unique_ptr<FILE,decltype(&fclose)> xclose(fp, fclose);
        fun(read);
    } else {
        ERR_EXIT("fopen: %s", path);
    }
}

template <typename F> void Main::initializer::fun(F read)
{
    Elem el = {}; //Unit sa = {};
    while (code_t code = read(el, el)) {
        if (el.size() < 3) {
            ERR_MSG("%d :size<3", numb(code));
            continue;
        }
        el.resize(el.size() - igntail_);
        if (!this->add(code, std::move(el))) {
            ERR_MSG("%d :add-fail", numb(code));
        }
    }
}

Main::Main(int argc, char* const argv[])
    : date(gregorian::day_clock::local_day())
{
    initializer ini(this, argc, argv);
    (void)ini;
}

int Main::run(int argc, char* const argv[])
{
    constexpr int Wn=10000;
    constexpr int Yi=Wn * Wn;

    for (Elem const & el : *this) {
        Av av = std::accumulate(el.rbegin()+1, el.rend(), Av{});
        auto last = el.end()-1;
        auto lasp = last-1;;
        long volx = 0;
        long vola = (av.volume+last->volume)/el.size();

        for (auto& x : el) {
            volx += ::labs(x.volume - vola);
        }
        volx /= el.size();

        auto vlh = Ma(2, el.rbegin()+1, el.rend(), boost::signals2::detail::null_output_iterator()
                , [](Av const& l, Av const& r){ return l.volume<r.volume; });
                //, boost::make_function_output_iterator( [&last](Av const& a) { if (a.volume < last->volume) last->volume; } )

        printf("%06d", numb(el.code));
        {
            long lsz = el.gbx*el.oc[1]/100;
            printf("\t%6.2f %5.2f %.3ld", lsz/double(Yi), last->amount/double(Yi), 1000*last->amount/lsz);
            printf(" %.3ld", 1000*volx/vola);
        } {
            printf("\t%.3ld %.3ld %.3ld %.3ld"
                    , 100*lasp->volume/last->volume
                    , 100*vlh[0].volume/last->volume, 100*vola/last->volume, 100*vlh[1].volume/last->volume);
        } {
            auto Chr = [](int b, int lastv) { return 1000*(lastv-b)/b; };
            printf("\t% .3d % .3d %.3d", Chr(lasp->oc[1],last->oc[1]), Chr(el.oc[0],el.oc[1]), Chr(el.lohi[0],el.lohi[1]));
        }
        printf("\t%d\n", (int)el.size());
    }
    return 0;
}

