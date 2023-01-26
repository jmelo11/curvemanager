
#include <curvemanager/schemas/updatequoterequest.hpp>

namespace QuantLibParser
{
    template <>
    void Schema<UpdateQuoteRequest>::initSchema() {
        mySchema_ = readJSONFile("updatequote.request.schema.json");
    };

    template <>
    void Schema<UpdateQuoteRequest>::initDefaultValues(){};

}  // namespace QuantLibParser
