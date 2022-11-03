/*
 * Created on Sat Oct 29 2022
 *
 * Jose Melo - 2022
 */

#include <qlp/schemas/requests/curvebuilderrequest.hpp>
#include <qlp/schemas/requests/updatequoterequest.hpp>
#include <qlp/schemas/termstructures/bootstrapcurveschema.hpp>
#include <qlp/schemas/termstructures/discountcurveschema.hpp>
#include <qlp/schemas/termstructures/flatforwardcurveschema.hpp>
#include <qlp/schemas/termstructures/rateindexschema.hpp>
#include <ql/termstructures/yield/discountcurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <curvemanager/curvemanager.hpp>

namespace CurveManager
{
    CurveBuilder::CurveBuilder(const json& data, MarketStore& marketStore) : data_(data), marketStore_(marketStore) {
        Schema<CurveBuilderRequest> schema;  // ok
        schema.validate(data);
        if (!data_.empty()) preprocessData();
    };

    void CurveBuilder::preprocessData() {
        Schema<DiscountCurve> discountCurveSchema;
        Schema<FlatForward> flatForwardSchema;
        Schema<PiecewiseYieldCurve<Discount, LogLinear>> bootstrapCurveSchema;

        Schema<YieldTermStructure> termStructureSchema;
        for (auto& curve : data_.at("CURVES")) {
            termStructureSchema.validate(curve);
            if (curve.at("TYPE") == "DISCOUNT") {
                discountCurveSchema.setDefaultValues(curve);
                discountCurveSchema.validate(curve);
            }
            else if (curve.at("TYPE") == "FLATFORWARD") {
                flatForwardSchema.setDefaultValues(curve);
                flatForwardSchema.validate(curve);
            }
            else if (curve.at("TYPE") == "PIECEWISE") {
                bootstrapCurveSchema.setDefaultValues(curve);
                bootstrapCurveSchema.validate(curve);
            }
            const std::string& name = curve.at("NAME");
            curveConfigs_[name]     = curve;
            RelinkableHandle<YieldTermStructure> handle;
            marketStore_.addCurveHandle(name, handle);
        }
        Schema<InterestRateIndex> indexSchema;
        for (auto& index : data_.at("INDEXES")) {
            indexSchema.validate(index);
            indexSchema.setDefaultValues(index);

            const std::string& name = index.at("NAME");
            indexConfigs_[name]     = index;
            buildIndex(name);
        }
    }

    void CurveBuilder::build() {
        const std::string& refDate            = data_.at("REFDATE");
        Settings::instance().evaluationDate() = parse<Date>(refDate);
        for (const auto& [name, curve] : curveConfigs_) buildCurve(name, curve);
    };

    void CurveBuilder::buildCurve(const std::string& curveName, const json& curveParams) {
        if (!marketStore_.hasCurve(curveName)) {
            const std::string& curveType = curveParams.at("TYPE");
            boost::shared_ptr<YieldTermStructure> curvePtr;
            if (curveType == "DISCOUNT") {
                curvePtr = buildDiscountCurve(curveName, curveParams);
            }
            else if (curveType == "PIECEWISE") {
                curvePtr = buildPiecewiseCurve(curveName, curveParams);
            }
            else if (curveType == "FLATFORWARD") {
                curvePtr = buildFlatForwardCurve(curveName, curveParams);
            }
            bool enableExtrapolation = curveParams.at("ENABLEEXTRAPOLATION");
            if (enableExtrapolation) curvePtr->enableExtrapolation();
            RelinkableHandle<YieldTermStructure>& handle = marketStore_.getCurveHandle(curveName);
            handle.linkTo(curvePtr);

            curvePtr->unregisterWith(Settings::instance().evaluationDate());
            marketStore_.addCurve(curveName, curvePtr);
        }
    }

    boost::shared_ptr<YieldTermStructure> CurveBuilder::buildPiecewiseCurve(const std::string& curveName, const json& curveParams) {
        auto helpers          = buildRateHelpers(curveParams.at("RATEHELPERS"), curveName);
        DayCounter dayCounter = parse<DayCounter>(curveParams.at("DAYCOUNTER"));
        Date qlRefDate        = Settings::instance().evaluationDate();
        boost::shared_ptr<YieldTermStructure> curvePtr(new PiecewiseYieldCurve<Discount, LogLinear>(qlRefDate, helpers, dayCounter));
        return curvePtr;
    };

    boost::shared_ptr<YieldTermStructure> CurveBuilder::buildDiscountCurve(const std::string& curveName, const json& curveParams) {
        Date qlRefDate    = Settings::instance().evaluationDate();
        const json& nodes = curveParams.at("NODES");
        std::vector<Date> dates;
        std::vector<double> dfs;
        for (const auto& node : nodes) {
            dates.push_back(parse<Date>(node.at("DATE")));
            dfs.push_back(node.at("VALUE"));
        }
        if (qlRefDate != dates[0])
            throw std::runtime_error("Error building curve" + curveName +
                                     ": Reference date (REFDATE) must be equal to the first node date (NODES/DATES) in the curve.");

        DayCounter dayCounter = parse<DayCounter>(curveParams.at("DAYCOUNTER"));
        boost::shared_ptr<YieldTermStructure> curvePtr(new DiscountCurve(dates, dfs, dayCounter));
        return curvePtr;
    };

    boost::shared_ptr<YieldTermStructure> CurveBuilder::buildFlatForwardCurve(const std::string& curveName, const json& curveParams) {
        DayCounter dayCounter   = parse<DayCounter>(curveParams.at("DAYCOUNTER"));
        Compounding compounding = parse<Compounding>(curveParams.at("COMPOUNDING"));
        Frequency frequency     = parse<Frequency>(curveParams.at("FREQUENCY"));
        double rate             = curveParams.at("RATE");
        Date qlRefDate          = Settings::instance().evaluationDate();
        boost::shared_ptr<YieldTermStructure> curvePtr(new FlatForward(qlRefDate, rate, dayCounter, compounding, frequency));
        return curvePtr;
    };

    void CurveBuilder::updateQuotes(const json& prices) {
        Schema<UpdateQuoteRequest> schema;
        schema.validate(prices);
        for (const auto& pair : prices) {
            std::string curveName = pair.at("NAME");
            if (!marketStore_.hasQuote(curveName)) {
                throw std::runtime_error("No quote found for " + curveName);
            }
        }

        marketStore_.freeze();
        for (const auto& pair : prices) {
            Handle<Quote> handle                 = marketStore_.getQuote(pair.at("NAME"));
            boost::shared_ptr<SimpleQuote> quote = boost::static_pointer_cast<SimpleQuote>(handle.currentLink());
            quote->setValue(pair["VALUE"]);
        }
        marketStore_.unfreeze();
    }

    std::vector<boost::shared_ptr<RateHelper>> CurveBuilder::buildRateHelpers(const json& rateHelperVector, const std::string& currentCurve) {
        std::vector<boost::shared_ptr<RateHelper>> rateHelpers;
        rateHelpers.reserve(rateHelperVector.size());

        auto priceGetter = [&](double price, const std::string& ticker) {
            if (!marketStore_.hasQuote(ticker)) {
                boost::shared_ptr<Quote> quote(new SimpleQuote(price));
                Handle<Quote> handle(quote);
                marketStore_.addQuote(ticker, handle);
                return handle;
            }
            else {
                return marketStore_.getQuote(ticker);
            }
        };

        auto indexGetter = [&](const std::string& indexName) {
            if (currentCurve != indexName) buildCurve(indexName, curveConfigs_.at(indexName));
            return marketStore_.getIndex(indexName);
        };

        auto curveGetter = [&](const std::string& curveName) {
            if (currentCurve != curveName) buildCurve(curveName, curveConfigs_.at(curveName));
            return marketStore_.getCurveHandle(curveName);
        };

        for (auto const& helper : rateHelperVector) {
            if (helper.find("TYPE") == helper.end()) throw std::runtime_error("Rate helper type not specified: " + helper.dump(4));
            const std::string& type = helper.at("TYPE");

            if (type == "DEPOSIT") {
                boost::shared_ptr<DepositRateHelper> h = JsonToObjectWrapper<DepositRateHelper>(helper, priceGetter);
                h->unregisterWith(Settings::instance().evaluationDate());
                rateHelpers.push_back(h);
            }
            else if (type == "FXSWAP") {
                auto h = JsonToObjectWrapper<FxSwapRateHelper>(helper, priceGetter, curveGetter);
                h->unregisterWith(Settings::instance().evaluationDate());
                rateHelpers.push_back(h);
            }
            else if (type == "BOND") {
                auto h = JsonToObjectWrapper<FixedRateBondHelper>(helper, priceGetter);
                h->unregisterWith(Settings::instance().evaluationDate());
                rateHelpers.push_back(h);
            }
            else if (type == "SWAP") {
                auto h = JsonToObjectWrapper<SwapRateHelper>(helper, priceGetter, indexGetter, curveGetter);
                h->unregisterWith(Settings::instance().evaluationDate());
                rateHelpers.push_back(h);
            }
            else if (type == "OIS") {
                auto h = JsonToObjectWrapper<OISRateHelper>(helper, priceGetter, indexGetter, curveGetter);
                h->unregisterWith(Settings::instance().evaluationDate());
                rateHelpers.push_back(h);
            }
            else if (type == "XCCY") {
                auto h = JsonToObjectWrapper<CrossCcyFixFloatSwapHelper>(helper, priceGetter, indexGetter, curveGetter);
                h->unregisterWith(Settings::instance().evaluationDate());
                rateHelpers.push_back(h);
            }
            else if (type == "XCCYBASIS") {
                auto h = JsonToObjectWrapper<CrossCcyBasisSwapHelper>(helper, priceGetter, indexGetter, curveGetter);
                h->unregisterWith(Settings::instance().evaluationDate());
                rateHelpers.push_back(h);
            }
            else if (type == "TENORBASIS") {
                auto h = JsonToObjectWrapper<TenorBasisSwapHelper>(helper, priceGetter, indexGetter, curveGetter);
                h->unregisterWith(Settings::instance().evaluationDate());
                rateHelpers.push_back(h);
            }
            else {
                throw std::runtime_error("Helper of type " + type + " not supported");
            }
        }
        return rateHelpers;
    };

    boost::shared_ptr<IborIndex> CurveBuilder::buildIndex(const std::string& name) {
        if (!marketStore_.hasIndex(name)) {
            const json& config      = indexConfigs_.at(name);
            const std::string& type = config.at("TYPE");
            Period tenor            = parse<Period>(config.at("TENOR"));
            DayCounter dayCounter   = parse<DayCounter>(config.at("DAYCOUNTER"));
            Currency currency       = parse<Currency>(config.at("CURRENCY"));
            Calendar calendar       = parse<Calendar>(config.at("CALENDAR"));
            int fixingDays          = config.at("FIXINGDAYS");

            BusinessDayConvention convention = BusinessDayConvention::Unadjusted;
            bool endOfMonth                  = false;
            auto& curveHandle                = marketStore_.getCurveHandle(name);
            if (type == "OVERNIGHT") {
                boost::shared_ptr<IborIndex> index(new OvernightIndex(name, fixingDays, currency, calendar, dayCounter, curveHandle));
                marketStore_.addCurveHandle(name, curveHandle);
                marketStore_.addIndex(name, index);
            }
            else if (type == "IBOR") {
                boost::shared_ptr<IborIndex> index(
                    new IborIndex(name, tenor, fixingDays, currency, calendar, convention, endOfMonth, dayCounter, curveHandle));
                marketStore_.addCurveHandle(name, curveHandle);
                marketStore_.addIndex(name, index);
            }
            else {
                std::string errorJson = config.dump();
                throw std::runtime_error("Index type " + type + " not supported \n" + errorJson);
            }
        }
        return marketStore_.getIndex(name);
    };

}  // namespace CurveManager