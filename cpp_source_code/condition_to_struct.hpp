#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "data_struct.hpp"

namespace py = pybind11;

GalaxyCondition galaxy_condition_to_struct(py::dict galaxy_condition);
