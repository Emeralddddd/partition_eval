#include "lib.hpp"

auto main() -> int
{
  auto const lib = library {};

  return lib.name == "eval" ? 0 : 1;
}
