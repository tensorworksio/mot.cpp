# Multi Object Tracking
Yet another C++ adaptation of SOTA multi-object tracking algorithm
<p align="center">
    <img src="results/MOT20-01.gif" alt="Demo">
</p>

## Supported Trackers
- [SORT](https://github.com/abewley/sort) ![Support](https://img.shields.io/badge/support-yes-brightgreen.svg)
- [BoTSORT](https://github.com/NirAharon/BoT-SORT) ![Support](https://img.shields.io/badge/support-yes-brightgreen.svg)

## Dataset
Get the MOT dataset of your choice from [MOT website](https://motchallenge.net/)
```bash
cd data
wget https://motchallenge.net/data/MOT20.zip
unzip MOT20.zip && rm MOT20.zip
```

## Configure
In `config` folder, add your custom tracker config

## Compile

```shell
# Ensure to reset the deps
meson subprojects update --reset
meson setup build --wipe
```

```shell
meson compile -C build
meson test -C build
```

## Run
```shell
cd build/app
./mot -h

# Example
./mot -i data/MOT20/train/<seq-name> -c config/sort.toml --display
```

## Evaluate
```shell
chmod +x mot-eval.sh
./mot-eval.sh --dataset data/MOT20 --split train --config app/config/sort.toml --save
# experiment output available in runs folder
```

## Run with your detector
https://github.com/tensorworksio/TensorRT-Vision/tree/main/app/mot