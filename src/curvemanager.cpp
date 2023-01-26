
#include <curvemanager/curvemanager.hpp>
#include <curvemanager/schemas/all.hpp>
#include <qlp/schemas/ratehelpers/all.hpp>
#include <qlp/schemas/termstructures/all.hpp>

namespace CurveManager
{
    using namespace QuantExt;
    using namespace QuantLibParser;

    CurveBuilder::CurveBuilder(const json& data, MarketStore& marketStore) : data_(data), marketStore_(marketStore) {
        // validate the general input schema
        Schema<CurveBuilderRequest> schema;
        schema.validate(data_);
        if (!data_.empty()) preprocessData();
    };

    void CurveBuilder::preprocessData() {
        
        // build curves depending on the type
        Schema<DiscountCurve> discountCurveSchema;
        Schema<BootstrapCurve> bootstrapCurveSchema;
        Schema<FlatForward> flatForwardSchema;
        for (auto& curve : data_.at("curves")) {
            const std::string& name = curve.at("curveName");
            const auto& config      = curve.at("curveConfig");
            curveConfigs_[name]     = config;

            RelinkableHandle<YieldTermStructure> handle;
            marketStore_.addCurveHandle(name, handle);

            if (config.at("curveType") == "Discount") {
                auto curvePtr = boost::shared_ptr<YieldTermStructure>(new DiscountCurve(discountCurveSchema.makeObj(config)));
                handle.linkTo(curvePtr);
                curvePtr->unregisterWith(Settings::instance().evaluationDate());
                marketStore_.addCurve(name, curvePtr);
            }
            else if (config.at("curveType") == "FlatForward") {
                auto curvePtr = boost::shared_ptr<YieldTermStructure>(new FlatForward(flatForwardSchema.makeObj(config)));
                handle.linkTo(curvePtr);
                curvePtr->unregisterWith(Settings::instance().evaluationDate());
                marketStore_.addCurve(name, curvePtr);
            }
            else if (config.at("curveType") == "Piecewise") {
                bootstrapCurveSchema.setDefaultValues(config);
                bootstrapCurveSchema.validate(config);
            }
        }

        // build indexes
        Schema<IborIndex> iborSchema;
        Schema<OvernightIndex> overnightIndexSchema;

        CurveGetter curveGetter = [&](const std::string& name) { return marketStore_.getCurveHandle(name); };

        // build index depending on the type
        for (auto& indexParams : data_.at("indexes")) {
            const std::string& name = indexParams.at("indexName");
            if (!marketStore_.hasIndex(name)) {
                const std::string& indexType = indexParams.at("indexType");
                indexConfigs_[name]          = indexParams;
                if (indexType == "OvernightIndex") {
                    auto index = boost::shared_ptr<IborIndex>(new OvernightIndex(overnightIndexSchema.makeObj(indexParams, curveGetter)));
                    marketStore_.addIndex(name, index);
                }
                else if (indexType == "IborIndex") {
                    auto index = boost::make_shared<IborIndex>(iborSchema.makeObj(indexParams, curveGetter));
                    marketStore_.addIndex(name, index);
                }
                else {
                    std::string errorJson = indexParams.dump();
                    throw std::runtime_error("Index type " + indexType + " not supported \n" + errorJson);
                }
            }
        }
    }

    void CurveBuilder::build() {
        const std::string& refDate            = data_.at("refDate");
        Settings::instance().evaluationDate() = parse<Date>(refDate);
        for (const auto& [name, curve] : curveConfigs_) buildCurve(name, curve);
    };

    void CurveBuilder::buildCurve(const std::string& curveName, const json& curveParams) {
        const std::string& curveType = curveParams.at("curveType");
        if (!marketStore_.hasCurve(curveName) && curveType == "Piecewise") {
            boost::shared_ptr<YieldTermStructure> curvePtr;

            curvePtr = buildPiecewiseCurve(curveName, curveParams);

            bool enableExtrapolation = curveParams.at("enableExtrapolation");
            if (enableExtrapolation) curvePtr->enableExtrapolation();
            RelinkableHandle<YieldTermStructure>& handle = marketStore_.getCurveHandle(curveName);
            handle.linkTo(curvePtr);

            curvePtr->unregisterWith(Settings::instance().evaluationDate());
            marketStore_.addCurve(curveName, curvePtr);
        }
    }

    boost::shared_ptr<YieldTermStructure> CurveBuilder::buildPiecewiseCurve(const std::string& curveName, const json& curveParams) {
        auto helpers          = buildRateHelpers(curveParams.at("rateHelpers"), curveName);
        DayCounter dayCounter = parse<DayCounter>(curveParams.at("dayCounter"));
        Date qlRefDate        = Settings::instance().evaluationDate();
        boost::shared_ptr<YieldTermStructure> curvePtr(new PiecewiseYieldCurve<Discount, LogLinear>(qlRefDate, helpers, dayCounter));
        return curvePtr;
    };

    void CurveBuilder::updateQuotes(const json& prices) {
        Schema<UpdateQuoteRequest> schema;
        schema.validate(prices);
        for (const auto& pair : prices) {
            std::string ticker = pair.at("ticker");
            if (!marketStore_.hasQuote(ticker)) {
                throw std::runtime_error("No quote found for " + ticker);
            }
        }

        marketStore_.freeze();
        for (const auto& pair : prices) {
            Handle<Quote> handle                 = marketStore_.getQuote(pair.at("ticker"));
            boost::shared_ptr<SimpleQuote> quote = boost::static_pointer_cast<SimpleQuote>(handle.currentLink());
            quote->setValue(pair.at("value"));
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
            if (marketStore_.getCurveHandle(curveName).empty())
                throw std::runtime_error("An error happened: handle for curve " + curveName + " is still empty.");
            return marketStore_.getCurveHandle(curveName);
        };

        std::vector<boost::shared_ptr<RateHelper>> rateHelpers;
        rateHelpers.reserve(rateHelperVector.size());
        size_t rateHelperCount = 0;
        for (auto const& helperParams : rateHelperVector) {
            const std::string& type = helperParams.at("helperType");
            boost::shared_ptr<RateHelper> helper;
            try {
                if (type == "Deposit") {
                    Schema<DepositRateHelper> schema;
                    helper = boost::make_shared<DepositRateHelper>(schema.makeObj(helperParams, priceGetter));
                }
                else if (type == "FxSwap") {
                    Schema<FxSwapRateHelper> schema;
                    helper = boost::make_shared<FxSwapRateHelper>(schema.makeObj(helperParams, priceGetter, curveGetter));
                }
                else if (type == "Bond") {
                    Schema<FixedRateBondHelper> schema;
                    helper = boost::make_shared<FixedRateBondHelper>(schema.makeObj(helperParams, priceGetter));
                }
                else if (type == "Swap") {
                    Schema<SwapRateHelper> schema;
                    helper = boost::make_shared<SwapRateHelper>(schema.makeObj(helperParams, priceGetter, indexGetter, curveGetter));
                }
                else if (type == "OIS") {
                    Schema<OISRateHelper> schema;
                    helper = boost::make_shared<OISRateHelper>(schema.makeObj(helperParams, priceGetter, indexGetter, curveGetter));
                }
                else if (type == "Xccy") {
                    Schema<CrossCcyFixFloatSwapHelper> schema;
                    helper = boost::make_shared<CrossCcyFixFloatSwapHelper>(schema.makeObj(helperParams, priceGetter, indexGetter, curveGetter));
                }
                else if (type == "XccyBasis") {
                    Schema<CrossCcyBasisSwapHelper> schema;
                    helper = boost::make_shared<CrossCcyBasisSwapHelper>(schema.makeObj(helperParams, priceGetter, indexGetter, curveGetter));
                }
                else if (type == "TenorBasis") {
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

}  // namespace CurveManager