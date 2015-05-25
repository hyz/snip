// #include "myconfig.h"
#include <boost/range.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <sstream>
//#include "libjson/json.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/memorystream.h"
#include "rapidjson/encodedstream.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/reader.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"

#include "log.h"
#include "myerror.h"
#include "json.h"

namespace rjson = rapidjson;

using namespace std;
using namespace boost;

namespace boost_swap_impl
{
    void swap_impl(json::variant& left, std::string& right)
    {
        std::string tmp;
        tmp.swap(right);
        left = boost::move(tmp);
    }
} // namespace boost_swap_impl

namespace json {

bool object::rename(const std::string & fr, const std::string & to)
{
    json::variant tmp;
    {
        auto i = find(fr);
        if (i == end())
            return false;

        std::swap(tmp, i->second);
        erase(i);
    }

    insert( std::make_pair(to, tmp) );
    return true;
}

template <typename writer>
struct o_visitor : boost::static_visitor<>
{
    writer & w_;

    o_visitor(writer& w) : w_(w) {}

    void operator()(json::object const& obj) {
        w_.StartObject();
        BOOST_FOREACH(json::object::value_type const& p, obj)
            (*this)(p);
        w_.EndObject();
    }
    void operator()(json::array const& arr) {
        w_.StartArray();
        BOOST_FOREACH(auto const& x, arr)
            boost::apply_visitor( *this, x );
        w_.EndArray();
    }
    void operator()(json::string const& s) { w_.String(s.c_str()); }
    void operator()(json::int_t x) { w_.Int64(x); }
    void operator()(json::null_t) { w_.Null(); }
    void operator()(double x) { w_.Double(x); }
    void operator()(bool x) { w_.Bool(x); }

    void operator()(json::object::value_type const& p) {
        w_.String(p.first.c_str());
        boost::apply_visitor( *this, p.second );
    }
};

struct sax_like_handler
{
    struct frame_t {
        boost::optional<std::string> name;
        json::variant var_;
        frame_t(json::object const& o) : var_(o) {}
        frame_t(json::array const& a) : var_(a) {}
    };

    template <typename T>
    struct i_visitor : boost::static_visitor<> {
        T & val_;
        frame_t & fx_;

        i_visitor(T& v, frame_t& fx) : val_(v), fx_(fx) {}

        void operator()(json::object& obj) {
            if (fx_.name) {
                std::string & k = fx_.name.value();
                json::object::value_type p;
                boost::swap(k, p.first);
                p.second = val_; // boost::swap(p.second, val_);
                obj.object_impl_t::insert(p);
                fx_.name = boost::optional<std::string>();
            } else {
                put_name(val_);
            }
        }
        template <typename X> void put_name(X) {}
        void put_name(std::string& n) { fx_.name= n; }

        void operator()(json::array& arr) {
            arr.array_impl_t::push_back(val_);
        }

        template <typename X> void operator()(X) {}
    };

    std::vector<frame_t> stack_;
    json::variant& root_;

    sax_like_handler(json::variant& v)
        : root_(v)
    {}

	bool Null() {
        json::variant v = json::null_t();
        return _save(v);
    }
	bool Bool(bool b) {
        json::variant v = b;
        return _save(v);
    }
	bool Int64(int64_t i) {
        json::variant v = int_t(i);
        return _save(v);
    }
	bool Uint64(uint64_t u) { return Int64(u); }
	bool Uint(unsigned u) { return Int64(u); }
	bool Int(int i) { return Int64(i); }

	bool Double(double d) {
        json::variant v = d;
        return _save(v);
    }
    bool String(const char* str, rjson::SizeType length, bool copy) { 
        std::string v(str, length);
        return _save(v);
	}

	bool StartObject() {
        stack_.push_back( frame_t(json::object()) );
        return true;
    }
	bool EndObject(rjson::SizeType memberCount)
    {
        json::variant tmp;
        boost::swap(tmp, stack_.back().var_);
        stack_.pop_back();
        return _save(tmp);
    }

	bool StartArray()
    {
        stack_.push_back( frame_t(json::array()) );
        return true;
    }
	bool EndArray(rjson::SizeType elementCount)
    {
        json::variant tmp;
        boost::swap(tmp, stack_.back().var_);
        stack_.pop_back();
        return _save(tmp);
    }

    template <typename T>
    bool _save(T & val) {
        if (stack_.empty()) {
            root_ = val;
            // boost::swap(val, root_);
        } else {
            frame_t& fx = stack_.back();
            i_visitor<T> vis(val, fx);
            boost::apply_visitor( vis, fx.var_ );
        }
        return true;
    }
};

bool _decode(json::variant& result, char const* beg, char const* end)
{
    // AUTO_CPU_TIMER("json:decode");

    sax_like_handler handler(result);
    rjson::MemoryStream mems(beg, end - beg);
    rjson::EncodedInputStream<rjson::UTF8<>, rjson::MemoryStream> ins(mems);

    rjson::Reader reader;
    if (!reader.Parse(ins, handler))
    {
        //rjson::ParseErrorCode e = reader.GetParseErrorCode();
        //size_t o = reader.GetErrorOffset();
        // cout << "Error: " << GetParseError_En(e) << endl;;
        // cout << " at offset " << o << " near '" << string(json).substr(o, 10) << "...'" << endl;
        return false;
    }
    return true;
}

void pretty_print(FILE* out_f, json::variant const & jv)
{
	char out_buf[65536];
    rjson::FileWriteStream os(out_f, out_buf, sizeof(out_buf));
    rjson::PrettyWriter<rjson::FileWriteStream> writer(os);

	o_visitor<rjson::PrettyWriter<rjson::FileWriteStream>> ov(writer);
    boost::apply_visitor( ov, jv );
}

std::string encode(json::variant const & jv)
{
    typedef rjson::Writer<rjson::StringBuffer> writer_t;
    rjson::StringBuffer out;
    writer_t writer(out);

    o_visitor<writer_t> ov(writer);
    boost::apply_visitor( ov, jv );

    return out.GetString();
}

template <typename T>
std::string encode_2(T const & jv)
{
    typedef rjson::Writer<rjson::StringBuffer> writer_t;
    rjson::StringBuffer out;
    writer_t writer(out);

    o_visitor<writer_t> ov(writer);
    ov(jv);

    return out.GetString();
}

std::string encode(json::array const & jv) { return encode_2(jv); }
std::string encode(json::object const & jv) { return encode_2(jv); }

} // namespace json

//#include <iostream>
//
//int main() {
//    const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ";
//
//    sax_like_handler handler;
//    Reader reader;
//    StringStream ss(json);
//	reader.Parse(ss, handler);
//
//	return 0;
//}

