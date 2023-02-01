#include <curvemanager/schemas/discountfactorsrequest.hpp>

namespace QuantLibParser
{
    template <>
    void Schema<DiscountFactorsRequest>::initSchema() {
        mySchema_ = readJSONFile("discountfactors.request.schema.json");
    };

    template <>
    void Schema<DiscountFactorsRequest>::initDefaultValues(){};

}  // namespace QuantLibParser
