
#include <curvemanager.hpp>
#include <parser.hpp>
#include <schemas/termstructures/rateindexschema.hpp>
#include <schemas/termstructures/yieldtermstructureschema.hpp>

namespace CurveManager {
		
	CurveBuilder::CurveBuilder(const json& data, MarketStore& marketStore): data_(data), marketStore_(marketStore) {
		if (!data_.empty())
			preprocessData();
		buildAllCurves();
	};

	void CurveBuilder::preprocessData() {		
		Schema<YieldTermStructure> curveSchema;
		for (auto& curve : data_.at("CURVES")) {
			try
			{
				curveSchema.validate(curve);
			}
			catch (const std::exception& e)
			{
				std::string error= e.what();
				throw std::runtime_error(error);
			}			
			curveSchema.setDefaultValues(curve);
			const std::string& name = curve.at("NAME");
			curveConfigs_[name] = curve;
			RelinkableHandle<YieldTermStructure> handle;
			marketStore_.addCurveHandle(name, handle);
		}
		Schema<IborIndex> indexSchema;
		for (auto& index : data_.at("INDICES")) {
			try
			{
				indexSchema.validate(index);
			}
			catch (const std::exception& e)
			{
				std::string error = e.what();
				throw std::runtime_error(error);
			}
			indexSchema.setDefaultValues(index);

			const std::string& name = index.at("NAME");			
			indexConfigs_[name] = index;
			buildIndex(name);
		}
	}

	void CurveBuilder::buildAllCurves() {
		const std::string& refDate = data_.at("REFDATE");
		Settings::instance().evaluationDate() = parse<Date>(refDate);
		for (const auto& [name, curve] : curveConfigs_)
			buildPiecewiseCurve(name, curve);
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
			if (enableExtrapolation)
				curvePtr->enableExtrapolation();
			RelinkableHandle<YieldTermStructure>& handle = marketStore_.getCurveHandle(curveName);
			handle.linkTo(curvePtr);

			curvePtr->unregisterWith(Settings::instance().evaluationDate());
			marketStore_.addCurve(curveName, curvePtr);
		}
	}

	boost::shared_ptr<YieldTermStructure> CurveBuilder::buildPiecewiseCurve(const std::string& curveName, const json& curveParams) {		
		auto helpers = buildRateHelpers(curveParams.at("RATEHELPERS"), curveName);
		DayCounter dayCounter = parse<DayCounter>(curveParams.at("DAYCOUNTER"));			
		Date qlRefDate = Settings::instance().evaluationDate();
		boost::shared_ptr<YieldTermStructure> curvePtr(new LogLinearPiecewiseCurve(qlRefDate, helpers, dayCounter));
		return curvePtr;		
	};
	
	boost::shared_ptr<YieldTermStructure> CurveBuilder::buildDiscountCurve(const std::string& curveName, const json& curveParams) {
		const json& nodes = curveParams.at("NODES");
		std::vector<Date> dates;
		std::vector<double> dfs;
		for (auto& [k, v] : nodes.items()) {
			dates.push_back(parse<Date>(k));
			dfs.push_back(v);
		}
		DayCounter dayCounter = parse<DayCounter>(curveParams.at("DAYCOUNTER"));
		boost::shared_ptr<YieldTermStructure> curvePtr(new DiscountCurve(dates, dfs, dayCounter));
		return curvePtr;
	};

	
	boost::shared_ptr<YieldTermStructure> CurveBuilder::buildFlatForwardCurve(const std::string& curveName, const json& curveParams) {
		DayCounter dayCounter = parse<DayCounter>(curveParams.at("DAYCOUNTER"));
		Compounding compounding = parse<Compounding>(curveParams.at("COMPOUNDING"));
		Frequency frequency = parse<Frequency>(curveParams.at("FREQUENCY"));
		double rate = curveParams.at("RATE");
		Date qlRefDate = Settings::instance().evaluationDate();
		boost::shared_ptr<YieldTermStructure> curvePtr(new FlatForward(qlRefDate, rate, dayCounter, compounding, frequency));
		return curvePtr;
	};
	

	void CurveBuilder::updateQuotes(const json& prices) {
		for (const auto& [ticker, price] : prices.items())
			if (!marketStore_.hasQuote(ticker)) throw std::runtime_error("No quote found for " + ticker);

		marketStore_.freeze();
		for (const auto& [ticker, price] : prices.items()) {
			Handle<Quote> handle = marketStore_.getQuote(ticker);
			boost::shared_ptr<SimpleQuote> quote = boost::static_pointer_cast<SimpleQuote>(handle.currentLink());
			quote->setValue(price);
		}
		marketStore_.unfreeze();
	}

	std::vector<boost::shared_ptr<RateHelper>> CurveBuilder::buildRateHelpers(const json& rateHelperVector, const std::string& currentCurve) {
		std::vector<boost::shared_ptr<RateHelper>> rateHelpers;
		rateHelpers.reserve(rateHelperVector.size());

		auto priceGetter = [&](double price, const std::string& ticker) {
			if (!marketStore_.hasQuote(ticker)) {
				boost::shared_ptr<Quote> quote(new SimpleQuote(price));
				Handle<Quote>handle(quote);
				marketStore_.addQuote(ticker, handle);
				return handle;
			}
			else {
				return marketStore_.getQuote(ticker);
			}
		};

		auto indexGetter = [&](const std::string& indexName) {
			if (currentCurve != indexName)
				buildCurve(indexName, curveConfigs_.at(indexName));
			return marketStore_.getIndex(indexName);
		};

		auto curveGetter = [&](const std::string& curveName) {
			if (currentCurve != curveName)
				buildCurve(curveName, curveConfigs_.at(curveName));
			return marketStore_.getCurveHandle(curveName);
		};

		for (auto const& helper : rateHelperVector) {
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
		if (!marketStore_.hasIndex(name))
		{			
			const json& config = indexConfigs_.at(name);
			const std::string& type = config.at("TYPE");
			Period tenor = parse<Period>(config.at("TENOR"));
			DayCounter dayCounter = parse<DayCounter>(config.at("DAYCOUNTER"));
			Currency currency = parse<Currency>(config.at("CURRENCY"));
			Calendar calendar = parse<Calendar>(config.at("CALENDAR"));
			int fixingDays = config.at("FIXINGDAYS");

			BusinessDayConvention convention = BusinessDayConvention::Unadjusted;
			bool endOfMonth = false;
			auto& curveHandle = marketStore_.getCurveHandle(name);
			if (type == "OVERNIGHT")
			{
				boost::shared_ptr<IborIndex>index(new OvernightIndex(name, fixingDays, currency, calendar, dayCounter, curveHandle));
				marketStore_.addCurveHandle(name, curveHandle);
				marketStore_.addIndex(name, index);
			}
			else if (type == "IBOR") {
				boost::shared_ptr<IborIndex>index(new IborIndex(name, tenor, fixingDays, currency, calendar, convention, endOfMonth, dayCounter, curveHandle));
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

	
}