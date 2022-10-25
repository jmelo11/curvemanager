#pragma once

#include <curvemanager/curvemanager.hpp>
#include <bondpricer/bondpricer.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11_json/pybind11_json.hpp>

namespace py = pybind11;

using namespace curvemanager;
using namespace std;


PYBIND11_MODULE(curvemanager, m) {
    m.doc() = "CurveManager"; // optional module docstring

    py::class_<MarketStore>(m, "MarketStore")   
        .def("allCurves", &MarketStore::allCurves)     
        .def("results", &MarketStore::results);

    py::class_<CurveBuilder>(m, "CurveBuilder")
        .def(py::init<json, MarketStore>(), py::arg("data"), py::arg("marketStore"))
        .def("build", &CurveBuilder::build)
        .def("updateQuotes", &CurveBuilder::updateQuotes, py::arg("prices"));
        
}
