#ifndef CA442F25_7871_4CAA_BA17_54EED273907D
#define CA442F25_7871_4CAA_BA17_54EED273907D

#include <qlp/schemas/schema.hpp>

namespace QuantLibParser
{
    class CurveBuilderRequest;

    template <>
    void Schema<CurveBuilderRequest>::initSchema();
    
    template <>
    void Schema<CurveBuilderRequest>::initDefaultValues();

}  // namespace QuantLibParser

#endif /* CA442F25_7871_4CAA_BA17_54EED273907D */
