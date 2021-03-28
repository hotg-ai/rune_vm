//
// Created by Kirill Delimbetov - github.com/delimbetov - on 20.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <stdexcept>

#define CHECK_THROW(cond)                                                   \
    do {                                                                    \
        if (!(cond)) {                                                      \
            throw std::runtime_error("Check failed: " #cond " file: " __FILE__); \
        }                                                                   \
    } while (0)
