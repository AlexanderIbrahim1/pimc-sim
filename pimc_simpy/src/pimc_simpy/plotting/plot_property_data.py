"""
This module contains functions that make it easy to plot curves of PropertyData instances.
"""

from __future__ import annotations

from collections.abc import Sequence
from typing import Optional
from typing import Union

import matplotlib.pyplot as plt
import numpy as np

from pimc_simpy.data import PropertyData
from pimc_simpy.statistics import calculate_cumulative_means_and_sems


_PROPERTY_RESCALED_PLOT_Y_LIM_PADDING_FRAC: float = 0.05


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


def plot_property(properties: Union[PropertyData, Sequence[PropertyData]], *, labels: Optional[Sequence[str]] = None) -> None:
    if isinstance(properties, PropertyData):
        if labels is not None:
            labels = labels[0]

        _plot_single_property(properties, label=labels)
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
        if labels is not None:
            label = labels[i]
            ax.plot(data.epochs, data.values, label=label)
        else:
            ax.plot(data.epochs, data.values)

        ax.legend()

    plt.show()


def plot_property_rescaled(
    properties: Union[PropertyData, Sequence[PropertyData]],
    *,
    lower: float = 0.0,
    upper: float = 1.0,
    labels: Optional[Sequence[str]] = None,
) -> None:
    if isinstance(properties, PropertyData):
        if labels is not None:
            labels = labels[0]

        _plot_single_property(properties, label=labels)
        return

    if len(properties) == 0:
        raise ValueError("There must be at least one type of data to plot.")

    if labels is not None and len(labels) != len(properties):
        raise ValueError("The number of labels must match the number of types of data to plot.")

    if lower >= upper:
        raise ValueError("The rescaling requires that 'lower < upper'.")

    fig = plt.figure()
    ax = fig.add_subplot()

    x_lim_min = properties[0].epochs[0]
    x_lim_max = properties[0].epochs[-1]

    padding = (upper - lower) * _PROPERTY_RESCALED_PLOT_Y_LIM_PADDING_FRAC
    y_lim_min = lower - padding
    y_lim_max = upper + padding
    ax.set_xlim(x_lim_min, x_lim_max)
    ax.set_ylim(y_lim_min, y_lim_max)

    new_properties = [_rescale_property_data(data) for data in properties]
    print(new_properties)

    for i, data in enumerate(new_properties):
        if labels is not None:
            label = labels[i]
            ax.plot(data.epochs, data.values, label=label)
        else:
            ax.plot(data.epochs, data.values)

        ax.legend()

    plt.show()


def _rescale_property_data(data: PropertyData) -> PropertyData:
    minimum = np.min(data.values).astype(np.float64)
    maximum = np.max(data.values).astype(np.float64)

    print(minimum, maximum)

    if np.isclose(minimum, maximum):
        new_values = np.full(data.values.size, 0.5)
        return PropertyData(data.epochs, new_values)

    new_values = data.values
    new_values -= minimum
    new_values /= maximum - minimum

    return PropertyData(data.epochs, new_values)


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


def _plot_single_property(data: PropertyData, *, label: Optional[str] = None) -> None:
    fig = plt.figure()
    ax = fig.add_subplot()

    ax.set_xlim(data.epochs[0], data.epochs[-1])

    if label is not None:
        ax.plot(data.epochs, data.values, label=label)
    else:
        ax.plot(data.epochs, data.values)

    plt.show()
