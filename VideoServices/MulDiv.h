#include <cstdint>

inline int MulDiv(int number, int multiplier, int divisor)
{
    // Compute with 64-bit to prevent overflow.
    // Adds (divisor/2) for rounding if divisor is positive.
    return static_cast<int>((static_cast<int64_t>(number) * multiplier + divisor/2) / divisor);
}
