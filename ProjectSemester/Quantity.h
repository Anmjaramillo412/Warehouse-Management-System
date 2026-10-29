#pragma once

// ================================================================
// QUANTITY HELPERS
// ----------------------------------------------------------------
// Material-level quantities (stock, BOM, movements, projections,
// orders, receipts, purchases) are stored as double but are always
// kept at a maximum of 3 decimals. Every arithmetic result is passed
// through roundQty() so values such as 0.1 + 0.2 never end up as
// 0.30000000000000004, and every value shown to a user or written to
// a file goes through formatQty() so it never appears as
// "6600.000000" or "1e+06".
// ================================================================

#include <cmath>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>

constexpr int    QTY_DECIMALS = 3;
constexpr double QTY_SCALE    = 1000.0;

// Round to 3 decimals
inline double roundQty(double value)
{
    if (!std::isfinite(value))
    {
        return 0.0;
    }

    double r = std::round(value * QTY_SCALE) / QTY_SCALE;

    // Avoid "-0"
    if (r == 0.0)
    {
        return 0.0;
    }

    return r;
}

// True when the value is finite and has no more than 3 decimals
// (small tolerance for binary floating point representation).
inline bool hasValidQtyDecimals(double value)
{
    if (!std::isfinite(value))
    {
        return false;
    }

    double scaled = value * QTY_SCALE;

    return std::fabs(scaled - std::round(scaled)) < 1e-6;
}

// True when the value is effectively zero at 3-decimal precision
inline bool isZeroQty(double value)
{
    return roundQty(value) == 0.0;
}

// a < b / a > b compared at 3-decimal precision
inline bool qtyLess(double a, double b)
{
    return roundQty(a) < roundQty(b);
}

inline bool qtyGreater(double a, double b)
{
    return roundQty(a) > roundQty(b);
}

// Format with at most 3 decimals and no trailing zeros:
// 12 -> "12", 0.5 -> "0.5", 2.125 -> "2.125", 1e6 -> "1000000"
inline std::string formatQty(double value)
{
    double r = roundQty(value);

    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << std::fixed << std::setprecision(QTY_DECIMALS) << r;

    std::string s = out.str();

    if (s.find('.') != std::string::npos)
    {
        while (!s.empty() && s.back() == '0')
        {
            s.pop_back();
        }

        if (!s.empty() && s.back() == '.')
        {
            s.pop_back();
        }
    }

    if (s.empty() || s == "-0")
    {
        s = "0";
    }

    return s;
}

// Parse a quantity from text (accepts "." or ","). Returns 0 when
// the text is empty or not numeric.
inline double parseQty(const std::string& text)
{
    std::string s = text;

    for (size_t i = 0; i < s.size(); i++)
    {
        if (s[i] == ',')
        {
            s[i] = '.';
        }
    }

    if (s.empty())
    {
        return 0.0;
    }

    char* end = nullptr;

    double v = std::strtod(s.c_str(), &end);

    if (end == s.c_str())
    {
        return 0.0;
    }

    return roundQty(v);
}
