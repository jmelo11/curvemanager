#pragma once

#ifndef JSONTOOBJECT_HPP
#define JSONTOOBJECT_HPP

//Contents of Header
#include <qlp/parser.hpp>
#include <qlp/schemas/ratehelpers/bondratehelperschema.hpp>
#include <qlp/schemas/ratehelpers/depositratehelperschema.hpp>
#include <qlp/schemas/ratehelpers/oisratehelperschema.hpp>
#include <qlp/schemas/ratehelpers/swapratehelperschema.hpp>
#include <qlp/schemas/ratehelpers/fixfloatxccyratehelperschema.hpp>
#include <qlp/schemas/ratehelpers/fxswapratehelperschema.hpp>
#include <qlp/schemas/ratehelpers/tenorbasisratehelperschema.hpp>
#include <qlp/schemas/ratehelpers/xccybasisratehelperschema.hpp>

#define SETVAR(dict, dataType, name) dataType name = parse<dataType>(dict.at(#name))

namespace CurveManager
{
	using namespace QuantLib;
	using namespace QuantExt;
	using namespace QuantLibParser;
	using json = nlohmann::json;
	
	static bool has(const json& j, const std::string& key)
	{
		return j.find(key) != j.end();
	}

	template <typename Helper, typename F0, typename... Fs>
	struct JsonToObject;

	template <typename Helper, typename F0, typename... Fs>
	struct JsonToObject
	{
		auto static initialize(const json& params, F0& priceGetter, Fs &...otherGetters);
	};

	// DepositRateHelper param tuple
	template <typename F0>
	struct JsonToObject<DepositRateHelper, F0>
	{
		auto static initialize(const json& params, F0& priceGetter)
		{
			SETVAR(params, DayCounter, DAYCOUNTER);
			SETVAR(params, Calendar, CALENDAR);

			double FIXINGDAYS = params.at("FIXINGDAYS");
			bool ENDOFMONTH = params.at("ENDOFMONTH");
			SETVAR(params, BusinessDayConvention, CONVENTION);

			// non-defaults
			SETVAR(params, Period, TENOR);

			Handle<Quote> RATE = priceGetter(params.at("RATE"), params.at("RATETICKER"));
			return boost::shared_ptr<DepositRateHelper>(new DepositRateHelper(RATE, TENOR, FIXINGDAYS, CALENDAR, CONVENTION, ENDOFMONTH, DAYCOUNTER));
		}
	};

	// FXSWAP param tuple
	template <typename F0, typename F1>
	struct JsonToObject<FxSwapRateHelper, F0, F1>
	{
		auto static initialize(const json& params, F0& priceGetter, F1& curveGetter)
		{

			SETVAR(params, Calendar, CALENDAR);
			double FIXINGDAYS = params.at("FIXINGDAYS");
			bool ENDOFMONTH = params.at("ENDOFMONTH");
			bool BASECURRENCYCOLLATERAL = params.at("BASECURRENCYCOLLATERAL");

			SETVAR(params, BusinessDayConvention, CONVENTION);

			RelinkableHandle<YieldTermStructure> COLLATERALCURVE = has(params, "COLLATERALCURVE") ? curveGetter(params.at("COLLATERALCURVE")) : RelinkableHandle<YieldTermStructure>();

			// non-defaults
			Period TENOR;
			if (has(params, "ENDDATE"))
			{
				SETVAR(params, Date, ENDDATE);
				int days = ENDDATE - Settings::instance().evaluationDate();
				if (days > 0)
				{
					TENOR = Period(days, TimeUnit::Days);
				}
				else
				{
					throw std::runtime_error("End date must be after today.");
				}
			}
			else
			{
				TENOR = parse<Period>(params.at("TENOR"));
			}

			auto FXPOINTS = priceGetter(params.at("FXPOINTS"), params.at("FXPOINTSTICKER"));
			auto FXSPOT = priceGetter(params.at("FXSPOT"), params.at("FXSPOTTICKER"));

			return boost::shared_ptr<FxSwapRateHelper>(new FxSwapRateHelper(FXPOINTS, FXSPOT, TENOR, FIXINGDAYS, CALENDAR, CONVENTION, ENDOFMONTH, BASECURRENCYCOLLATERAL, COLLATERALCURVE));
		}
	};


	template <typename F0>
	struct JsonToObject<FixedRateBondHelper, F0>
	{
		auto static initialize(const json& params, F0& priceGetter)
		{

			SETVAR(params, Calendar, CALENDAR);
			SETVAR(params, BusinessDayConvention, CONVENTION);
			SETVAR(params, Frequency, FREQUENCY);
			SETVAR(params, Period, TENOR);
			SETVAR(params, DayCounter, COUPONDAYCOUNTER);
			SETVAR(params, DayCounter, IRRDAYCOUNTER);

			bool ENDOFMONTH = params.at("ENDOFMONTH");
			double SETTLEMENTDAYS = params.at("SETTLEMENTDAYS");
			double FACEAMOUNT = params.at("FACEAMOUNT");
			double COUPON = params.at("COUPON");

			Date STARTDATE;
			if (has(params, "STARTDATE")) {
				STARTDATE = parse<Date>(params.at("STARTDATE"));
			}
			else {
				STARTDATE = Settings::instance().evaluationDate();
			}
			Date ENDDATE;
			if (has(params, "ENDDATE")) {
				ENDDATE = parse<Date>(params.at("ENDDATE"));
			}
			else {
				ENDDATE = STARTDATE + TENOR;
			}

			/* coupon rate */

			InterestRate couponRate(COUPON, COUPONDAYCOUNTER, Compounding::Simple, Frequency::Annual);
			std::vector<InterestRate> coupons{ couponRate };
			auto RATE = priceGetter(params.at("RATE"), params.at("RATETICKER"));

			Schedule schedule = MakeSchedule().from(STARTDATE).to(ENDDATE).withTenor(TENOR).withFrequency(FREQUENCY).withCalendar(CALENDAR).withConvention(CONVENTION);

			FixedRateBond bond(SETTLEMENTDAYS, FACEAMOUNT, schedule, coupons);
			boost::shared_ptr<SimpleQuote> cleanPrice(boost::make_shared<SimpleQuote>(bond.cleanPrice(RATE->value(), IRRDAYCOUNTER, Compounding::Compounded, Frequency::Annual)));
			Handle<Quote> handlePrice(cleanPrice);
			return boost::shared_ptr<FixedRateBondHelper>(new FixedRateBondHelper(handlePrice, SETTLEMENTDAYS, FACEAMOUNT, schedule, std::vector<double>{COUPON}, COUPONDAYCOUNTER));
		}
	};

	// SwapRateHelper param tuple
	template <typename F0, typename F1, typename F2>
	struct JsonToObject<SwapRateHelper, F0, F1, F2>
	{
		auto static initialize(const json& params, F0& priceGetter, F1& indexGetter, F2& curveGetter)
		{

			SETVAR(params, DayCounter, DAYCOUNTER);
			SETVAR(params, Calendar, CALENDAR);
			SETVAR(params, BusinessDayConvention, CONVENTION);

			SETVAR(params, Frequency, FREQUENCY);

			bool ENDOFMONTH = params.at("ENDOFMONTH");
			double SPREAD = params.at("SPREAD");
			int SETTLEMENTDAYS = params.at("SETTLEMENTDAYS");
	

			SETVAR(params, Period, FWDSTART);
			SETVAR(params, Period, TENOR);

			boost::shared_ptr<Quote> spreadPtr(new SimpleQuote(SPREAD));
			Handle<Quote> SPREADQUOTE(spreadPtr);

			RelinkableHandle<YieldTermStructure> DISCOUNTINGCURVE = has(params, "DISCOUNTINGCURVE") ? curveGetter(params.at("DISCOUNTINGCURVE")) : RelinkableHandle<YieldTermStructure>();

			auto RATE = priceGetter(params.at("RATE"), params.at("RATETICKER"));

			boost::shared_ptr<IborIndex> INDEX = indexGetter(params.at("INDEX"));
			return boost::shared_ptr<SwapRateHelper>(new SwapRateHelper(RATE, TENOR, CALENDAR, FREQUENCY, CONVENTION, DAYCOUNTER, INDEX, SPREADQUOTE, FWDSTART, DISCOUNTINGCURVE, SETTLEMENTDAYS));
		}
	};

	// OISRateHelper param tuple
	template <typename F0, typename F1, typename F2>
	struct JsonToObject<OISRateHelper, F0, F1, F2>
	{
		auto static initialize(const json& params, F0& priceGetter, F1& indexGetter, F2& curveGetter)
		{

			SETVAR(params, DayCounter, DAYCOUNTER);
			SETVAR(params, Calendar, CALENDAR);
			SETVAR(params, BusinessDayConvention, CONVENTION);

			SETVAR(params, Frequency, FREQUENCY);
			
			bool ENDOFMONTH = params.at("ENDOFMONTH");
			double SPREAD = params.at("SPREAD");
			int SETTLEMENTDAYS = params.at("SETTLEMENTDAYS");
			int PAYMENTLAG = params.at("PAYMENTLAG");
			bool TELESCOPICVALUEDATES = params.at("TELESCOPICVALUEDATES");

			SETVAR(params, Period, FWDSTART);;

			SETVAR(params, Period, TENOR);

			RelinkableHandle<YieldTermStructure> DISCOUNTINGCURVE = has(params, "DISCOUNTINGCURVE") ? curveGetter(params.at("DISCOUNTINGCURVE")) : RelinkableHandle<YieldTermStructure>();

			auto RATE = priceGetter(params.at("RATE"), params.at("RATETICKER"));
			boost::shared_ptr<OvernightIndex> INDEX = boost::dynamic_pointer_cast<OvernightIndex>(indexGetter(params.at("INDEX")));
			return boost::shared_ptr<OISRateHelper>(
				new OISRateHelper(SETTLEMENTDAYS, TENOR, RATE, INDEX, DISCOUNTINGCURVE, TELESCOPICVALUEDATES, PAYMENTLAG, CONVENTION, FREQUENCY, CALENDAR, FWDSTART, SPREAD));
		}
	};

	// XCCY param tuple
	template <typename F0, typename F1, typename F2>
	struct JsonToObject<CrossCcyFixFloatSwapHelper, F0, F1, F2>
	{
		auto static initialize(const json& params, F0& priceGetter, F1& indexGetter, F2& curveGetter)
		{

			SETVAR(params, DayCounter, DAYCOUNTER);
			SETVAR(params, Calendar, CALENDAR);
			SETVAR(params, BusinessDayConvention, CONVENTION);

			SETVAR(params, Frequency, FREQUENCY);
			
			bool ENDOFMONTH = params.at("ENDOFMONTH");
			double SPREAD = params.at("SPREAD");
			int SETTLEMENTDAYS = params.at("SETTLEMENTDAYS");
			

			SETVAR(params, Period, TENOR);
			SETVAR(params, Currency, CURRENCY);

			boost::shared_ptr<Quote> spreadPtr(new SimpleQuote(SPREAD));
			Handle<Quote> SPREADQUOTE(spreadPtr);
			RelinkableHandle<YieldTermStructure> DISCOUNTINGCURVE = has(params, "DISCOUNTINGCURVE") ? curveGetter(params.at("DISCOUNTINGCURVE")) : RelinkableHandle<YieldTermStructure>();
			auto RATE = priceGetter(params.at("RATE"), params.at("RATETICKER"));
			auto FXSPOT = priceGetter(params.at("FXSPOT"), params.at("FXSPOTTICKER"));
			boost::shared_ptr<IborIndex> INDEX = indexGetter(params.at("INDEX"));
			return boost::shared_ptr<CrossCcyFixFloatSwapHelper>(new CrossCcyFixFloatSwapHelper(RATE, FXSPOT, SETTLEMENTDAYS, CALENDAR, CONVENTION, TENOR, CURRENCY, FREQUENCY, CONVENTION, DAYCOUNTER, INDEX, DISCOUNTINGCURVE, SPREADQUOTE, ENDOFMONTH));
		}
	};

	template <typename F0, typename F1, typename F2>
	struct JsonToObject<TenorBasisSwapHelper, F0, F1, F2>
	{
		auto static initialize(const json& params, F0& priceGetter, F1& indexGetter, F2& curveGetter)
		{

			SETVAR(params, Period, TENOR);

			bool SPREADONSHORT = params.at("SPREADONSHORT");

			RelinkableHandle<YieldTermStructure> DISCOUNTINGCURVE = has(params, "DISCOUNTINGCURVE") ? curveGetter(params.at("DISCOUNTINGCURVE")) : RelinkableHandle<YieldTermStructure>();

			auto SPREAD = priceGetter(params.at("SPREAD"), params.at("SPREADTICKER"));
			boost::shared_ptr<IborIndex> SHORTINDEX = indexGetter(params.at("SHORTINDEX"));
			boost::shared_ptr<IborIndex> LONGINDEX = indexGetter(params.at("LONGINDEX"));

			/*check if any of the curves can be build, otherwise fail*/
			if (SHORTINDEX->forwardingTermStructure().empty() && LONGINDEX->forwardingTermStructure().empty())
			{
				RelinkableHandle<YieldTermStructure> handle = curveGetter(params.at("SHORTINDEX"));
				if (handle.empty())
				{
					handle = curveGetter(params.at("LONGINDEX"));
					LONGINDEX = indexGetter(params.at("LONGINDEX"));
				}
				else
				{
					SHORTINDEX = indexGetter(params.at("SHORTINDEX"));
				}
			}

			Period SHORTPAYTENOR = has(params, "SHORTPAYTENOR") ? parse<Period>(params.at("SHORTPAYTENOR")) : Period();
			return boost::shared_ptr<TenorBasisSwapHelper>(
				new TenorBasisSwapHelper(SPREAD, TENOR, LONGINDEX, SHORTINDEX, SHORTPAYTENOR, DISCOUNTINGCURVE, SPREADONSHORT));
		}
	};

	// XCCYBasis param tuple
	template <typename F0, typename F1, typename F2>
	struct JsonToObject<CrossCcyBasisSwapHelper, F0, F1, F2>
	{
		auto static initialize(const json& params, F0& priceGetter, F1& indexGetter, F2& curveGetter)
		{

			SETVAR(params, Calendar, CALENDAR);
			SETVAR(params, BusinessDayConvention, CONVENTION);
			SETVAR(params, Period, TENOR);

			bool ENDOFMONTH = params.at("ENDOFMONTH");
			bool FLATISDOMESTIC = params.at("FLATISDOMESTIC");
			int SETTLEMENTDAYS = params.at("SETTLEMENTDAYS");

			auto SPREAD = priceGetter(params.at("SPREAD"), params.at("SPREADTICKER"));

			RelinkableHandle<YieldTermStructure> FLATDISCOUNTINGCURVE = has(params, "FLATDISCOUNTINGCURVE") ? curveGetter(params.at("FLATDISCOUNTINGCURVE")) : RelinkableHandle<YieldTermStructure>();
			RelinkableHandle<YieldTermStructure> SPREADDISCOUNTINGCURVE = has(params, "SPREADDISCOUNTINGCURVE") ? curveGetter(params.at("SPREADDISCOUNTINGCURVE")) : RelinkableHandle<YieldTermStructure>();
			if (FLATDISCOUNTINGCURVE.empty() && SPREADDISCOUNTINGCURVE.empty())
				throw std::runtime_error("Either FLATDISCOUNTINGCURVE or SPREADDISCOUNTINGCURVE must be specified");


			auto FXSPOT = priceGetter(params.at("FXSPOT"), params.at("FXSPOTTICKER"));

			boost::shared_ptr<IborIndex> FLATINDEX = indexGetter(params.at("FLATINDEX"));
			boost::shared_ptr<IborIndex> SPREADINDEX = indexGetter(params.at("SPREADINDEX"));

			return boost::shared_ptr<CrossCcyBasisSwapHelper>(
				new CrossCcyBasisSwapHelper(SPREAD, FXSPOT, SETTLEMENTDAYS, CALENDAR, TENOR, CONVENTION, FLATINDEX, SPREADINDEX, FLATDISCOUNTINGCURVE, SPREADDISCOUNTINGCURVE, ENDOFMONTH, FLATISDOMESTIC));
		}
	};

	template<typename T, typename ...Fs>
	auto JsonToObjectWrapper(const json& params, Fs&... getters) {	
		json givenParams = params;
		Schema<T> schema;
		schema.validate(givenParams);
		schema.setDefaultValues(givenParams);
		return JsonToObject<T, Fs...>::initialize(givenParams, getters...);
	}
}

#endif // !JSONTOOBJECT_H