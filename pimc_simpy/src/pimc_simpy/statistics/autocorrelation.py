"""
This module contains code for calculating the autocorrelation time for a 1D sequence
of floating-point values calculated from a Markov-chain Monte Carlo simulation.

This code is slightly modified after being taken directly from
    `https://github.com/dfm/emcee/blob/main/src/emcee/autocorr.py`
    (repo has MIT License)
"""

import numpy as np
from numpy.typing import NDArray

# This is the suggested value we use when determining how to truncate the number of terms
# used to calculate the autocorrelation time.
# Suggested by Sokal, taken from here: `https://emcee.readthedocs.io/en/stable/tutorials/autocorr/`
_DEFAULT_SOKAL_CUTOFF: float = 5.0


# Automated windowing procedure following Sokal (1989)
def _autocorrelation_window(autocorr_times: NDArray, sokal_cutoff: float):
    m = np.arange(len(autocorr_times)) < sokal_cutoff * autocorr_times
    if np.any(m):
        return np.argmin(m)
    return len(autocorr_times) - 1


def _next_pow_two(n: int):
    i = 1
    while i < n:
        i = i << 1
    return i


def _single_autocorrelation1d(mcmc_data: NDArray, normalize: bool) -> NDArray:
    n = _next_pow_two(len(mcmc_data))

    # Compute the FFT and then (from that) the auto-correlation function
    f = np.fft.fft(mcmc_data - np.mean(mcmc_data), n=2 * n)
    acf = np.fft.ifft(f * np.conjugate(f))[: len(mcmc_data)].real
    acf /= 4 * n

    # Optionally normalize
    if normalize:
        acf /= acf[0]

    return acf


def _averaged_autocorrelation1d(mcmc_data: NDArray, normalize: bool) -> NDArray:
    averaged_autocorrelation = np.zeros(mcmc_data.shape[1])

    for run in mcmc_data:
        averaged_autocorrelation += _single_autocorrelation1d(run, normalize)

    averaged_autocorrelation /= mcmc_data.shape[0]

    return averaged_autocorrelation


def autocorrelation1d(mcmc_data: NDArray, *, normalize: bool = True) -> NDArray:
    if len(mcmc_data.shape) == 1:
        return _single_autocorrelation1d(mcmc_data, normalize)
    elif len(mcmc_data.shape) == 2:
        return _averaged_autocorrelation1d(mcmc_data, normalize)
    else:
        raise RuntimeError(
            "The autocorrelations can only be calculated for the following cases:\n"
            "  - a 1D numpy array representing the MCMC data for a single run\n"
            "  - a 2D numpy array representing the MCMC data from multiple separate runs\n"
        )


def autocorrelation_time_from_autocorrelations(
    autocorrelations: NDArray, *, sokal_cutoff: float = _DEFAULT_SOKAL_CUTOFF
) -> float:
    autocorrelation_times = 2.0 * np.cumsum(autocorrelations) - 1.0
    ideal_window = _autocorrelation_window(autocorrelation_times, sokal_cutoff)
    return autocorrelation_times[ideal_window]


def autocorrelation_time_from_data(mcmc_data: NDArray, *, sokal_cutoff: float = _DEFAULT_SOKAL_CUTOFF) -> float:
    autocorrelations = autocorrelation1d(mcmc_data, normalize=True)
    return autocorrelation_time_from_autocorrelations(autocorrelations, sokal_cutoff=sokal_cutoff)
