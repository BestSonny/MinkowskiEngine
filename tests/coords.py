# Copyright (c) Chris Choy (chrischoy@ai.stanford.edu).
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is furnished to do
# so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Please cite "4D Spatio-Temporal ConvNets: Minkowski Convolutional Neural
# Networks", CVPR'19 (https://arxiv.org/abs/1904.08755) if you use any part
# of the code.
import unittest

import torch

from MinkowskiEngine import CoordsKey, CoordsManager


class Test(unittest.TestCase):

    def test_hash(self):
        try:
            import numpy as np
        except:
            return

        N, M = 1000, 1000
        I, J = np.meshgrid(np.arange(N), np.arange(M))
        I = I.reshape(-1, 1) - 100
        J = J.reshape(-1, 1) - 100
        K = np.zeros_like(I)
        C = np.hstack((I, J, K))
        coords_manager = CoordsManager(D=2)
        coords_key = CoordsKey(2)
        coords_key.setTensorStride(1)
        coords_manager.initialize(torch.from_numpy(C).int(), coords_key)
        print(coords_manager)

    def test_coords_key(self):
        key = CoordsKey(D=1)
        key.setKey(1)
        self.assertTrue(key.getKey() == 1)
        key.setTensorStride([1])
        print(key)

    def test_coords_manager(self):
        key = CoordsKey(D=1)
        key.setTensorStride(1)

        cm = CoordsManager(D=1)
        coords = (torch.rand(20, 2) * 10).int()
        print(coords)
        unique_coords = torch.unique(coords, dim=0)
        print('Num unique: ', unique_coords.shape)

        # Initialize map
        mapping = cm.initialize(
            coords, key, force_remap=True, allow_duplicates=False)
        print(mapping, len(mapping))
        cm.print_diagnostics(key)
        print(cm)

        # Create a strided map
        stride_key = cm.stride(key, [4])
        print('Stride: ', cm.get_coords(stride_key))
        cm.print_diagnostics(key)
        print(cm)

        # Create a transposed stride map
        transposed_key = cm.transposed_stride(
            stride_key, [2], [3], [1])
        print('Transposed Stride: ', cm.get_coords(transposed_key))
        print(cm)

        # Create a transposed stride map
        transposed_key = cm.transposed_stride(
            stride_key, [2], [3], [1], force_creation=True)
        print('Forced Transposed Stride: ', cm.get_coords(transposed_key))
        print(cm)

        # Create a reduction map
        key = cm.reduce()
        print('Reduction: ', cm.get_coords(key))
        print(cm)

        print('Reduction mapping: ', cm.get_row_indices_per_batch(stride_key))
        print(cm)


if __name__ == '__main__':
    unittest.main()
