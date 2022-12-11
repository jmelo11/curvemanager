/*
 * Created on Sat Oct 29 2022
 *
 * Jose Melo - 2022
 */

#ifndef A02E616D_E693_447C_B341_9A3B4E69200A
#define A02E616D_E693_447C_B341_9A3B4E69200A

#include <curvemanager/marketstore.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <iostream>
#include <map>
#include <stdexcept>

namespace CurveManager
{
    using namespace QuantLib;
    using json = nlohmann::json;

    class CurveBuilder {
       public:
        CurveBuilder(const json& data, MarketStore& marketStore);
        void build();
        void updateQuotes(const json& prices);

       private:
        void preprocessData();
        void buildCurve(const std::string& name, const json& curve);
        boost::shared_ptr<YieldTermStructure> buildPiecewiseCurve(const std::string& name, const json& curve);
        std::vector<boost::shared_ptr<RateHelper>> buildRateHelpers(const json& rateHelperVector, const std::string& currentCurve);
        boost::shared_ptr<IborIndex> buildIndex(const std::string& name);

        json data_;
        MarketStore& marketStore_;
        std::unordered_map<std::string, json> curveConfigs_;
        std::unordered_map<std::string, json> indexConfigs_;
    };
}  // namespace CurveManager
#endif /* A02E616D_E693_447C_B341_9A3B4E69200A */
