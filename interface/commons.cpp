#include "commons.hpp"

using namespace moss;

bool global_controls::trigger_gc = false;

bool global_controls::exit_called = false;

std::filesystem::path global_controls::pwd = "./";