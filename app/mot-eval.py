# https://github.com/cheind/py-motmetrics

from argparse import ArgumentParser
from pathlib import Path

import motmetrics as mm
import numpy as np


def parse_arguments():
    parser = ArgumentParser()

    parser.add_argument(
        "--gt",
        dest="gt",
        required=True,
        help="Path to ground truth file (gt.txt)"
    )
    parser.add_argument(
        "--input", "-i",
        dest="input",
        required=True,
        help="Path to tracking output file"
    )

    args = parser.parse_args()

    # Validate files exist
    if not Path(args.gt).is_file():
        raise FileNotFoundError(f"Ground truth file not found: {args.gt}")
    if not Path(args.input).is_file():
        raise FileNotFoundError(f"Input file not found: {args.input}")

    return args


def motMetricsEnhancedCalculator(gtSource, outSource):
    # load ground truth
    gt = np.loadtxt(gtSource, delimiter=",")

    # load tracking output
    out = np.loadtxt(outSource, delimiter=",")

    # Create an accumulator that will be updated during each frame
    acc = mm.MOTAccumulator(auto_id=True)

    # Max frame number maybe different for gt and t files
    for frame in range(int(gt[:, 0].max())):
        frame += 1  # detection and frame numbers begin at 1

        # select id, x, y, width, height for current frame
        # required format for distance calculation is X, Y, Width, Height \
        # We already have this format
        gt_dets = gt[gt[:, 0] == frame, 1:6]  # select all detections in gt
        out_dets = out[out[:, 0] == frame, 1:6]  # select all detections in out

        C = mm.distances.iou_matrix(
            gt_dets[:, 1:], out_dets[:, 1:], max_iou=0.5
        )  # format: gt, t

        # Call update once for per frame.
        # format: gt object ids, t object ids, distance
        acc.update(
            gt_dets[:, 0].astype("int").tolist(),
            out_dets[:, 0].astype("int").tolist(),
            C,
        )

    mh = mm.metrics.create()

    summary = mh.compute(
        acc,
        metrics=[
            "num_frames",
            "idf1",
            "idp",
            "idr",
            "recall",
            "precision",
            "num_objects",
            "mostly_tracked",
            "partially_tracked",
            "mostly_lost",
            "num_false_positives",
            "num_misses",
            "num_switches",
            "num_fragmentations",
            "mota",
            "motp",
        ],
        name="acc",
    )

    strsummary = mm.io.render_summary(
        summary,
        # formatters={'mota' : '{:.2%}'.format},
        namemap={
            "idf1": "IDF1",
            "idp": "IDP",
            "idr": "IDR",
            "recall": "Rcll",
            "precision": "Prcn",
            "num_objects": "GT",
            "mostly_tracked": "MT",
            "partially_tracked": "PT",
            "mostly_lost": "ML",
            "num_false_positives": "FP",
            "num_misses": "FN",
            "num_switches": "IDsw",
            "num_fragmentations": "FM",
            "mota": "MOTA",
            "motp": "MOTP",
        },
    )
    print(strsummary)


if __name__ == "__main__":
    args = parse_arguments()
    motMetricsEnhancedCalculator(args.gt, args.input)
