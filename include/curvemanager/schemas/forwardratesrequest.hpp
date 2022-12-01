#ifndef DF7A702E_6E4D_4BBD_8590_3D0EBFDC3C91
#define DF7A702E_6E4D_4BBD_8590_3D0EBFDC3C91

#include <qlp/schemas/commonschemas.hpp>
#include <qlp/schemas/schema.hpp>

namespace QuantLibParser
{
    class ForwardRatesRequest;

    template <>
    void Schema<ForwardRatesRequest>::initSchema();

    template <>
    void Schema<ForwardRatesRequest>::initDefaultValues();

}  // namespace QuantLibParser

#endif /* DF7A702E_6E4D_4BBD_8590_3D0EBFDC3C91 */
