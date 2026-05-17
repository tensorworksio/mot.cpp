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

<details open>
    <summary>SORT</summary>

```toml
tracker = "sort"
max_time_lost = 15
match_thresh = 0.3

[kalman]
time_step = 1
process_noise_scale = 1.0
measurement_noise_scale = 1.0
```
</details>
<details>
    <summary>BoTSORT</summary>

```toml
tracker = "botsort"
max_time_lost = 15
track_high_thresh = 0.5
track_low_thresh = 0.1
new_track_thresh = 0.6
first_match_thresh = 0.3
second_match_thresh = 0.1
unconfirmed_match_thresh = 0.2
proximity_thresh = 0.5
appearance_thresh = 0.9

[kalman]
time_step = 1
process_noise_scale = 1.0
measurement_noise_scale = 1.0
```
</details>

## 🖥️ Local

### Compile

**Requirements:** GCC 14+, CMake, Meson, Ninja, pkg-config, OpenCV
```shell
# Ensure to reset the deps
meson subprojects update --reset
meson setup build --wipe
```

```shell
meson compile -C build
meson test -C build
```

### Run
```shell
cd build/app
./mot -h

# Example
./mot -i data/MOT20/train/<seq-name> -c config/sort.toml --display
```

### Evaluate
**Requirements:** Python 3.12+, numpy<2.0.0, motmetrics

First, set up your Python environment and dependencies:
```shell
python3 -m venv venv
./venv/bin/pip install -r requirements.txt
```

Then run the evaluation script:
```shell
chmod +x mot-eval.sh
./mot-eval.sh --dataset data/MOT20 --split train --config app/config/sort.toml --save
# experiment output available in runs folder
```

---

## 🐳 Docker

Build the image:
```shell
docker build -t mot.cpp .
```

Run the tracker on a dataset sequence mounted from the host:
```shell
# Allow Docker to connect to the X11 display server
xhost +local:docker

docker run --rm \
    --user $(id -u):$(id -g) \
    --env DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix:ro \
    -v $(pwd)/data:/data \
    -v $(pwd)/app/config:/opt/mot.cpp/app/config \
    mot.cpp -i /data/MOT20/train/MOT20-01 -c /opt/mot.cpp/app/config/sort.toml --display
```

Run evaluation across a full dataset split:
```shell
docker run --rm \
    --user $(id -u):$(id -g) \
    -v $(pwd)/data:/data \
    -v $(pwd)/runs:/opt/mot.cpp/runs \
    --entrypoint /opt/mot.cpp/mot-eval.sh \
    mot.cpp --dataset /data/MOT20 --split train --config /opt/mot.cpp/app/config/sort.toml --save
```

## Run with your detector
https://github.com/tensorworksio/TensorRT-Vision/tree/main/app/mot