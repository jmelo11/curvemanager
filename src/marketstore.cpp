#include <curvemanager/marketstore.hpp>
#include <qlp/schemas/requests/discountfactorsrequest.hpp>
#include <qlp/schemas/requests/forwardratesrequest.hpp>
#include <qlp/schemas/requests/zeroratesrequest.hpp>

namespace CurveManager {
	
	MarketStore::MarketStore() {};

	boost::shared_ptr<YieldTermStructure> MarketStore::getCurve(const std::string& name) const {
		if (hasCurve(name))
			return curveMap_.at(name);
		throw std::runtime_error("Curve not found: " + name);
	};

	boost::shared_ptr<IborIndex> MarketStore::getIndex(const std::string& name) const {
		if (hasIndex(name))
			return indexMap_.at(name);
		throw std::runtime_error("Index not found: " + name);
	}
	
	Handle<Quote>& MarketStore::getQuote(const std::string& ticker) {
		if (hasQuote(ticker))
			return quoteMap_.at(ticker);
		throw std::runtime_error("Index not found: " + ticker);
	}

	RelinkableHandle<YieldTermStructure>& MarketStore::getCurveHandle(const std::string& name) {
		if (hasCurveHandle(name))
			return curveHandleMap_.at(name);
		throw std::runtime_error("Curve handle not found: " + name);
	}

	bool MarketStore::hasCurve(const std::string& name) const {
		if (curveMap_.find(name) != curveMap_.end())
			return true;
		return false;
	}

	bool MarketStore::hasIndex(const std::string& name) const {
		if (indexMap_.find(name) != indexMap_.end())
			return true;
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
		quoteMap_.insert({ ticker, handle });
	}

	void MarketStore::addCurveHandle(const std::string& name, RelinkableHandle<YieldTermStructure>& handle) {
		curveHandleMap_.insert({ name, handle });
	}
	
	void MarketStore::freeze() {

		for (const auto& [name, curve] : curveMap_) {
			auto ptr = boost::dynamic_pointer_cast<PiecewiseYieldCurve<Discount, LogLinear>>(curve);
			if (ptr)
				ptr->freeze();
		}

	}

	void MarketStore::unfreeze() {
		for (const auto& [name, curve] : curveMap_) {
			auto ptr = boost::dynamic_pointer_cast<PiecewiseYieldCurve<Discount, LogLinear>>(curve);
			if (ptr)
				ptr->unfreeze();
		}
	}

	std::vector<std::string> MarketStore::allCurves() const {
		std::vector<std::string> names;
		for (const auto& [name, curve] : curveMap_)
			names.push_back(name);
		return names;
	}

	std::vector<std::string> MarketStore::allIndexes() const {
		std::vector<std::string> names;
		for (const auto& [name, index] : indexMap_)
			names.push_back(name);
		return names;
	}

	json MarketStore::results(const std::vector<std::string>& dates) const {
		std::vector<Date> qlDates;
		json results = json::array();

		for (const auto& date : dates)
			qlDates.push_back(parse<Date>(date));

		auto curves = allCurves();
		for (const auto& curve : curves) {
			json data;
			auto qlCurve = getCurve(curve);
			std::vector<double> values;
			for (const auto& date : qlDates) {
				values.push_back(qlCurve->discount(date));
			}
			data["NAME"] = curve;
			data["DATES"] = dates;
			data["VALUES"] = values;
			results.push_back(data);
		}
		return results;
	}

	json MarketStore::discountRequest(const json& request) const {
		Schema<DiscountFactorsRequest> schema;
		schema.validate(request);
		json data = schema.setDefaultValues(request);

		auto curve = getCurve(data.at("CURVE"));
		
		json response = R"({"DATES":[], "VALUES": []})"_json;
		for (const auto& date : data.at("DATES")) {
			auto qlDate = parse<Date>(date);
			response["VALUES"].push_back(curve->discount(qlDate));
		}
		response["DATES"] = request["DATES"];
		return response;
	
	}

	json MarketStore::zeroRateRequest(const json& request) const {
		Schema<ZeroRatesRequests> schema;
		schema.validate(request);
		json data = schema.setDefaultValues(request);

		auto curve = getCurve(data.at("CURVE"));
		DayCounter dayCounter = parse<DayCounter>(data.at("DAYCOUNTER"));
		Compounding comp = parse<Compounding>(data.at("COMPOUNDING"));
		Frequency freq = parse<Frequency>(data.at("FREQUENCY"));		
		json response;
		response["VALUES"] = json::array();
		for (const auto& date : data.at("DATES")) {
			auto qlDate = parse<Date>(date);			
			response["VALUES"].push_back(curve->zeroRate(qlDate, dayCounter, comp, freq).rate());
		}
		response["DATES"] = request["DATES"];
		return response;
	}

	json MarketStore::forwardRateRequest(const json& request) const {
		Schema<ForwardRatesRequest> schema;
		schema.validate(request);
		json data = schema.setDefaultValues(request);

		auto curve = getCurve(data.at("CURVE"));
		DayCounter dayCounter = parse<DayCounter>(data.at("DAYCOUNTER"));
		Compounding comp = parse<Compounding>(data.at("COMPOUNDING"));
		Frequency freq = parse<Frequency>(data.at("FREQUENCY"));
		json response;
		response["VALUES"] = json::array();
		for (const auto& dates : data.at("DATES")) {
			auto startDate = parse<Date>(dates[0]);
			auto endDate = parse<Date>(dates[1]);
			response["VALUES"].push_back(curve->forwardRate(startDate,endDate, dayCounter, comp, freq).rate());
		}
		response["DATES"] = request["DATES"];
		return response;
	}
}

