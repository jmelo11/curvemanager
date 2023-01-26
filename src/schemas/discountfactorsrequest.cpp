#include <curvemanager/schemas/discountfactorsrequest.hpp>
#include <qlp/schemas/commonschemas.hpp>

namespace QuantLibParser
{
    template <>
    void Schema<DiscountFactorsRequest>::initSchema() {
        mySchema_ = readJSONFile("discountfactors.request.schema.json");
    };

    template <>
    void Schema<DiscountFactorsRequest>::initDefaultValues(){};

}  // namespace QuantLibParser
