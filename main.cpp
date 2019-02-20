#include <optional>
using std::nullopt;
using std::optional;
#include <variant>
using std::holds_alternative;
using std::variant;
#include <system_error>
using std::error_condition;
#include <string>
using std::string;
#include <sstream>
using std::stringstream;

//#define MONADIC 1

#ifdef MONADIC
bool& emptyState()
{
    thread_local bool threadState {};
    return threadState;
}

#define try_unwrap(optionalExp)                  \
    [&]() -> auto                                \
    {                                            \
        auto val = (optionalExp);                \
                                                 \
        if (!val) {                              \
            emptyState() = true;                 \
            return decltype(val)::value_type {}; \
        }                                        \
        emptyState() = false;                    \
        return *val;                             \
    }                                            \
    ();                                          \
    if (emptyState())                            \
        return nullopt;

#define try_expect(optionalExp, error)       \
    [&]() -> auto                            \
    {                                        \
        auto val = (optionalExp);            \
                                             \
        if (val) {                           \
            emptyState() = false;            \
            return *val;                     \
        }                                    \
        emptyState() = true;                 \
        return decltype(val)::value_type {}; \
    }                                        \
    ();                                      \
    if (emptyState())                        \
        return error;

#endif //MONADIC

struct MyStruct {
    int member;
};

optional<MyStruct> getParams(const std::string& v)
{
    MyStruct tmp;
    if (stringstream(v) >> tmp.member)
        return tmp;

    return nullopt;
}

template <typename R, typename E>
using outcome = variant<R, E>;

bool test_expect(const string& input)
{
    auto val = [&]() -> outcome<string, error_condition> {
#ifdef MONADIC
        MyStruct tmp = try_expect(getParams(input), std::errc::no_link);
#else
        optional<MyStruct> maybeTmp = getParams(input);
        if (!maybeTmp)
            return std::errc::no_link;

        auto tmp = *maybeTmp;
#endif //MONADIC

        tmp.member = 42;
        stringstream s;
        s << tmp.member;
        return s.str();
    }();

    return holds_alternative<string>(val);
}

bool test_unwrap(const string& input)
{
    auto val = [&]() -> optional<string> {
#ifdef MONADIC
        MyStruct tmp = try_unwrap(getParams(input));
#else
        optional<MyStruct> maybeTmp = getParams(input);
        if (!maybeTmp)
            return nullopt;

        MyStruct tmp = *maybeTmp;
#endif // MONADIC

        tmp.member = 1337;
        stringstream s;
        s << tmp.member;
        return s.str();
    }();

    return val.has_value();
}

#include <functional>
using std::function;
#include <iostream>
using std::cerr;
using std::clog;
using std::cout;
using std::endl;

#define test(func, input, output) \
    test_helper(func, #func, input, output)

template <typename FuncT, typename InputT, typename OutputT>
auto test_helper(FuncT func, const std::string& fName, const InputT& input, const OutputT& output) -> void
{
    if (!(func(input) == output))
        clog << fName << " failed on input \"" << input << "\"\n";
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cerr << "too few args\n";
        return 2;
    }
    unsigned long long rounds = std::stoll(string(argv[1]));
    cout << "running " << rounds << " tests\n";
#ifdef MONADIC
    cout << "with monadic active" << endl;
#else
    cout << "with monadic inactive" << endl;
#endif //MONADIC

    const string good = "1337";
    const string bad = "lol";
    for (auto i = rounds; i > 0; i--) {
        test(test_unwrap, good, true);
        test(test_unwrap, bad, false);
        test(test_expect, good, true);
        test(test_expect, bad, false);
    }
}
