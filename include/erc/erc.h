#pragma once

// IWYU pragma: begin_exports
#include "resource.h"

// IWYU pragma: end_exports

#define ERC_IMPORT(Name, Path) \
    namespace erc { extern constinit ::erc::resource Name; }

#define ERC_EXPOET(Name)
