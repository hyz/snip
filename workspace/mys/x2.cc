#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/qi_repeat.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/adapted/boost_array.hpp>
//#include <boost/spirit/include/phoenix.hpp>
//namespace phoenix = boost::phoenix;
#include <boost/array.hpp>
#include <boost/container/static_vector.hpp>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <string>

namespace filesystem = boost::filesystem;
namespace gregorian = boost::gregorian;
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

int main(int argc, char* const argv[])
{
    try {
        int Main(int argc, char* const argv[]);
        return Main(argc, argv);
    } catch (std::exception const& e) {
        std::cerr << e.what() << '\n';
    }
    return 1;
}

struct Av {
    float amount;
    float volume;
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
BOOST_FUSION_ADAPT_STRUCT(Av, (float,amount)(float,volume))
struct RecBS : Av {
    //unsigned sec;
    char bsflag;
};
BOOST_FUSION_ADAPT_STRUCT(RecBS, (float,amount)(char,bsflag)(float,volume))

gregorian::date to_date(std::string const& s) // xdetail/20151221/SZ002280.csv
{
    static const qi::int_parser<unsigned, 10, 4, 4> _4digit = {};
    static const qi::int_parser<unsigned, 10, 2, 2> _2digit = {};
    //using boost::phoenix::ref; using qi::_1;
    using qi::omit;
    using ascii::digit;
    using qi::lit;
    using ascii::char_;

    unsigned y, m, d;
    auto it = s.cbegin();
    return qi::parse(it, s.cend()
            , omit[-lit('/') >> *(+(char_-'/') >> '/')]
                  >> _4digit >> _2digit >> _2digit
            , y, m, d)
        ? gregorian::date(y,m,d) : gregorian::date();
}
int to_code(std::string const& s)
{
    static const qi::int_parser<int, 10, 6, 6> _6digit = {};
    using ascii::char_;
    using qi::lit;

    int y;
    auto it = s.cbegin();
    return qi::parse(it, s.cend(), lit('S') >> (lit('Z')|'H') >> _6digit >>'.'>>ascii::no_case["csv"], y)
        ? y : 0;
}

#include <iostream>

int xsum(FILE* ifs, int code, gregorian::date const& date, FILE* fp)
{
    typedef char* iterator_type; // typedef std::string::const_iterator iterator_type;

    static const qi::int_parser<unsigned, 10, 2, 2> _2digit = {};
    qi::rule<iterator_type, unsigned> rule_sec
        = _2digit[qi::_val=3600*qi::_1] >> _2digit[qi::_val+=60*qi::_1] >> _2digit[qi::_val+=qi::_1];
    using qi::int_;
    using qi::float_;
    using ascii::char_;
    qi::rule<iterator_type, RecBS> rule_recbs = (float_ >> ',' >> char_ >> ',' >> float_);

    array<Av,20> avmax20[2] = {};
    Av avbuysell[2] = {};
    //array<Av,12> av12p[2] = {};
    float ochl[4] = {};

    struct Vols : std::vector<int>/*boost::container::static_vector<int,60*5>*/ {
        unsigned minutex = std::numeric_limits<unsigned>::max();
    } vols;
    vols.reserve(60*5);

    struct AVM1 : Av { unsigned minutex; } avminute1[2] = {};

    char linebuf[128]; // std::string str; // 092500,48.40,B,200
    while (fgets(linebuf, sizeof(linebuf), ifs)) {
        RecBS rec = {};
        unsigned sec = 0;
        char* pos = linebuf; //str.cbegin();
        if (qi::parse(pos, &linebuf[128], rule_sec >>','>> rule_recbs, sec, rec) /*&& pos == end*/) {
            float val = rec.amount;
            rec.amount = val * rec.volume;
            bool bsflag = (rec.bsflag == 'B');

            if (ochl[0] < 0.001)
                ochl[0] = ochl[1] = ochl[2] = val;
            if (val < ochl[1]) {
                ochl[1] = val;
            } else if (val > ochl[2]) {
                ochl[2] = val;
            }
            ochl[3] = val;

            {
                auto& w = avbuysell[bsflag];
                w.amount += rec.amount;
                w.volume += rec.volume;
            }{
                //auto& bs12p = av12p[bsflag];
                //auto& w = bs12p[sec < 3600*13 ? abs(sec - 60*(60*9+30))/20%6 : 6+(abs(sec - 3600*13)/20%6)];
                //w.amount += rec.amount;
                //w.volume += rec.volume;
            }

            unsigned minx = sec/60; //minute index

            auto& m1 = avminute1[bsflag];
            if (minx == m1.minutex) {
                m1.amount += rec.amount;
                m1.volume += rec.volume;

            } else {
                if (vols.minutex == m1.minutex) {
                    vols.back() += m1.volume;
                } else {
                    vols.push_back(m1.volume);
                    vols.minutex = m1.minutex;
                }

                auto& mx = avmax20[bsflag];

                if (m1.volume > mx[0].volume) {
                    Av* p = std::lower_bound(&mx[0], &mx[0] + mx.size()
                            , m1, [](Av const& elem, Av const& i){return elem.volume<i.volume;});
                    std::memmove(&mx[0], &mx[1], sizeof(mx[0])*(p - &mx[0]));
                    *p = m1;
                }
                m1.minutex = minx;
                m1.amount = rec.amount;
                m1.volume = rec.volume;
            }

        } else {
            std::cerr << "Parsing failed\n";
            return 2;
        }
    }

    gregorian::date::ymd_type ymd = date.year_month_day();
    fprintf(fp, "%06d\t%u%02u%02u", code, unsigned(ymd.year), unsigned(ymd.month), unsigned(ymd.day));

    {
        std::nth_element(vols.begin(), vols.begin()+vols.size()/2, vols.end());
        vols.resize(vols.size()/2);
        unsigned long v = 0;
        for (int x : vols) {
            v += x;
        }
        fprintf(fp, "\t%lu", v);
    }

    fprintf(fp, "\t%.1f\t%.2f\t%.1f\t%.2f"
            , avbuysell[1].volume, avbuysell[1].amount, avbuysell[0].volume, avbuysell[0].amount);
    {
        Av s[2] = {};
        for (int i=0; i<2; ++i) {
            for (auto& v : avmax20[i]) {
                s[i] = v;
            }
        }
        fprintf(fp, "\t%.1f\t%.2f\t%.1f\t%.2f", s[1].volume, s[1].amount, s[0].volume, s[0].amount);
    }

    fprintf(fp, "\t%.2f\t%.2f\t%.2f\t%.2f" , ochl[0], ochl[1], ochl[2], ochl[3]);
    fprintf(fp, "\n");

    return 0;
}

struct XRec {
    gregorian::date date; typedef boost::fusion::vector<int, int, int> date_parts;
    std::vector<float> ochl; typedef std::vector<float> ochl_type;
    std::vector<Av> s; typedef std::vector<Av> sum_type;
    std::vector<std::vector<Av>> v; typedef std::vector<std::vector<Av>> xdetail_type;
};
BOOST_FUSION_ADAPT_STRUCT(XRec, date, ochl, s, v)

void print(std::istream& ifs, gregorian::date const& date0, gregorian::date const& date1)
{
    static const qi::int_parser<unsigned, 10, 4, 4> _4digit = {};
    static const qi::int_parser<unsigned, 10, 2, 2> _2digit = {};
    using qi::float_; //using qi::_val; using qi::_1; using ascii::char_; using qi::int_; using phoenix::ref;

    typedef std::string::const_iterator iterator_type;

    qi::rule<iterator_type,Av(),ascii::space_type> rule_av = (float_ >> float_);
    qi::rule<iterator_type,XRec::ochl_type(),ascii::space_type> rule_ochl = (float_ >> float_ >> float_ >> float_);
    qi::rule<iterator_type,XRec::date_parts(),ascii::space_type> rule_date = (_4digit >> _2digit >> _2digit);
    qi::rule<iterator_type,XRec::xdetail_type(),ascii::space_type> rule_xdetail = qi::repeat(12)[rule_av];
    qi::rule<iterator_type,XRec::sum_type(),ascii::space_type> rule_sum = rule_av >> rule_av;
    qi::rule<iterator_type,XRec(),ascii::space_type> parser =
        rule_date
        >> rule_ochl
        >> rule_sum
        >> rule_xdetail
        ;

    std::string str;
    while (getline(ifs, str)) {
        XRec xr = {};

        iterator_type iter = str.begin();
        iterator_type end = str.end();
        bool r = qi::phrase_parse(iter, end, parser, ascii::space, xr);//(xr.date, xr.s, volume, amount, xr.ochl, xr.v);
        if (r /*&& iter == end*/) {
            //std::cout << boost::fusion::tuple_open('[');
            //std::cout << boost::fusion::tuple_close(']');
            //std::cout << boost::fusion::tuple_delimiter(", ");
            //std::cout << "got: " << boost::fusion::as_vector(xr) << std::endl;

            int volume = xr.s[0].volume + xr.s[1].volume;
            float amount = xr.s[0].amount + xr.s[1].amount;

            printf("[  ]\t%.2f\t%+.3f\t%+.2f\t%.2f\t%.2f""\t%.2f\t%.2f\t%.2f""\t%.2f,\t%.2f,\t%.2f,\t%.2f""\n"
                    , (xr.s[1].amount / xr.s[0].amount)
                    , (xr.s[1].amount - xr.s[0].amount)/(xr.s[1].amount + xr.s[0].amount)
                    , (xr.s[1].amount - xr.s[0].amount)/10000
                    , (xr.s[1].amount + xr.s[0].amount)/10000
                    , (xr.s[1].volume + xr.s[0].volume)/100
                    //
                    , amount/volume
                    , xr.s[1].amount/xr.s[1].volume
                    , xr.s[0].amount/xr.s[0].volume
                    , xr.ochl[0], xr.ochl[1], xr.ochl[2], xr.ochl[3]
                    );
            for (unsigned i = 0; i<xr.v.size(); ++i) {
                printf("[%02d]\t%.2f\t%+.2f\t%+.2f\t%.2f\n", i+1
                        , (xr.v[i][1].amount / xr.v[i][0].amount)
                        , (xr.v[i][1].amount - xr.v[i][0].amount)/(xr.v[i][1].amount + xr.v[i][0].amount)
                        , (xr.v[i][1].amount - xr.v[i][0].amount)/10000
                        , (xr.v[i][1].volume + xr.v[i][0].volume)/(volume)
                        );
            }

        } else {
            std::cerr << "Parsing failed\n";
            break;
        }
    }
}

int Main(int argc, char* const argv[])
{
    if (argc != 2) {
        return 1; //return print();
    }
    if (!filesystem::is_directory(argv[1])/* || !filesystem::is_directory(argv[2])*/) {
        std::cerr << "!filesystem::is_directory\n";
        return 2;
    }

    gregorian::date date = to_date(argv[1]);
    // std::cout << argv[1] <<" "<< date <<"\n";
    //char* const ofn = argv[2]; //filesystem::path const odir(argv[2]);

    for (auto& x : filesystem::directory_iterator(argv[1])) {
        auto& cp = x.path();
        int code = to_code(cp.leaf().string());
        if (!code || !filesystem::is_regular_file(cp))
            continue;
        //std::cout << (cp / "LEAF").generic_string() <<'\t'<<code<<"\n";

        if (FILE* fp = fopen(cp.generic_string().c_str(), "r")) {
            xsum(fp, code, date, stdout);
            fclose(fp);
        }
    }
    return 0;
}

