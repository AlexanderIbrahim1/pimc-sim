"""
This module contains functions that make it easy to plot curves of PropertyData instances.
"""

from __future__ import annotations

from collections.abc import Sequence
from typing import Optional
from typing import Union

import matplotlib.pyplot as plt

from pimc_simpy.data.property_data import PropertyData
from pimc_simpy.statistics import calculate_cumulative_means_and_sems


def plot_property_cumulative(
    properties: Union[PropertyData, Sequence[PropertyData]], *, labels: Optional[Sequence[str]] = None
) -> None:
    if isinstance(properties, PropertyData):
        if labels is not None:
            labels = labels[0]

        _plot_single_property_cumulative(properties, label=labels)
        return

    if len(properties) == 0:
        raise ValueError("There must be at least one type of data to plot.")

    if labels is not None and len(labels) != len(properties):
        raise ValueError("The number of labels must match the number of types of data to plot.")

    fig = plt.figure()
    ax = fig.add_subplot()

    x_lim_min = properties[0].epochs[0]
    x_lim_max = properties[0].epochs[-1]
    ax.set_xlim(x_lim_min, x_lim_max)

    for i, data in enumerate(properties):
        cumulative_means, cumulative_sems = calculate_cumulative_means_and_sems(data)

        if labels is not None:
            label = labels[i]
            ax.plot(data.epochs, cumulative_means, label=label)
        else:
            ax.plot(data.epochs, cumulative_means)

        ax.fill_between(data.epochs, cumulative_means + cumulative_sems, cumulative_means - cumulative_sems, alpha=0.5)

        ax.legend()

    plt.show()


def _plot_single_property_cumulative(data: PropertyData, *, label: Optional[str] = None) -> None:
    cumulative_means, cumulative_sems = calculate_cumulative_means_and_sems(data)

    fig = plt.figure()
    ax = fig.add_subplot()

    ax.set_xlim(data.epochs[0], data.epochs[-1])

    if label is not None:
        ax.plot(data.epochs, cumulative_means, label=label)
    else:
        ax.plot(data.epochs, cumulative_means)

    ax.fill_between(data.epochs, cumulative_means + cumulative_sems, cumulative_means - cumulative_sems, alpha=0.5)

    plt.show()
