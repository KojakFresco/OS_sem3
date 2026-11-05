#!/usr/bin/env python3

import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt


def read_results(path):
    with path.open(newline="", encoding="utf-8") as file:
        rows = list(csv.DictReader(file))

    for row in rows:
        row["size"] = int(row["size"])
        row["threads"] = int(row["threads"])
        row["average_ms"] = float(row["average_ms"])
        row["speedup"] = float(row["speedup"])
        row["efficiency"] = float(row["efficiency"])
    return rows


def plot_metric(rows, metric, title, ylabel, output_path):
    patterns = sorted({row["pattern"] for row in rows})
    sizes = sorted({row["size"] for row in rows})
    figure, axes = plt.subplots(
        len(patterns),
        len(sizes),
        figsize=(16, 10),
        squeeze=False,
        sharex=True,
    )

    for row_index, pattern in enumerate(patterns):
        for column_index, size in enumerate(sizes):
            points = [
                row for row in rows
                if row["pattern"] == pattern and row["size"] == size
            ]
            points.sort(key=lambda row: row["threads"])
            axes[row_index][column_index].plot(
                [row["threads"] for row in points],
                [row[metric] for row in points],
                marker="o",
                markersize=3,
            )
            axes[row_index][column_index].set_title(f"{pattern}, n={size}")
            axes[row_index][column_index].set_xlabel("Threads")
            axes[row_index][column_index].set_ylabel(ylabel)
            axes[row_index][column_index].grid(True, alpha=0.3)

    figure.suptitle(title)
    figure.tight_layout()
    figure.savefig(output_path, dpi=150)
    plt.close(figure)


def main():
    parser = argparse.ArgumentParser(description="Plot parallel merge sort benchmarks")
    parser.add_argument(
        "input",
        nargs="?",
        type=Path,
        default=Path("benchmark_res.csv"),
        help="CSV file produced by benchmark",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("benchmark_plots"),
        help="Directory for generated PNG files",
    )
    args = parser.parse_args()

    rows = read_results(args.input)
    if not rows:
        raise ValueError("Benchmark CSV is empty")

    args.output_dir.mkdir(parents=True, exist_ok=True)
    plot_metric(
        rows,
        "average_ms",
        "Parallel merge sort: execution time",
        "Time, ms",
        args.output_dir / "time.png",
    )
    plot_metric(
        rows,
        "speedup",
        "Parallel merge sort: speedup",
        "Speedup",
        args.output_dir / "speedup.png",
    )
    plot_metric(
        rows,
        "efficiency",
        "Parallel merge sort: efficiency",
        "Efficiency",
        args.output_dir / "efficiency.png",
    )


if __name__ == "__main__":
    main()
