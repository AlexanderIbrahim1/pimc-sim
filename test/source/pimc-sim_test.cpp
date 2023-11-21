#include "lib.hpp"

auto main() -> int
{
  auto const lib = library {};

  return lib.name == "pimc-sim" ? 0 : 1;
}
