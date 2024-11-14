project('mot.cpp', 'cpp')

# Include directory
inc_dir = include_directories('include')

# Dependencies
spdlog_dep = dependency('spdlog')
json_dep = dependency('nlohmann_json')
dlib_dep = dependency('dlib-1', method : 'pkg-config')
boost_dep = dependency('boost', modules : ['filesystem', 'program_options', 'json'])
opencv_dep = dependency('opencv4', modules : ['core', 'highgui', 'imgproc', 'imgcodecs', 'video', 'videoio'])

# Source files
kalman_files = run_command('find', 'src/kalman', '-name', '*.cpp', check: true).stdout().strip().split('\n')


src_files = files(
  'src/sort.cpp',
  'src/botsort.cpp'
) + kalman_files

# Build shared library
library('mot', src_files,
  dependencies : [spdlog_dep, json_dep, dlib_dep, boost_dep, opencv_dep],
  include_directories : inc_dir
)

# Main executable
executable('mot', [src_files, 'app/mot.cpp'],
  dependencies : [spdlog_dep, json_dep, dlib_dep, boost_dep, opencv_dep],
  include_directories : inc_dir,
  install : true
)