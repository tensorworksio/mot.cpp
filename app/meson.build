app_src = files(
    'main.cpp'
)

# Copy config folder
run_command('cp', '-r', 'config', meson.current_build_dir(), check: true)

# Create a symbolic link to the data folder
run_command('rm', '-f', join_paths(meson.current_build_dir(), 'data'), check: true)
run_command('ln', '-s', join_paths(meson.source_root(), 'data'), join_paths(meson.current_build_dir(), 'data'), check: true)

# Excutable
executable('mot',
    sources: app_src,
    include_directories: inc_dir,
    dependencies: dependencies,
    link_with: mot_lib,
    install: true
)