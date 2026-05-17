gtest_dep = dependency('gtest', required: true)
gtest_main_dep = dependency('gtest_main', required: true)

test_sources = [
    'test_kalman.cpp',
    'test_sort.cpp',
    'test_botsort.cpp',
    'test_hungarian.cpp',
]

test_exe = executable('mot_tests',
    test_sources,
    dependencies: [mot_dep, gtest_dep, gtest_main_dep],
)

test('mot_tests', test_exe)
