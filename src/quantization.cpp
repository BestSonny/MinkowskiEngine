/*  Copyright (c) Chris Choy (chrischoy@ai.stanford.edu).
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 *  Please cite "4D Spatio-Temporal ConvNets: Minkowski Convolutional Neural
 *  Networks", CVPR'19 (https://arxiv.org/abs/1904.08755) if you use any part
 *  of the code.
 */

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "robin_coordsmap.hpp"
#include "utils.hpp"

namespace py = pybind11;

struct IndexLabel {
  int index;
  int label;

  IndexLabel() : index(-1), label(-1) {}
  IndexLabel(int index_, int label_) : index(index_), label(label_) {}
};

using CoordsLabelMap =
    robin_hood::unordered_flat_map<vector<int>, IndexLabel, byte_hash_vec<int>>;

vector<int>
quantize(py::array_t<int, py::array::c_style | py::array::forcecast> coords) {
  py::buffer_info coords_info = coords.request();
  auto &shape = coords_info.shape;

  ASSERT(shape.size() == 2,
         "Dimension must be 2. The dimension of the input: ", shape.size());

  int *p_coords = (int *)coords_info.ptr;
  int nrows = shape[0], ncols = shape[1];

  // Create coords map
  CoordsMap map;
  auto map_batch = map.initialize(p_coords, nrows, ncols, false);
  vector<int> &mapping = map_batch.first;

  return move(mapping);
}

vector<py::array> quantize_label(
    py::array_t<int, py::array::c_style | py::array::forcecast> coords,
    py::array_t<int, py::array::c_style | py::array::forcecast> labels,
    int invalid_label) {
  py::buffer_info coords_info = coords.request();
  py::buffer_info labels_info = labels.request();
  auto &shape = coords_info.shape;
  auto &lshape = labels_info.shape;

  ASSERT(shape.size() == 2,
         "Dimension must be 2. The dimension of the input: ", shape.size());

  ASSERT(shape[0] == lshape[0], "Coords nrows must be equal to label size.");

  int *p_coords = (int *)coords_info.ptr;
  int *p_labels = (int *)labels_info.ptr;
  int nrows = shape[0], ncols = shape[1];

  // Create coords map
  CoordsLabelMap map;
  map.reserve(nrows);
  for (int i = 0; i < nrows; i++) {
    vector<int> coord(ncols);
    copy_n(p_coords + i * ncols, ncols, coord.data());
    auto map_iter = map.find(coord);
    if (map_iter == map.end()) {
      map[move(coord)] = IndexLabel(i, p_labels[i]);
    } else if (map_iter->second.label != p_labels[i]) {
      map_iter->second.label = invalid_label;
    }
  }

  // Copy the concurrent vector to std vector
  py::array_t<int> py_mapping = py::array_t<int>(map.size());
  py::array_t<int> py_colabels = py::array_t<int>(map.size());

  py::buffer_info py_mapping_info = py_mapping.request();
  py::buffer_info py_colabels_info = py_colabels.request();
  int *p_py_mapping = (int *)py_mapping_info.ptr;
  int *p_py_colabels = (int *)py_colabels_info.ptr;

  int c = 0;
  for (const auto &kv : map) {
    p_py_mapping[c] = kv.second.index;
    p_py_colabels[c] = kv.second.label;
    c++;
  }

  vector<py::array> return_pair = {py_mapping, py_colabels};
  return return_pair;
}
