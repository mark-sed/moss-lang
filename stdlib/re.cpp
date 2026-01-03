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

    auto grammar_v = mslib::get_attr(vflags, "grammar", vm, err);
    if (err)
        return flags;
    auto grammar_enum = dyn_cast<EnumValue>(grammar_v);
    if (!grammar_enum) {
        // This should not really ever happen as compile is the same as instantiation
        // and grammar is typechecked in the function call.
        err = create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::UNEXPECTED_TYPE, "enum REFlags.Grammars", grammar_v->get_type()->get_name().c_str()));
        return flags;
    }
    auto gname = grammar_enum->get_name();
    auto grammar_const = ECMAScript;
    if (gname == "ECMAScript") {
        grammar_const = ECMAScript;
    } else if (gname == "basic") {
        grammar_const = basic;
    } else if (gname == "extended") {
        grammar_const = extended;
    } else if (gname == "awk") {
        grammar_const = awk;
    } else if (gname == "grep") {
        grammar_const = grep;
    } else if (gname == "egrep") {
        grammar_const = egrep;
    } else {
        assert(false && "Extra name in enum Grammars");
    }
    flags |= grammar_const;

    auto icase_v = mslib::get_attr(vflags, "icase", vm, err);
    if (err)
        return flags;
    if (icase_v == BuiltIns::True)
        flags |= std::regex_constants::icase;

    auto nosubs_v = mslib::get_attr(vflags, "nosubs", vm, err);
    if (err)
        return flags;
    if (nosubs_v == BuiltIns::True)
        flags |= std::regex_constants::nosubs;

    auto optimize_v = mslib::get_attr(vflags, "optimize", vm, err);
    if (err)
        return flags;
    if (optimize_v == BuiltIns::True)
        flags |= std::regex_constants::optimize;

    auto collate_v = mslib::get_attr(vflags, "collate", vm, err);
    if (err)
        return flags;
    if (collate_v == BuiltIns::True)
        flags |= std::regex_constants::collate;

    auto multiline_v = mslib::get_attr(vflags, "multiline", vm, err);
    if (err)
        return flags;
    if (multiline_v == BuiltIns::True)
        flags |= std::regex_constants::multiline;

    return flags;
}

Value *re::Pattern(Interpreter *vm, CallFrame *, Value *ths, Value *pattern, Value *flags, Value *&err) {
    ths->set_attr("flags", flags);
    ths->set_attr("pattern", pattern);
    auto patt = mslib::get_string(pattern);
    std::regex *regex = nullptr;
    try {
        regex = new std::regex(patt, extract_syntax_flags(vm, flags, err));
    } catch (std::regex_error err) {
        // throw moss error
        outs << "REGEX ERROR!\n";
        regex = new std::regex(patt);
    }
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