#include "time.hpp"
#include "moss.hpp"
#include "builtins.hpp"
#include <chrono>
#include <ctime>

using namespace moss;
using namespace mslib;
using namespace time;
using namespace std::chrono;

const std::unordered_map<std::string, mslib::mslib_dispatcher>& time::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        {"time", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 0);
            return time::time(vm, err);
        }},
        {"localtime", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 1);
            return time::localtime(vm, cf, args[0].value, err);
        }},
        {"strftime", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            assert(cf->get_args().size() == 2);
            return time::strftime(vm, cf, cf->get_arg("format"), cf->get_arg("t"), err);
        }},
    };
    return registry;
}

Value *time::time(Interpreter *vm, Value *&err) {
    double timestamp = duration_cast<duration<double>>(
        system_clock::now().time_since_epoch()
    ).count();
    return new FloatValue(timestamp);
}

Value *time::localtime(Interpreter *vm, CallFrame *cf, Value *secs, Value *&err) {
    std::time_t t;
    if (isa<NilValue>(secs)) {
        t = std::time(nullptr);
    } else {
        assert(isa<FloatValue>(secs) || isa<IntValue>(secs) && "Allowed other type than int,float or nil?");
        t = static_cast<std::time_t>(secs->as_float());
    }
    std::tm *local = std::localtime(&t);
    if (!local) {
        err = mslib::create_os_error(diags::Diagnostic(*vm->get_src_file(), diags::TIME_TIMESTAMP_OOR));
        return nullptr;
    }
    auto tm_year = new IntValue(1900+local->tm_year);
    auto tm_mon = new IntValue(1 + local->tm_mon); // C/C++ uses 0 for January
    auto tm_mday = new IntValue(local->tm_mday);
    auto tm_hour = new IntValue(local->tm_hour);
    auto tm_min = new IntValue(local->tm_min);
    auto tm_sec = new IntValue(local->tm_sec);
    auto tm_wday = new IntValue(local->tm_wday);
    auto tm_yday = new IntValue(local->tm_yday);
    auto tm_isdst = new IntValue(local->tm_isdst);
    // tm_zone and tm_gmtoff is only on posix
#ifdef __linux__ // Add also mac
    auto tm_zone = new StringValue(local->tm_zone);
    auto tm_gmtoff = new IntValue(local->tm_gmtoff);
#else
    auto tm_zone = new StringValue("");
    auto tm_gmtoff = BuiltIns::Nil;
#endif

    auto timestru = mslib::call_constructor(vm, cf, "StructTime", {tm_year, tm_mon, tm_mday, tm_hour, tm_min, tm_sec, tm_wday, tm_yday, tm_isdst, tm_zone, tm_gmtoff}, err);
    // If err then timestru is nullptr anyway and err is set
    return timestru;
}

static ustring strftime_dynamic(const std::string& format, const std::tm* tm) {
    // start with a reasonable size and grow if needed
    size_t used_size = 64;
    ustring result(used_size, '\0');
    size_t len = std::strftime(&result[0], used_size, format.c_str(), tm);
    while (len == 0) {
        // try larger buffer if not enough space
        used_size += 256;
        result.resize(used_size);
        len = std::strftime(&result[0], used_size, format.c_str(), tm);
    }
    result.resize(len);
    return result;
}

Value *time::strftime(Interpreter *vm, CallFrame *cf, Value *format, Value *t, Value *&err) {
    auto get_time_int_att = [&](const char *name) -> opcode::IntConst {
        // Early exit if error was set
        if (err)
            return 0;
        Value *v = mslib::get_attr(t, name, vm, err);
        if (!v)
            return 0;
        IntValue *iv = dyn_cast<IntValue>(v);
        if (!iv) {
            err = create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::UNEXPECTED_TYPE, "Int", v->get_type()->get_name().c_str()));
            return 0;
        }
        return iv->get_value();
    };

    std::tm temp_time {};
    std::tm *time_to_format = &temp_time;
    if (isa<NilValue>(t)) {
        auto now = std::time(nullptr);
        time_to_format = std::localtime(&now);
    } else {
        assert(t->get_type()->get_name() == "StructTime" && "Unexpected type");
        temp_time.tm_year = get_time_int_att("tm_year")-1900;
        temp_time.tm_mon = get_time_int_att("tm_mon")-1;
        temp_time.tm_mday = get_time_int_att("tm_mday");
        temp_time.tm_hour = get_time_int_att("tm_hour");
        temp_time.tm_min = get_time_int_att("tm_min");
        temp_time.tm_sec = get_time_int_att("tm_sec");
        temp_time.tm_wday = get_time_int_att("tm_wday");
        temp_time.tm_yday = get_time_int_att("tm_yday");
        temp_time.tm_isdst = get_time_int_att("tm_isdst");
        // tm_zone and tm_gmtoff is only on posix
#ifdef __linux__ // Add also mac
        // Following value might be nil
        Value *tm_gmtoff_v = mslib::get_attr(t, "tm_gmtoff", vm, err);
        if (tm_gmtoff_v && !isa<NilValue>(tm_gmtoff_v))
            temp_time.tm_gmtoff = get_time_int_att("tm_gmtoff");
        
        if (err)
            return nullptr;

        Value *tm_zone_v = mslib::get_attr(t, "tm_zone", vm, err);
        if (!tm_zone_v)
            return nullptr;
        auto tm_zone_str = dyn_cast<StringValue>(tm_zone_v);
        if (!tm_zone_str) {
            err = create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::UNEXPECTED_TYPE, "String", tm_zone_v->get_type()->get_name().c_str()));
            return nullptr;
        }
        // tm_zone might be empty string
        if (tm_zone_str->get_value().length() > 0)
            temp_time.tm_zone = tm_zone_str->get_value().c_str();
#endif
    }
    
    auto format_str = dyn_cast<StringValue>(format);
    assert(format_str && "Unexpected type");

    ustring formatted = strftime_dynamic(format_str->get_value(), time_to_format);
    return new StringValue(formatted);
}