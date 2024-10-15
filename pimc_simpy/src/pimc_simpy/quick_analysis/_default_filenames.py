"""
This module holds the default filenames for the output files from a simulation.
"""

# --- move acceptance rates (multiple, [int, int])
_BISECTION_MULTIBEAD_POSITION_MOVE_ACCEPT_FILENAME: str = "bisection_multibead_position_move_accept.dat"
_SINGLE_BEAD_POSITION_MOVE_ACCEPT_FILENAME: str = "single_bead_position_move_accept.dat"
_CENTRE_OF_MASS_POSITION_MOVE_ACCEPT_FILENAME: str = "centre_of_mass_position_move_accept.dat"

# --- move step size information (multiple, [float, int])
_BISECTION_MULTIBEAD_POSITION_MOVE_INFO_FILENAME: str = "bisection_multibead_position_move_info.dat"

# --- histograms (radial_distribution_function.py)
_CENTROID_RADIAL_DIST_HISTO_FILENAME: str = "centroid_radial_dist_histo.dat"
_RADIAL_DIST_HISTO_FILENAME: str = "radial_dist_histo.dat"

# --- single value estimates (single, [float])
_CENTRE_OF_MASS_STEP_SIZE_FILENAME: str = "centre_of_mass_step_size.dat"
_ABSOLUTE_CENTROID_DISTANCE_FILENAME: str = "absolute_centroid_distance.dat"
_RMS_CENTROID_DISTANCE_FILENAME: str = "rms_centroid_distance.dat"
_KINETIC_ENERGY_FILENAME: str = "kinetic.dat"
_PAIR_POTENTIAL_ENERGY_FILENAME: str = "pair_potential.dat"
_QUADRUPLET_POTENTIAL_ENERGY_FILENAME: str = "quadruplet_potential.dat"
_TRIPLET_POTENTIAL_ENERGY_FILENAME: str = "triplet_potential.dat"

# --- misc
_BOX_SIDES_FILENAME: str = "box_sides.dat"  # (unique)
_TIMER_FILENAME: str = "timer.dat"  # (multiple, [int, int, int])
