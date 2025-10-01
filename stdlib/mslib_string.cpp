#include "mslib_string.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cstdlib>

using namespace moss;
using namespace mslib;

Value *String::String_constructor(Interpreter *vm, Value *v, Value *&err) {
    (void)err;
    if (isa<ObjectValue>(v)) {
        Value *trash_err = nullptr;
        auto rval = mslib::call_type_converter(vm, v, "String", known_names::TO_STRING_METHOD, trash_err);
        if (rval && !trash_err) {
            if (!isa<StringValue>(rval)) {
                rval = StringValue::get(rval->as_string());
            }
            return rval;
        }
    }
    return StringValue::get(v->as_string());
}

static opcode::IntConst count_substrings(const std::string& str, const std::string& sub) {
    if (sub.empty()) return 0; // avoid infinite loop
    opcode::IntConst count = 0;
    std::size_t pos = 0;

    while ((pos = str.find(sub, pos)) != std::string::npos) {
        ++count;
        pos += sub.size(); // move past this occurrence
    }

    return count;
}

Value *String::count(Interpreter *vm, Value *ths, Value *sub, Value *&err) {
    auto ths_str = mslib::get_string(ths);
    auto sub_str = mslib::get_string(sub);
    if (sub_str.empty())
        return IntValue::get(ths_str.length());
    return IntValue::get(count_substrings(ths_str, sub_str));
}


Value *String::capitalize(Interpreter *vm, Value *ths, Value *&err) {
    auto strv = dyn_cast<StringValue>(ths);
    assert(strv && "not string");
    auto text = strv->get_value();
    ustring res = strv->get_value();
    std::transform(text.begin(), text.end(), res.begin(), ::tolower);
    res[0] = std::toupper(res[0]);
    return StringValue::get(res);
}

Value *String::upper(Interpreter *vm, Value *ths, Value *&err) {
    auto strv = dyn_cast<StringValue>(ths);
    assert(strv && "not string");
    ustring text = strv->get_value();
    ustring res(text.length(), '\0');
    std::transform(text.begin(), text.end(), res.begin(), ::toupper);
    return StringValue::get(res);
}

Value *String::lower(Interpreter *vm, Value *ths, Value *&err) {
    auto strv = dyn_cast<StringValue>(ths);
    assert(strv && "not string");
    ustring text = strv->get_value();
    ustring res(text.length(), '\0');
    std::transform(text.begin(), text.end(), res.begin(), ::tolower);
    return StringValue::get(res);
}

Value *String::replace(Interpreter *vm, Value *ths, Value *target, Value *value, Value *count, Value *&err) {
    // TODO: Have empty string replace be in between letters:
    // >>> "hello".replace("", "X")
    // 'XhXeXlXlXoX'
    auto strv = dyn_cast<StringValue>(ths);
    assert(strv && "not string");
    auto targv = dyn_cast<StringValue>(target);
    assert(targv && "not string");
    auto valuev = dyn_cast<StringValue>(value);
    assert(valuev && "no string");
    auto countv = dyn_cast<IntValue>(count);
    assert(countv && "not int");
    return StringValue::get(utils::replace_n(strv->get_value(), targv->get_value(), valuev->get_value(), countv->get_value()));
}

Value *String::multi_replace(Interpreter *vm, Value *ths, Value *mappings, Value *&err) {
    // TODO: Have empty string replace be in between letters:
    // >>> "hello".replace("", "X")
    // 'XhXeXlXlXoX'
    auto strv = dyn_cast<StringValue>(ths);
    assert(strv && "not string");
    auto mapgsv = dyn_cast<ListValue>(mappings);
    assert(mapgsv && "not string");
    auto result = strv->get_value();
    for (auto elem: mapgsv->get_vals()) {
        auto elemv = dyn_cast<ListValue>(elem);
        bool correct_elem = true;
        if (!elemv)
            correct_elem = false;
        else if (elemv->get_vals().size() != 2)
            correct_elem = false;
        else if (!isa<StringValue>(elemv->get_vals()[0]) || !isa<StringValue>(elemv->get_vals()[1]))
            correct_elem = false;
        if (!correct_elem) {
            err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_MULTI_REPLACE_ELEM));
            return nullptr;
        }
        auto target = dyn_cast<StringValue>(elemv->get_vals()[0]);
        auto value = dyn_cast<StringValue>(elemv->get_vals()[1]);
        utils::replace_in_n(result, target->get_value(), value->get_value());
    }
    return StringValue::get(result);
}

static std::vector<ustring> split_whitespace(const std::string& text, long max_split=-1) {
    std::vector<ustring> tokens;
    size_t splits_done = 0;

    auto it = text.begin();
    while (it != text.end()) {
        // Skip leading whitespace
        it = std::find_if_not(it, text.end(), [](unsigned char ch){ return std::isspace(ch); });
        if (it == text.end()) break;

        // If max_split reached, take the rest of the string as the last token
        if (splits_done > 0 && splits_done == max_split) {
            tokens.emplace_back(it, text.end());
            break;
        }

        // Find next whitespace
        auto end = std::find_if(it, text.end(), [](unsigned char ch){ return std::isspace(ch); });
        tokens.emplace_back(it, end);
        it = end;
        ++splits_done;
    }

    return tokens;
}

std::vector<ustring> split_on(const std::string& text, const std::string& delim, long max_split=-1) {
    std::vector<ustring> tokens;
    size_t start = 0;
    size_t splits_done = 0;

    if (delim.empty()) {
        // Treat empty delimiter as splitting every character
        for (char ch : text) {
            tokens.push_back(std::string(1, ch));
        }
        return tokens;
    }

    while (start <= text.size()) {
        if (splits_done > 0 && splits_done == max_split) {
            // Take the rest of the string as last token
            tokens.push_back(text.substr(start));
            break;
        }

        size_t pos = text.find(delim, start);
        if (pos == ustring::npos) {
            tokens.push_back(text.substr(start));
            break;
        }

        tokens.push_back(text.substr(start, pos - start));
        start = pos + delim.size();
        ++splits_done;
    }

    return tokens;
}

Value *String::split(Interpreter *vm, Value *ths, Value *sep, Value *max_split, Value *&err) {
    auto strv = dyn_cast<StringValue>(ths);
    assert(strv && "not string");
    auto max_splitv = dyn_cast<IntValue>(max_split);
    assert(max_splitv && "not int");

    StringValue *sepv = dyn_cast<StringValue>(sep);
    assert((sepv || isa<NilValue>(sep)) && "incorrect type");

    std::vector<ustring> splitted;
    if (!sepv) {
        splitted = split_whitespace(strv->get_value(), max_splitv->get_value());
    } else {
        splitted = split_on(strv->get_value(), sepv->get_value(), max_splitv->get_value());
    }
    std::vector<Value *> splitted_str;
    splitted_str.reserve(splitted.size());
    for (const auto &s: splitted) {
        splitted_str.push_back(StringValue::get(s));
    }

    return new ListValue(splitted_str);
}

Value *String::isfun(Interpreter *vm, Value *ths, std::function<bool(std::wint_t)> fn, Value *&err) {
    auto strv = dyn_cast<StringValue>(ths);
    assert(strv && "not string");
    ustring str_text = strv->get_value();
    // Save current locale
    char* old_locale = std::setlocale(LC_CTYPE, nullptr);

    // Copy it because setlocale returns pointer to internal storage
    std::string saved_locale = old_locale ? old_locale : "C";

#ifdef __windows__
    std::setlocale(LC_CTYPE, ".UTF-8");
#else
    std::setlocale(LC_CTYPE, "");
#endif

    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    std::wstring text = conv.from_bytes(str_text);
    if (text.empty())
        return BuiltIns::False;
    for (std::wint_t c: text){
        if (!fn(c)) {
            // Restore previous locale
            std::setlocale(LC_CTYPE, saved_locale.c_str());
            return BuiltIns::False;
        }
    }
    // Restore previous locale
    std::setlocale(LC_CTYPE, saved_locale.c_str());
    return BuiltIns::True;
}

Value *String::index(Interpreter *vm, Value *ths, Value *value, Value *&err) {
    auto strv = mslib::get_string(ths);
    ustring subst = mslib::get_string(value);

    size_t pos = strv.find(subst);
    if (pos == std::string::npos) {
        return IntValue::get(-1);
    }
    return IntValue::get(pos);
}