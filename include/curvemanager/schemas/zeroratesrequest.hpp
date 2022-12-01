#ifndef CD5B3844_BD17_4F3F_BC6B_38C6162936B5
#define CD5B3844_BD17_4F3F_BC6B_38C6162936B5

#include <qlp/schemas/commonschemas.hpp>
#include <qlp/schemas/schema.hpp>

namespace QuantLibParser
{
    class ZeroRatesRequests;

    template <>
    void Schema<ZeroRatesRequests>::initSchema();

    template <>
    void Schema<ZeroRatesRequests>::initDefaultValues();

}  // namespace QuantLibParser

#endif /* CD5B3844_BD17_4F3F_BC6B_38C6162936B5 */
