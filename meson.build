project('mot.cpp', 'cpp',
        version : '0.2',
        default_options : [
          'cpp_std=c++17',
          'warning_level=3',
          'werror=true'
        ]
)

# Include directory
inc_dir = include_directories('include')

# Dependencies
vision_core_proj = subproject('vision-core')
vision_core_dep = vision_core_proj.get_variable('vision_core_dep')

# 3rd party
spdlog_dep = dependency('spdlog')
json_dep = dependency('nlohmann_json')
dlib_dep = dependency('dlib-1', method : 'pkg-config')
boost_dep = dependency('boost', modules : ['filesystem', 'program_options', 'json'])
opencv_dep = dependency('opencv4', modules : ['core', 'highgui', 'imgproc', 'imgcodecs', 'video', 'videoio'])

dependencies = [vision_core_dep, spdlog_dep, json_dep, dlib_dep, boost_dep, opencv_dep]

# Source files
src_files = files(
  'src/kalman/xywh.cpp',
  'src/kalman/xysr.cpp',

  'src/tracking/tracker.cpp',
  'src/tracking/sort.cpp',
  'src/tracking/botsort.cpp'
)

# Build shared library
mot_lib =library(
  'mot', 
  sources: src_files,
  include_directories: inc_dir,
  dependencies : dependencies
)

# Export Dependencies
mot_dep = declare_dependency(
  link_with: mot_lib,
  include_directories: inc_dir
)

# App
subdir('app')