#ifndef B69A940C_91B3_425C_9D96_DCBB613C09BC
#define B69A940C_91B3_425C_9D96_DCBB613C09BC

#include <qlp/schemas/commonschemas.hpp>
#include <qlp/schemas/schema.hpp>

namespace QuantLibParser
{
    class DiscountFactorsRequest;

    template <>
    void Schema<DiscountFactorsRequest>::initSchema();

    template <>
    void Schema<DiscountFactorsRequest>::initDefaultValues();

}  // namespace QuantLibParser

#endif /* B69A940C_91B3_425C_9D96_DCBB613C09BC */
