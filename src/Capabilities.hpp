//
// Created by Kirill Delimbetov - github.com/delimbetov - on 20.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <cstdint>
#include <Common.hpp>

namespace rune_vm_internal {
#error 1. some capabilities impl are common, some platform specific
#error 2. common caps should include default implementation, but they should also be overridable by user
#error 3. there should be a way to check which caps are not implemented at all
#error 4. caps should be rune-specific
}


