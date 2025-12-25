#pragma once

namespace libvoicefeat::constants
{
    // --------------------------
    // MATH CONSTANTS
    // --------------------------

    constexpr double PI = 3.14159265358979323846f;
    constexpr double K_LOG_EPS = 1e-10;
    constexpr double EPS = 1e-8f;

    // --------------------------
    // FEATURE CONSTANTS
    // --------------------------

    constexpr int DEFAULT_MFCC_FILTERS_NUM = 26;
    constexpr int DEFAULT_GFCC_FILTERS_NUM = 32;
    constexpr int DEFAULT_LFCC_FILTERS_NUM = 30;
    constexpr int DEFAULT_PNCC_FILTERS_NUM = 40;
    constexpr int DEFAULT_PLP_FILTERS_NUM  = 20;

    constexpr int DEFAULT_MFCC_COEFFS_NUM = 13;
    constexpr int DEFAULT_GFCC_COEFFS_NUM = 20;
    constexpr int DEFAULT_LFCC_COEFFS_NUM = 20;
    constexpr int DEFAULT_PNCC_COEFFS_NUM = 20;
    constexpr int DEFAULT_PLP_COEFFS_NUM  = 13;

    constexpr int DEFAULT_MFCC_MIN_FREQ = 0.0f;
    constexpr int DEFAULT_GFCC_MIN_FREQ = 50.0f;
    constexpr int DEFAULT_LFCC_MIN_FREQ = 0.0f;
    constexpr int DEFAULT_PNCC_MIN_FREQ = 20.0f;
    constexpr int DEFAULT_PLP_MIN_FREQ  = 0.0f;
}
