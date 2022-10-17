#pragma once
#include <curvemanager.hpp>
#include <parser.hpp>

namespace CurveManager {
	
	CurveBuilder::CurveBuilder(const json& data) : data_(data) {
		if (!data_.empty())
			preprocessData(data_);
		buildAllCurves();
	};

	void CurveBuilder::preprocessData(const json& data) {
		data_ = data;
		curveConfigs_.clear();
		indexConfigs_.clear();
		for (const auto& curve : data_.at("CURVES")) {
			const std::string& name = curve.at("NAME");
			curveConfigs_[name] = curve;
			RelinkableHandle<YieldTermStructure> handle;
			marketStore_.addCurveHandle(name, handle);
		}
		for (const auto& index : data_.at("INDEX")) {
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
		const std::string& curveType = curveParams.at("TYPE");
		if (curveType == "DISCOUNTCURVE") {
			buildDiscountCurve(curveName, curveParams);
		}
		else if (curveType == "PIECEWISECURVE") {
			buildPiecewiseCurve(curveName, curveParams);
		}				
	}

	void CurveBuilder::buildPiecewiseCurve(const std::string& curveName, const json& curveParams) {
		if (!marketStore_.hasCurve(curveName)) {
			auto helpers = buildRateHelpers(curveParams.at("RATEHELPERS"), curveName);

			DayCounter dayCounter = parse<DayCounter>(curveParams.at("DAYCOUNTER"));
			bool enableExtrapolation = curveParams.at("ENABLEEXTRAPOLATION");

			Date qlRefDate = Settings::instance().evaluationDate();
			boost::shared_ptr<YieldTermStructure> curvePtr(new LogLinearPiecewiseCurve(qlRefDate, helpers, dayCounter));
			if (enableExtrapolation)
				curvePtr->enableExtrapolation();

			RelinkableHandle<YieldTermStructure>& handle = marketStore_.getCurveHandle(curveName);
			handle.linkTo(curvePtr);

			curvePtr->unregisterWith(Settings::instance().evaluationDate());
			marketStore_.addCurve(curveName, curvePtr);
		}
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