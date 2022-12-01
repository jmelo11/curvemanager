#ifndef F932C702_BF27_4D1A_9EBD_474A99C7A4DB
#define F932C702_BF27_4D1A_9EBD_474A99C7A4DB

#include <qlp/schemas/schema.hpp>

namespace QuantLibParser
{
    class UpdateQuoteRequest;

    template <>
    void Schema<UpdateQuoteRequest>::initSchema();

    template <>
    void Schema<UpdateQuoteRequest>::initDefaultValues();

}  // namespace QuantLibParser

#endif /* F932C702_BF27_4D1A_9EBD_474A99C7A4DB */
