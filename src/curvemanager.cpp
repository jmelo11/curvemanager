
#include <curvemanager/curvemanager.hpp>
#include <curvemanager/schemas/all.hpp>
#include <qlp/schemas/ratehelpers/all.hpp>
#include <qlp/schemas/termstructures/all.hpp>

namespace CurveManager
{
    using namespace QuantExt;
    using namespace QuantLibParser;

    CurveBuilder::CurveBuilder(const json& data, MarketStore& marketStore) : data_(data), marketStore_(marketStore) {
        Schema<CurveBuilderRequest> schema;
        schema.validate(data_);
        if (!data_.empty()) preprocessData();
    };

    void CurveBuilder::preprocessData() {
        json curveValidation = R"({
            "title": "Curve type",
            "type": "object",
            "properties": {},
            "required": ["TYPE", "NAME"]
            })"_json;

        curveValidation["properties"]["TYPE"] = curveTypeSchema;
        curveValidation["properties"]["NAME"] = curveNameSchema;
        json_validator validator;

        Schema<DiscountCurve> discountCurveSchema;
        Schema<BootstrapCurve> bootstrapCurveSchema;
        Schema<FlatForward> flatForwardSchema;

        Schema<YieldTermStructure> termStructureSchema;
        for (auto& curve : data_.at("CURVES")) {
            try {
                validator.set_root_schema(curveValidation);  // insert root-schema
                validator.validate(curve);
            }
            catch (const std::exception& e) {
                std::string error = e.what();
                throw std::runtime_error("Validation of schema failed:\t" + error + "\n");
            }

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
        PriceGetter priceGetter = [&](double price, const std::string& ticker) {
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

        IndexGetter indexGetter = [&](const std::string& indexName) {
            if (currentCurve != indexName) buildCurve(indexName, curveConfigs_.at(indexName));
            return marketStore_.getIndex(indexName);
        };

        CurveGetter curveGetter = [&](const std::string& curveName) {
            if (currentCurve != curveName) buildCurve(curveName, curveConfigs_.at(curveName));
            return marketStore_.getCurveHandle(curveName);
        };

        std::vector<boost::shared_ptr<RateHelper>> rateHelpers;
        rateHelpers.reserve(rateHelperVector.size());
        size_t rateHelperCount = 0;
        for (auto const& helperParams : rateHelperVector) {
            const std::string& type = helperParams.at("TYPE");
            boost::shared_ptr<RateHelper> helper;
            try {
                if (type == "DEPOSIT") {
                    Schema<DepositRateHelper> schema;
                    helper = boost::make_shared<DepositRateHelper>(schema.makeObj(helperParams, priceGetter));
                }
                else if (type == "FXSWAP") {
                    Schema<FxSwapRateHelper> schema;
                    helper = boost::make_shared<FxSwapRateHelper>(schema.makeObj(helperParams, priceGetter, curveGetter));
                }
                else if (type == "BOND") {
                    Schema<FixedRateBondHelper> schema;
                    helper = boost::make_shared<FixedRateBondHelper>(schema.makeObj(helperParams, priceGetter));
                }
                else if (type == "SWAP") {
                    Schema<SwapRateHelper> schema;
                    helper = boost::make_shared<SwapRateHelper>(schema.makeObj(helperParams, priceGetter, indexGetter, curveGetter));
                }
                else if (type == "OIS") {
                    Schema<OISRateHelper> schema;
                    helper = boost::make_shared<OISRateHelper>(schema.makeObj(helperParams, priceGetter, indexGetter, curveGetter));
                }
                else if (type == "XCCY") {
                    Schema<CrossCcyFixFloatSwapHelper> schema;
                    helper = boost::make_shared<CrossCcyFixFloatSwapHelper>(schema.makeObj(helperParams, priceGetter, indexGetter, curveGetter));
                }
                else if (type == "XCCYBASIS") {
                    Schema<CrossCcyBasisSwapHelper> schema;
                    helper = boost::make_shared<CrossCcyBasisSwapHelper>(schema.makeObj(helperParams, priceGetter, indexGetter, curveGetter));
                }
                else if (type == "TENORBASIS") {
                    Schema<TenorBasisSwapHelper> schema;
                    helper = boost::make_shared<TenorBasisSwapHelper>(schema.makeObj(helperParams, priceGetter, indexGetter, curveGetter));
                }
                else {
                    throw std::runtime_error("Helper of type " + type + " not supported");
                }
                helper->unregisterWith(Settings::instance().evaluationDate());
                rateHelpers.push_back(helper);
            }
            catch (std::exception& e) {
                throw std::runtime_error("Error at curve " + currentCurve + ", rate helper pos " + std::to_string(rateHelperCount) + ":\n" +
                                         e.what());
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