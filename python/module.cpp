#pragma once

#include <curvemanager/curvemanager.hpp>
#include <curvemanager/standalonemethods.h>
#include <bondpricer/bondpricer.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11_json/pybind11_json.hpp>

namespace py = pybind11;

using namespace curvemanager;
using namespace std;


PYBIND11_MODULE(curvemanager, m) {
    m.doc() = "CurveManager"; // optional module docstring

    py::class_<CurveBuilder<Discount, LogLinear>>(m, "CurveBuilder")
        .def(py::init<json>(), py::arg("curveData"))
        .def("set", &CurveBuilder<Discount, LogLinear>::set, py::arg("curveData"))
        .def("build", static_cast<void(CurveBuilder<Discount, LogLinear>::*)(void)>(&CurveBuilder<Discount, LogLinear>::build))        
        .def("discounts",&CurveBuilder<Discount, LogLinear>::discounts, py::arg("curve"), py::arg("dates"))
        .def("forwardRates",&CurveBuilder<Discount, LogLinear>::forwardRates, py::arg("curve"), py::arg("dates"), py::arg("day_counter"), py::arg("compounding"), py::arg("frequency"))
        .def("zeroRates",&CurveBuilder<Discount, LogLinear>::zeroRates, py::arg("curve"), py::arg("dates"), py::arg("day_counter"), py::arg("compounding"), py::arg("frequency"))
        .def("nodes",&CurveBuilder<Discount, LogLinear>::nodes)
        .def("updateQuotes",&CurveBuilder<Discount, LogLinear>::updateQuotes, py::arg("quotes"))
        .def("freeze",&CurveBuilder<Discount, LogLinear>::freeze)
        .def("unfreeze",&CurveBuilder<Discount, LogLinear>::unfreeze)
        .def("flush",&CurveBuilder<Discount, LogLinear>::flush)
        .def("allCurves",&CurveBuilder<Discount, LogLinear>::allCurves);

    py::class_<BondPricer>(m, "BondPricer")
        .def(py::init<json>(), py::arg("bondData"))
        .def("bondCashflows", &BondPricer::bondCashflows)
        .def("zSpread", &BondPricer::zSpread, py::arg("params"))
        .def("ASW", &BondPricer::ASW, py::arg("params"));

    m.def("discounts", &discounts, py::arg("params"));
    m.def("forwardRates", &forwardRates, py::arg("params"));
    m.def("zeroRates", &zeroRates, py::arg("params"));
}
