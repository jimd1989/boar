/* Array representing one cycle of a sine wave. A tone of an arbitrary pitch
 * is derived from interpolated lookups to this table. */

#pragma once

#include "../constants/defaults.h"

static const float WAVE_SINE_000[DEFAULT_WAVELEN] = {
    0, 0.00306796, 0.00613588, 0.00920375, 0.0122715, 0.0153392,
    0.0184067, 0.0214741, 0.0245412, 0.0276081, 0.0306748, 0.0337412,
    0.0368072, 0.0398729, 0.0429383, 0.0460032, 0.0490677, 0.0521317,
    0.0551952, 0.0582583, 0.0613207, 0.0643826, 0.0674439, 0.0705046,
    0.0735646, 0.0766239, 0.0796824, 0.0827403, 0.0857973, 0.0888536,
    0.091909, 0.0949635, 0.0980171, 0.10107, 0.104122, 0.107172,
    0.110222, 0.113271, 0.116319, 0.119365, 0.122411, 0.125455,
    0.128498, 0.13154, 0.134581, 0.13762, 0.140658, 0.143695,
    0.14673, 0.149765, 0.152797, 0.155828, 0.158858, 0.161886,
    0.164913, 0.167938, 0.170962, 0.173984, 0.177004, 0.180023,
    0.18304, 0.186055, 0.189069, 0.19208, 0.19509, 0.198098,
    0.201105, 0.204109, 0.207111, 0.210112, 0.21311, 0.216107,
    0.219101, 0.222094, 0.225084, 0.228072, 0.231058, 0.234042,
    0.237024, 0.240003, 0.24298, 0.245955, 0.248928, 0.251898,
    0.254866, 0.257831, 0.260794, 0.263755, 0.266713, 0.269668,
    0.272621, 0.275572, 0.27852, 0.281465, 0.284408, 0.287347,
    0.290285, 0.293219, 0.296151, 0.29908, 0.302006, 0.304929,
    0.30785, 0.310767, 0.313682, 0.316593, 0.319502, 0.322408,
    0.32531, 0.32821, 0.331106, 0.334, 0.33689, 0.339777,
    0.342661, 0.345541, 0.348419, 0.351293, 0.354164, 0.357031,
    0.359895, 0.362756, 0.365613, 0.368467, 0.371317, 0.374164,
    0.377007, 0.379847, 0.382683, 0.385516, 0.388345, 0.39117,
    0.393992, 0.39681, 0.399624, 0.402435, 0.405241, 0.408044,
    0.410843, 0.413638, 0.41643, 0.419217, 0.422, 0.42478,
    0.427555, 0.430326, 0.433094, 0.435857, 0.438616, 0.441371,
    0.444122, 0.446869, 0.449611, 0.45235, 0.455084, 0.457813,
    0.460539, 0.46326, 0.465976, 0.468689, 0.471397, 0.4741,
    0.476799, 0.479494, 0.482184, 0.484869, 0.48755, 0.490226,
    0.492898, 0.495565, 0.498228, 0.500885, 0.503538, 0.506187,
    0.50883, 0.511469, 0.514103, 0.516732, 0.519356, 0.521975,
    0.52459, 0.527199, 0.529804, 0.532403, 0.534998, 0.537587,
    0.540171, 0.542751, 0.545325, 0.547894, 0.550458, 0.553017,
    0.55557, 0.558119, 0.560662, 0.563199, 0.565732, 0.568259,
    0.570781, 0.573297, 0.575808, 0.578314, 0.580814, 0.583309,
    0.585798, 0.588282, 0.59076, 0.593232, 0.595699, 0.598161,
    0.600616, 0.603067, 0.605511, 0.60795, 0.610383, 0.61281,
    0.615232, 0.617647, 0.620057, 0.622461, 0.624859, 0.627252,
    0.629638, 0.632019, 0.634393, 0.636762, 0.639124, 0.641481,
    0.643832, 0.646176, 0.648514, 0.650847, 0.653173, 0.655493,
    0.657807, 0.660114, 0.662416, 0.664711, 0.667, 0.669283,
    0.671559, 0.673829, 0.676093, 0.67835, 0.680601, 0.682846,
    0.685084, 0.687315, 0.689541, 0.691759, 0.693971, 0.696177,
    0.698376, 0.700569, 0.702755, 0.704934, 0.707107, 0.709273,
    0.711432, 0.713585, 0.715731, 0.71787, 0.720003, 0.722128,
    0.724247, 0.726359, 0.728464, 0.730563, 0.732654, 0.734739,
    0.736817, 0.738887, 0.740951, 0.743008, 0.745058, 0.747101,
    0.749136, 0.751165, 0.753187, 0.755201, 0.757209, 0.759209,
    0.761202, 0.763188, 0.765167, 0.767139, 0.769103, 0.771061,
    0.77301, 0.774953, 0.776888, 0.778817, 0.780737, 0.782651,
    0.784557, 0.786455, 0.788346, 0.79023, 0.792107, 0.793975,
    0.795837, 0.797691, 0.799537, 0.801376, 0.803208, 0.805031,
    0.806848, 0.808656, 0.810457, 0.812251, 0.814036, 0.815814,
    0.817585, 0.819348, 0.821103, 0.82285, 0.824589, 0.826321,
    0.828045, 0.829761, 0.83147, 0.83317, 0.834863, 0.836548,
    0.838225, 0.839894, 0.841555, 0.843208, 0.844854, 0.846491,
    0.84812, 0.849742, 0.851355, 0.852961, 0.854558, 0.856147,
    0.857729, 0.859302, 0.860867, 0.862424, 0.863973, 0.865514,
    0.867046, 0.868571, 0.870087, 0.871595, 0.873095, 0.874587,
    0.87607, 0.877545, 0.879012, 0.880471, 0.881921, 0.883363,
    0.884797, 0.886223, 0.88764, 0.889048, 0.890449, 0.891841,
    0.893224, 0.894599, 0.895966, 0.897325, 0.898674, 0.900016,
    0.901349, 0.902673, 0.903989, 0.905297, 0.906596, 0.907886,
    0.909168, 0.910441, 0.911706, 0.912962, 0.91421, 0.915449,
    0.916679, 0.917901, 0.919114, 0.920318, 0.921514, 0.922701,
    0.92388, 0.925049, 0.92621, 0.927363, 0.928506, 0.929641,
    0.930767, 0.931884, 0.932993, 0.934093, 0.935184, 0.936266,
    0.937339, 0.938404, 0.939459, 0.940506, 0.941544, 0.942573,
    0.943593, 0.944605, 0.945607, 0.946601, 0.947586, 0.948561,
    0.949528, 0.950486, 0.951435, 0.952375, 0.953306, 0.954228,
    0.955141, 0.956045, 0.95694, 0.957826, 0.958703, 0.959572,
    0.960431, 0.96128, 0.962121, 0.962953, 0.963776, 0.96459,
    0.965394, 0.96619, 0.966976, 0.967754, 0.968522, 0.969281,
    0.970031, 0.970772, 0.971504, 0.972226, 0.97294, 0.973644,
    0.974339, 0.975025, 0.975702, 0.97637, 0.977028, 0.977677,
    0.978317, 0.978948, 0.97957, 0.980182, 0.980785, 0.981379,
    0.981964, 0.982539, 0.983105, 0.983662, 0.98421, 0.984749,
    0.985278, 0.985798, 0.986308, 0.986809, 0.987301, 0.987784,
    0.988258, 0.988722, 0.989177, 0.989622, 0.990058, 0.990485,
    0.990903, 0.991311, 0.99171, 0.992099, 0.99248, 0.99285,
    0.993212, 0.993564, 0.993907, 0.99424, 0.994565, 0.994879,
    0.995185, 0.995481, 0.995767, 0.996045, 0.996313, 0.996571,
    0.99682, 0.99706, 0.99729, 0.997511, 0.997723, 0.997925,
    0.998118, 0.998302, 0.998476, 0.99864, 0.998795, 0.998941,
    0.999078, 0.999205, 0.999322, 0.999431, 0.999529, 0.999619,
    0.999699, 0.999769, 0.999831, 0.999882, 0.999925, 0.999958,
    0.999981, 0.999995, 1, 0.999995, 0.999981, 0.999958,
    0.999925, 0.999882, 0.999831, 0.999769, 0.999699, 0.999619,
    0.999529, 0.999431, 0.999322, 0.999205, 0.999078, 0.998941,
    0.998795, 0.99864, 0.998476, 0.998302, 0.998118, 0.997925,
    0.997723, 0.997511, 0.99729, 0.99706, 0.99682, 0.996571,
    0.996313, 0.996045, 0.995767, 0.995481, 0.995185, 0.994879,
    0.994565, 0.99424, 0.993907, 0.993564, 0.993212, 0.99285,
    0.99248, 0.992099, 0.99171, 0.991311, 0.990903, 0.990485,
    0.990058, 0.989622, 0.989177, 0.988722, 0.988258, 0.987784,
    0.987301, 0.986809, 0.986308, 0.985798, 0.985278, 0.984749,
    0.98421, 0.983662, 0.983105, 0.982539, 0.981964, 0.981379,
    0.980785, 0.980182, 0.97957, 0.978948, 0.978317, 0.977677,
    0.977028, 0.97637, 0.975702, 0.975025, 0.974339, 0.973644,
    0.97294, 0.972226, 0.971504, 0.970772, 0.970031, 0.969281,
    0.968522, 0.967754, 0.966976, 0.96619, 0.965394, 0.96459,
    0.963776, 0.962953, 0.962121, 0.96128, 0.960431, 0.959572,
    0.958703, 0.957826, 0.95694, 0.956045, 0.955141, 0.954228,
    0.953306, 0.952375, 0.951435, 0.950486, 0.949528, 0.948561,
    0.947586, 0.946601, 0.945607, 0.944605, 0.943593, 0.942573,
    0.941544, 0.940506, 0.939459, 0.938404, 0.937339, 0.936266,
    0.935184, 0.934093, 0.932993, 0.931884, 0.930767, 0.929641,
    0.928506, 0.927363, 0.92621, 0.925049, 0.92388, 0.922701,
    0.921514, 0.920318, 0.919114, 0.917901, 0.916679, 0.915449,
    0.91421, 0.912962, 0.911706, 0.910441, 0.909168, 0.907886,
    0.906596, 0.905297, 0.903989, 0.902673, 0.901349, 0.900016,
    0.898674, 0.897325, 0.895966, 0.894599, 0.893224, 0.891841,
    0.890449, 0.889048, 0.88764, 0.886223, 0.884797, 0.883363,
    0.881921, 0.880471, 0.879012, 0.877545, 0.87607, 0.874587,
    0.873095, 0.871595, 0.870087, 0.868571, 0.867046, 0.865514,
    0.863973, 0.862424, 0.860867, 0.859302, 0.857729, 0.856147,
    0.854558, 0.852961, 0.851355, 0.849742, 0.84812, 0.846491,
    0.844854, 0.843208, 0.841555, 0.839894, 0.838225, 0.836548,
    0.834863, 0.83317, 0.83147, 0.829761, 0.828045, 0.826321,
    0.824589, 0.82285, 0.821103, 0.819348, 0.817585, 0.815814,
    0.814036, 0.812251, 0.810457, 0.808656, 0.806848, 0.805031,
    0.803208, 0.801376, 0.799537, 0.797691, 0.795837, 0.793975,
    0.792107, 0.79023, 0.788346, 0.786455, 0.784557, 0.782651,
    0.780737, 0.778817, 0.776888, 0.774953, 0.77301, 0.771061,
    0.769103, 0.767139, 0.765167, 0.763188, 0.761202, 0.759209,
    0.757209, 0.755201, 0.753187, 0.751165, 0.749136, 0.747101,
    0.745058, 0.743008, 0.740951, 0.738887, 0.736817, 0.734739,
    0.732654, 0.730563, 0.728464, 0.726359, 0.724247, 0.722128,
    0.720003, 0.71787, 0.715731, 0.713585, 0.711432, 0.709273,
    0.707107, 0.704934, 0.702755, 0.700569, 0.698376, 0.696177,
    0.693971, 0.691759, 0.689541, 0.687315, 0.685084, 0.682846,
    0.680601, 0.67835, 0.676093, 0.673829, 0.671559, 0.669283,
    0.667, 0.664711, 0.662416, 0.660114, 0.657807, 0.655493,
    0.653173, 0.650847, 0.648514, 0.646176, 0.643832, 0.641481,
    0.639124, 0.636762, 0.634393, 0.632019, 0.629638, 0.627252,
    0.624859, 0.622461, 0.620057, 0.617647, 0.615232, 0.61281,
    0.610383, 0.60795, 0.605511, 0.603067, 0.600616, 0.598161,
    0.595699, 0.593232, 0.59076, 0.588282, 0.585798, 0.583309,
    0.580814, 0.578314, 0.575808, 0.573297, 0.570781, 0.568259,
    0.565732, 0.563199, 0.560662, 0.558119, 0.55557, 0.553017,
    0.550458, 0.547894, 0.545325, 0.542751, 0.540171, 0.537587,
    0.534998, 0.532403, 0.529804, 0.527199, 0.52459, 0.521975,
    0.519356, 0.516732, 0.514103, 0.511469, 0.50883, 0.506187,
    0.503538, 0.500885, 0.498228, 0.495565, 0.492898, 0.490226,
    0.48755, 0.484869, 0.482184, 0.479494, 0.476799, 0.4741,
    0.471397, 0.468689, 0.465976, 0.46326, 0.460539, 0.457813,
    0.455084, 0.45235, 0.449611, 0.446869, 0.444122, 0.441371,
    0.438616, 0.435857, 0.433094, 0.430326, 0.427555, 0.42478,
    0.422, 0.419217, 0.41643, 0.413638, 0.410843, 0.408044,
    0.405241, 0.402435, 0.399624, 0.39681, 0.393992, 0.39117,
    0.388345, 0.385516, 0.382683, 0.379847, 0.377007, 0.374164,
    0.371317, 0.368467, 0.365613, 0.362756, 0.359895, 0.357031,
    0.354164, 0.351293, 0.348419, 0.345541, 0.342661, 0.339777,
    0.33689, 0.334, 0.331106, 0.32821, 0.32531, 0.322408,
    0.319502, 0.316593, 0.313682, 0.310767, 0.30785, 0.304929,
    0.302006, 0.29908, 0.296151, 0.293219, 0.290285, 0.287347,
    0.284408, 0.281465, 0.27852, 0.275572, 0.272621, 0.269668,
    0.266713, 0.263755, 0.260794, 0.257831, 0.254866, 0.251898,
    0.248928, 0.245955, 0.24298, 0.240003, 0.237024, 0.234042,
    0.231058, 0.228072, 0.225084, 0.222094, 0.219101, 0.216107,
    0.21311, 0.210112, 0.207111, 0.204109, 0.201105, 0.198098,
    0.19509, 0.19208, 0.189069, 0.186055, 0.18304, 0.180023,
    0.177004, 0.173984, 0.170962, 0.167938, 0.164913, 0.161886,
    0.158858, 0.155828, 0.152797, 0.149765, 0.14673, 0.143695,
    0.140658, 0.13762, 0.134581, 0.13154, 0.128498, 0.125455,
    0.122411, 0.119365, 0.116319, 0.113271, 0.110222, 0.107172,
    0.104122, 0.10107, 0.0980171, 0.0949635, 0.091909, 0.0888536,
    0.0857973, 0.0827403, 0.0796824, 0.0766239, 0.0735646, 0.0705046,
    0.0674439, 0.0643826, 0.0613207, 0.0582583, 0.0551952, 0.0521317,
    0.0490677, 0.0460032, 0.0429383, 0.0398729, 0.0368072, 0.0337412,
    0.0306748, 0.0276081, 0.0245412, 0.0214741, 0.0184067, 0.0153392,
    0.0122715, 0.00920375, 0.00613588, 0.00306796, 1.22465e-16, -0.00306796,
    -0.00613588, -0.00920375, -0.0122715, -0.0153392, -0.0184067, -0.0214741,
    -0.0245412, -0.0276081, -0.0306748, -0.0337412, -0.0368072, -0.0398729,
    -0.0429383, -0.0460032, -0.0490677, -0.0521317, -0.0551952, -0.0582583,
    -0.0613207, -0.0643826, -0.0674439, -0.0705046, -0.0735646, -0.0766239,
    -0.0796824, -0.0827403, -0.0857973, -0.0888536, -0.091909, -0.0949635,
    -0.0980171, -0.10107, -0.104122, -0.107172, -0.110222, -0.113271,
    -0.116319, -0.119365, -0.122411, -0.125455, -0.128498, -0.13154,
    -0.134581, -0.13762, -0.140658, -0.143695, -0.14673, -0.149765,
    -0.152797, -0.155828, -0.158858, -0.161886, -0.164913, -0.167938,
    -0.170962, -0.173984, -0.177004, -0.180023, -0.18304, -0.186055,
    -0.189069, -0.19208, -0.19509, -0.198098, -0.201105, -0.204109,
    -0.207111, -0.210112, -0.21311, -0.216107, -0.219101, -0.222094,
    -0.225084, -0.228072, -0.231058, -0.234042, -0.237024, -0.240003,
    -0.24298, -0.245955, -0.248928, -0.251898, -0.254866, -0.257831,
    -0.260794, -0.263755, -0.266713, -0.269668, -0.272621, -0.275572,
    -0.27852, -0.281465, -0.284408, -0.287347, -0.290285, -0.293219,
    -0.296151, -0.29908, -0.302006, -0.304929, -0.30785, -0.310767,
    -0.313682, -0.316593, -0.319502, -0.322408, -0.32531, -0.32821,
    -0.331106, -0.334, -0.33689, -0.339777, -0.342661, -0.345541,
    -0.348419, -0.351293, -0.354164, -0.357031, -0.359895, -0.362756,
    -0.365613, -0.368467, -0.371317, -0.374164, -0.377007, -0.379847,
    -0.382683, -0.385516, -0.388345, -0.39117, -0.393992, -0.39681,
    -0.399624, -0.402435, -0.405241, -0.408044, -0.410843, -0.413638,
    -0.41643, -0.419217, -0.422, -0.42478, -0.427555, -0.430326,
    -0.433094, -0.435857, -0.438616, -0.441371, -0.444122, -0.446869,
    -0.449611, -0.45235, -0.455084, -0.457813, -0.460539, -0.46326,
    -0.465976, -0.468689, -0.471397, -0.4741, -0.476799, -0.479494,
    -0.482184, -0.484869, -0.48755, -0.490226, -0.492898, -0.495565,
    -0.498228, -0.500885, -0.503538, -0.506187, -0.50883, -0.511469,
    -0.514103, -0.516732, -0.519356, -0.521975, -0.52459, -0.527199,
    -0.529804, -0.532403, -0.534998, -0.537587, -0.540171, -0.542751,
    -0.545325, -0.547894, -0.550458, -0.553017, -0.55557, -0.558119,
    -0.560662, -0.563199, -0.565732, -0.568259, -0.570781, -0.573297,
    -0.575808, -0.578314, -0.580814, -0.583309, -0.585798, -0.588282,
    -0.59076, -0.593232, -0.595699, -0.598161, -0.600616, -0.603067,
    -0.605511, -0.60795, -0.610383, -0.61281, -0.615232, -0.617647,
    -0.620057, -0.622461, -0.624859, -0.627252, -0.629638, -0.632019,
    -0.634393, -0.636762, -0.639124, -0.641481, -0.643832, -0.646176,
    -0.648514, -0.650847, -0.653173, -0.655493, -0.657807, -0.660114,
    -0.662416, -0.664711, -0.667, -0.669283, -0.671559, -0.673829,
    -0.676093, -0.67835, -0.680601, -0.682846, -0.685084, -0.687315,
    -0.689541, -0.691759, -0.693971, -0.696177, -0.698376, -0.700569,
    -0.702755, -0.704934, -0.707107, -0.709273, -0.711432, -0.713585,
    -0.715731, -0.71787, -0.720003, -0.722128, -0.724247, -0.726359,
    -0.728464, -0.730563, -0.732654, -0.734739, -0.736817, -0.738887,
    -0.740951, -0.743008, -0.745058, -0.747101, -0.749136, -0.751165,
    -0.753187, -0.755201, -0.757209, -0.759209, -0.761202, -0.763188,
    -0.765167, -0.767139, -0.769103, -0.771061, -0.77301, -0.774953,
    -0.776888, -0.778817, -0.780737, -0.782651, -0.784557, -0.786455,
    -0.788346, -0.79023, -0.792107, -0.793975, -0.795837, -0.797691,
    -0.799537, -0.801376, -0.803208, -0.805031, -0.806848, -0.808656,
    -0.810457, -0.812251, -0.814036, -0.815814, -0.817585, -0.819348,
    -0.821103, -0.82285, -0.824589, -0.826321, -0.828045, -0.829761,
    -0.83147, -0.83317, -0.834863, -0.836548, -0.838225, -0.839894,
    -0.841555, -0.843208, -0.844854, -0.846491, -0.84812, -0.849742,
    -0.851355, -0.852961, -0.854558, -0.856147, -0.857729, -0.859302,
    -0.860867, -0.862424, -0.863973, -0.865514, -0.867046, -0.868571,
    -0.870087, -0.871595, -0.873095, -0.874587, -0.87607, -0.877545,
    -0.879012, -0.880471, -0.881921, -0.883363, -0.884797, -0.886223,
    -0.88764, -0.889048, -0.890449, -0.891841, -0.893224, -0.894599,
    -0.895966, -0.897325, -0.898674, -0.900016, -0.901349, -0.902673,
    -0.903989, -0.905297, -0.906596, -0.907886, -0.909168, -0.910441,
    -0.911706, -0.912962, -0.91421, -0.915449, -0.916679, -0.917901,
    -0.919114, -0.920318, -0.921514, -0.922701, -0.92388, -0.925049,
    -0.92621, -0.927363, -0.928506, -0.929641, -0.930767, -0.931884,
    -0.932993, -0.934093, -0.935184, -0.936266, -0.937339, -0.938404,
    -0.939459, -0.940506, -0.941544, -0.942573, -0.943593, -0.944605,
    -0.945607, -0.946601, -0.947586, -0.948561, -0.949528, -0.950486,
    -0.951435, -0.952375, -0.953306, -0.954228, -0.955141, -0.956045,
    -0.95694, -0.957826, -0.958703, -0.959572, -0.960431, -0.96128,
    -0.962121, -0.962953, -0.963776, -0.96459, -0.965394, -0.96619,
    -0.966976, -0.967754, -0.968522, -0.969281, -0.970031, -0.970772,
    -0.971504, -0.972226, -0.97294, -0.973644, -0.974339, -0.975025,
    -0.975702, -0.97637, -0.977028, -0.977677, -0.978317, -0.978948,
    -0.97957, -0.980182, -0.980785, -0.981379, -0.981964, -0.982539,
    -0.983105, -0.983662, -0.98421, -0.984749, -0.985278, -0.985798,
    -0.986308, -0.986809, -0.987301, -0.987784, -0.988258, -0.988722,
    -0.989177, -0.989622, -0.990058, -0.990485, -0.990903, -0.991311,
    -0.99171, -0.992099, -0.99248, -0.99285, -0.993212, -0.993564,
    -0.993907, -0.99424, -0.994565, -0.994879, -0.995185, -0.995481,
    -0.995767, -0.996045, -0.996313, -0.996571, -0.99682, -0.99706,
    -0.99729, -0.997511, -0.997723, -0.997925, -0.998118, -0.998302,
    -0.998476, -0.99864, -0.998795, -0.998941, -0.999078, -0.999205,
    -0.999322, -0.999431, -0.999529, -0.999619, -0.999699, -0.999769,
    -0.999831, -0.999882, -0.999925, -0.999958, -0.999981, -0.999995,
    -1, -0.999995, -0.999981, -0.999958, -0.999925, -0.999882,
    -0.999831, -0.999769, -0.999699, -0.999619, -0.999529, -0.999431,
    -0.999322, -0.999205, -0.999078, -0.998941, -0.998795, -0.99864,
    -0.998476, -0.998302, -0.998118, -0.997925, -0.997723, -0.997511,
    -0.99729, -0.99706, -0.99682, -0.996571, -0.996313, -0.996045,
    -0.995767, -0.995481, -0.995185, -0.994879, -0.994565, -0.99424,
    -0.993907, -0.993564, -0.993212, -0.99285, -0.99248, -0.992099,
    -0.99171, -0.991311, -0.990903, -0.990485, -0.990058, -0.989622,
    -0.989177, -0.988722, -0.988258, -0.987784, -0.987301, -0.986809,
    -0.986308, -0.985798, -0.985278, -0.984749, -0.98421, -0.983662,
    -0.983105, -0.982539, -0.981964, -0.981379, -0.980785, -0.980182,
    -0.97957, -0.978948, -0.978317, -0.977677, -0.977028, -0.97637,
    -0.975702, -0.975025, -0.974339, -0.973644, -0.97294, -0.972226,
    -0.971504, -0.970772, -0.970031, -0.969281, -0.968522, -0.967754,
    -0.966976, -0.96619, -0.965394, -0.96459, -0.963776, -0.962953,
    -0.962121, -0.96128, -0.960431, -0.959572, -0.958703, -0.957826,
    -0.95694, -0.956045, -0.955141, -0.954228, -0.953306, -0.952375,
    -0.951435, -0.950486, -0.949528, -0.948561, -0.947586, -0.946601,
    -0.945607, -0.944605, -0.943593, -0.942573, -0.941544, -0.940506,
    -0.939459, -0.938404, -0.937339, -0.936266, -0.935184, -0.934093,
    -0.932993, -0.931884, -0.930767, -0.929641, -0.928506, -0.927363,
    -0.92621, -0.925049, -0.92388, -0.922701, -0.921514, -0.920318,
    -0.919114, -0.917901, -0.916679, -0.915449, -0.91421, -0.912962,
    -0.911706, -0.910441, -0.909168, -0.907886, -0.906596, -0.905297,
    -0.903989, -0.902673, -0.901349, -0.900016, -0.898674, -0.897325,
    -0.895966, -0.894599, -0.893224, -0.891841, -0.890449, -0.889048,
    -0.88764, -0.886223, -0.884797, -0.883363, -0.881921, -0.880471,
    -0.879012, -0.877545, -0.87607, -0.874587, -0.873095, -0.871595,
    -0.870087, -0.868571, -0.867046, -0.865514, -0.863973, -0.862424,
    -0.860867, -0.859302, -0.857729, -0.856147, -0.854558, -0.852961,
    -0.851355, -0.849742, -0.84812, -0.846491, -0.844854, -0.843208,
    -0.841555, -0.839894, -0.838225, -0.836548, -0.834863, -0.83317,
    -0.83147, -0.829761, -0.828045, -0.826321, -0.824589, -0.82285,
    -0.821103, -0.819348, -0.817585, -0.815814, -0.814036, -0.812251,
    -0.810457, -0.808656, -0.806848, -0.805031, -0.803208, -0.801376,
    -0.799537, -0.797691, -0.795837, -0.793975, -0.792107, -0.79023,
    -0.788346, -0.786455, -0.784557, -0.782651, -0.780737, -0.778817,
    -0.776888, -0.774953, -0.77301, -0.771061, -0.769103, -0.767139,
    -0.765167, -0.763188, -0.761202, -0.759209, -0.757209, -0.755201,
    -0.753187, -0.751165, -0.749136, -0.747101, -0.745058, -0.743008,
    -0.740951, -0.738887, -0.736817, -0.734739, -0.732654, -0.730563,
    -0.728464, -0.726359, -0.724247, -0.722128, -0.720003, -0.71787,
    -0.715731, -0.713585, -0.711432, -0.709273, -0.707107, -0.704934,
    -0.702755, -0.700569, -0.698376, -0.696177, -0.693971, -0.691759,
    -0.689541, -0.687315, -0.685084, -0.682846, -0.680601, -0.67835,
    -0.676093, -0.673829, -0.671559, -0.669283, -0.667, -0.664711,
    -0.662416, -0.660114, -0.657807, -0.655493, -0.653173, -0.650847,
    -0.648514, -0.646176, -0.643832, -0.641481, -0.639124, -0.636762,
    -0.634393, -0.632019, -0.629638, -0.627252, -0.624859, -0.622461,
    -0.620057, -0.617647, -0.615232, -0.61281, -0.610383, -0.60795,
    -0.605511, -0.603067, -0.600616, -0.598161, -0.595699, -0.593232,
    -0.59076, -0.588282, -0.585798, -0.583309, -0.580814, -0.578314,
    -0.575808, -0.573297, -0.570781, -0.568259, -0.565732, -0.563199,
    -0.560662, -0.558119, -0.55557, -0.553017, -0.550458, -0.547894,
    -0.545325, -0.542751, -0.540171, -0.537587, -0.534998, -0.532403,
    -0.529804, -0.527199, -0.52459, -0.521975, -0.519356, -0.516732,
    -0.514103, -0.511469, -0.50883, -0.506187, -0.503538, -0.500885,
    -0.498228, -0.495565, -0.492898, -0.490226, -0.48755, -0.484869,
    -0.482184, -0.479494, -0.476799, -0.4741, -0.471397, -0.468689,
    -0.465976, -0.46326, -0.460539, -0.457813, -0.455084, -0.45235,
    -0.449611, -0.446869, -0.444122, -0.441371, -0.438616, -0.435857,
    -0.433094, -0.430326, -0.427555, -0.42478, -0.422, -0.419217,
    -0.41643, -0.413638, -0.410843, -0.408044, -0.405241, -0.402435,
    -0.399624, -0.39681, -0.393992, -0.39117, -0.388345, -0.385516,
    -0.382683, -0.379847, -0.377007, -0.374164, -0.371317, -0.368467,
    -0.365613, -0.362756, -0.359895, -0.357031, -0.354164, -0.351293,
    -0.348419, -0.345541, -0.342661, -0.339777, -0.33689, -0.334,
    -0.331106, -0.32821, -0.32531, -0.322408, -0.319502, -0.316593,
    -0.313682, -0.310767, -0.30785, -0.304929, -0.302006, -0.29908,
    -0.296151, -0.293219, -0.290285, -0.287347, -0.284408, -0.281465,
    -0.27852, -0.275572, -0.272621, -0.269668, -0.266713, -0.263755,
    -0.260794, -0.257831, -0.254866, -0.251898, -0.248928, -0.245955,
    -0.24298, -0.240003, -0.237024, -0.234042, -0.231058, -0.228072,
    -0.225084, -0.222094, -0.219101, -0.216107, -0.21311, -0.210112,
    -0.207111, -0.204109, -0.201105, -0.198098, -0.19509, -0.19208,
    -0.189069, -0.186055, -0.18304, -0.180023, -0.177004, -0.173984,
    -0.170962, -0.167938, -0.164913, -0.161886, -0.158858, -0.155828,
    -0.152797, -0.149765, -0.14673, -0.143695, -0.140658, -0.13762,
    -0.134581, -0.13154, -0.128498, -0.125455, -0.122411, -0.119365,
    -0.116319, -0.113271, -0.110222, -0.107172, -0.104122, -0.10107,
    -0.0980171, -0.0949635, -0.091909, -0.0888536, -0.0857973, -0.0827403,
    -0.0796824, -0.0766239, -0.0735646, -0.0705046, -0.0674439, -0.0643826,
    -0.0613207, -0.0582583, -0.0551952, -0.0521317, -0.0490677, -0.0460032,
    -0.0429383, -0.0398729, -0.0368072, -0.0337412, -0.0306748, -0.0276081,
    -0.0245412, -0.0214741, -0.0184067, -0.0153392, -0.0122715, -0.00920375,
    -0.00613588, -0.00306796
};

static const float *WAVE_SINES[DEFAULT_OCTAVES] = {
    WAVE_SINE_000, WAVE_SINE_000, WAVE_SINE_000, WAVE_SINE_000,
    WAVE_SINE_000, WAVE_SINE_000, WAVE_SINE_000, WAVE_SINE_000,
    WAVE_SINE_000, WAVE_SINE_000, WAVE_SINE_000, WAVE_SINE_000
};
