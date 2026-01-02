#include "re.hpp"
#include "values_cpp.hpp"
#include <regex>

using namespace moss;
using namespace mslib;
using namespace re;

const std::unordered_map<std::string, mslib::mslib_dispatcher>& re::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        {"Pattern", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto args = cf->get_args();
            assert(args.size() == 3);
            return Pattern(vm, cf, cf->get_arg("this"), cf->get_arg("pattern"), cf->get_arg("flags"), err);
        }},
        {"match", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto args = cf->get_args();
            assert(args.size() == 2);
            return match_or_search(true, vm, cf, cf->get_arg("this"), cf->get_arg("text"), err);
        }},
        {"search", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto args = cf->get_args();
            assert(args.size() == 2);
            return match_or_search(false, vm, cf, cf->get_arg("this"), cf->get_arg("text"), err);
        }},
    };
    return registry;
}

std::regex_constants::syntax_option_type extract_syntax_flags(Interpreter *vm, Value *vflags, Value *&err) {
    using namespace std::regex_constants;
    syntax_option_type flags = syntax_option_type{};

    //auto grammar_v = mslib::get_attr(vflags, "grammar", vm, err);
    //if (err)
    //    return flags;

    auto icase_v = mslib::get_attr(vflags, "icase", vm, err);
    if (err)
        return flags;
    if (icase_v == BuiltIns::True)
        flags |= std::regex_constants::icase;

    return flags;
}

Value *re::Pattern(Interpreter *vm, CallFrame *, Value *ths, Value *pattern, Value *flags, Value *&err) {
    ths->set_attr("flags", flags);
    ths->set_attr("pattern", pattern);
    auto patt = mslib::get_string(pattern);
    // TODO: Pass in compiletime flags
    auto regex = new std::regex(patt, extract_syntax_flags(vm, flags, err));
    ths->set_attr(known_names::REGEX_ATT, new t_cpp::RegexValue(regex));
    return ths;
}


Value *re::match_or_search(bool match, Interpreter *vm, CallFrame *cf, Value *ths, Value *text, Value *&err) {
    auto regex = mslib::get_attr(ths, known_names::REGEX_ATT, vm, err);
    t_cpp::RegexValue *rv = dyn_cast<t_cpp::RegexValue>(regex);
    // TODO: Raise error
    assert(rv && "__regex is not RegexValue");
    auto ts = mslib::get_string(text);
    std::smatch m;

    bool matched = false;
    // TODO: Pass in flags
    if (match) {
        matched = std::regex_match(ts, m, *rv->get_re());
    } else {
        matched = std::regex_search(ts, m, *rv->get_re());
    }
    if (matched) {
        auto patt = mslib::get_attr(ths, "pattern", vm, err);
        std::vector<Value *> groups;
        groups.reserve(m.size());

        for (const auto& sm : m) {
            StringValue *strv = StringValue::get(sm.str());
            groups.push_back(strv);
        }
        return mslib::call_constructor(vm, cf, "Match", {ths, text, new ListValue(groups)}, err);
    }
    return nullptr;
}