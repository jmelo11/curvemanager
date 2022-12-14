
#include <curvemanager/marketstore.hpp>
#include <curvemanager/schemas/all.hpp>
#include <qlp/parser.hpp>

namespace CurveManager
{
    using namespace QuantLibParser;

    MarketStore::MarketStore(){};

    boost::shared_ptr<YieldTermStructure> MarketStore::getCurve(const std::string& name) const {
        if (hasCurve(name)) return curveMap_.at(name);
        throw std::runtime_error("Curve not found: " + name);
    };

    boost::shared_ptr<IborIndex> MarketStore::getIndex(const std::string& name) const {
        if (hasIndex(name)) return indexMap_.at(name);
        throw std::runtime_error("Index not found: " + name);
    }

    Handle<Quote>& MarketStore::getQuote(const std::string& ticker) {
        if (hasQuote(ticker)) return quoteMap_.at(ticker);
        throw std::runtime_error("Index not found: " + ticker);
    }

    RelinkableHandle<YieldTermStructure>& MarketStore::getCurveHandle(const std::string& name) {
        if (hasCurveHandle(name)) return curveHandleMap_.at(name);
        throw std::runtime_error("Curve handle not found: " + name);
    }

    bool MarketStore::hasCurve(const std::string& name) const {
        if (curveMap_.find(name) != curveMap_.end()) return true;
        return false;
    }

    bool MarketStore::hasIndex(const std::string& name) const {
        if (indexMap_.find(name) != indexMap_.end()) return true;
        return false;
    }
    bool MarketStore::hasCurveHandle(const std::string& name) const {
        return curveHandleMap_.find(name) != curveHandleMap_.end();
    }

    bool MarketStore::hasQuote(const std::string& ticker) const {
        return quoteMap_.find(ticker) != quoteMap_.end();
    }

    void MarketStore::addFixing(const std::string& name, const Date& date, double fixing) {
        indexMap_[name]->addFixing(date, fixing, true);
    }

    void MarketStore::addCurve(const std::string& name, boost::shared_ptr<YieldTermStructure>& curve) {
        curveMap_[name] = curve;
    }

    void MarketStore::addIndex(const std::string& name, boost::shared_ptr<IborIndex>& index) {
        indexMap_[name] = index;
    }

    void MarketStore::addQuote(const std::string& ticker, Handle<Quote>& handle) {
        quoteMap_.insert({ticker, handle});
    }

    void MarketStore::addCurveHandle(const std::string& name, RelinkableHandle<YieldTermStructure>& handle) {
        curveHandleMap_.insert({name, handle});
    }

    void MarketStore::freeze() {
        for (const auto& [name, curve] : curveMap_) {
            auto ptr = boost::dynamic_pointer_cast<PiecewiseYieldCurve<Discount, LogLinear>>(curve);
            if (ptr) ptr->freeze();
        }
    }

    void MarketStore::unfreeze() {
        for (const auto& [name, curve] : curveMap_) {
            auto ptr = boost::dynamic_pointer_cast<PiecewiseYieldCurve<Discount, LogLinear>>(curve);
            if (ptr) ptr->unfreeze();
        }
    }

    std::vector<std::string> MarketStore::allCurves() const {
        std::vector<std::string> names;
        for (const auto& [name, curve] : curveMap_) names.push_back(name);
        return names;
    }

    std::vector<std::string> MarketStore::allIndexes() const {
        std::vector<std::string> names;
        for (const auto& [name, index] : indexMap_) names.push_back(name);
        return names;
    }

    json MarketStore::bootstrapResults() const {
        std::vector<Date> qlDates;
        json results = json::array();

        for (const auto& [name, curve] : curveMap_) {
            auto ptr = boost::dynamic_pointer_cast<PiecewiseYieldCurve<Discount, LogLinear>>(curve);
            if (ptr) {
                json data;
                auto nodes   = ptr->nodes();
                data["NAME"] = name;
                std::vector<double> values;
                std::vector<std::string> dates;
                for (const auto& pair : nodes) {
                    dates.push_back(parseDate(pair.first, DateFormat::MIXED));
                    values.push_back(pair.second);
                }
                data["DATES"]  = dates;
                data["VALUES"] = values;
                results.push_back(data);
            }
        }
        return results;
    }

    json MarketStore::discountRequest(const json& request) const {
        //shoulnt require ref date (not the same for the microservice)
        Schema<DiscountFactorsRequest> schema;
        schema.validate(request);
        json data = schema.setDefaultValues(request);

        auto curve = getCurve(data.at("CURVE"));

        json response = json::array();
        for (const auto& date : data.at("DATES")) {
            json row;
            row["DATE"]  = date;
            row["VALUE"] = curve->discount(parse<Date>(date));
            response.push_back(row);
        }
        return response;
    }

    json MarketStore::zeroRateRequest(const json& request) const {
        Schema<ZeroRatesRequests> schema;
        schema.validate(request);
        json data = schema.setDefaultValues(request);

        auto curve            = getCurve(data.at("CURVE"));
        DayCounter dayCounter = parse<DayCounter>(data.at("DAYCOUNTER"));
        Compounding comp      = parse<Compounding>(data.at("COMPOUNDING"));
        Frequency freq        = parse<Frequency>(data.at("FREQUENCY"));
        json response         = json::array();
        for (const auto& date : data.at("DATES")) {
            json row;
            row["DATE"]  = date;
            row["VALUE"] = curve->discount(parse<Date>(date));
            response.push_back(row);
        }
        return response;
    }

    json MarketStore::forwardRateRequest(const json& request) const {
        Schema<ForwardRatesRequest> schema;
        schema.validate(request);
        json data = schema.setDefaultValues(request);

        auto curve            = getCurve(data.at("CURVE"));
        DayCounter dayCounter = parse<DayCounter>(data.at("DAYCOUNTER"));
        Compounding comp      = parse<Compounding>(data.at("COMPOUNDING"));
        Frequency freq        = parse<Frequency>(data.at("FREQUENCY"));
        json response;
        response["VALUES"] = json::array();
        for (const auto& dates : data.at("DATES")) {
            auto startDate = parse<Date>(dates[0]);
            auto endDate   = parse<Date>(dates[1]);
            response["VALUES"].push_back(curve->forwardRate(startDate, endDate, dayCounter, comp, freq).rate());
        }
        response["DATES"] = request["DATES"];
        return response;
    }
}  // namespace CurveManager
