cmake_minimum_required(VERSION 3.4...3.18)
project(curvemanagerpython LANGUAGES CXX)

find_package(pybind11 REQUIRED)
find_package(curvemanager REQUIRED)
pybind11_add_module(curvemanagerpython module.cpp)