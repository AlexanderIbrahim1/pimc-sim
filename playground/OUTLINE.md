# Program Outline

The overall structure of the program:
- collect all the conditions for the simulation
  - number of beads, number of particles, PIMC hyperparameters, lattice type, density, etc.
- create the periodic lattice box
- create all the particle positions
  - put them all in worldlines
- run MC moves to change the state of the system
- run estimators when certain conditions are met


## Pseudocode
Here is the outline for a program that runs the simulation from scratch (no existing state):

def simulation_from_scratch():
    argparser = ArgParser()
    args = argparser.parse_args()

    n_particles = get_number_of_particles(args.unit_cell_boxes, args.lattice_type)
    box = make_periodic_box(args.unit_cell_boxes, args.lattice_type, args.density)

    worldlines = create_worldlines(args.unit_cell_boxes, args.lattice_type, args.density)

    potential_energy_output_file = Path(...)
    kinetic_energy_output_file = Path(...)
    position_output_file = Path(...)

    potential_energy_estimation_condition = []() -> bool {...}
    kinetic_energy_estimation_condition = []() -> bool {...}
    position_estimation_condition = []() -> bool {...}

    for i_block in range(0, args.n_blocks_warmup):
        for i_pass in range(0, args.n_passes_warmup):
            monte_carlo_move1(worldlines, box)
            monte_carlo_move2(worldlines, box)

    for i_block in range(0, args.n_blocks_estimation):
        for i_pass in range(0, args.n_passes_estimation):
            monte_carlo_move1(worldlines, box)
            monte_carlo_move2(worldlines, box)

            if potential_energy_estimation_condition(i_block, i_pass):
                potential_energy = estimate_potential_energy(worldlines, box)
                save_potential_energy(potential_energy_output_file, potential_energy, i_block, i_pass)

            if kinetic_energy_estimation_condition(i_block, i_pass):
                kinetic_energy = estimate_kinetic_energy(worldlines, box)
                save_kinetic_energy(kinetic_energy_output_file, kinetic_energy, i_block, i_pass)

            if position_estimation_condition(i_block, i_pass):
                position = estimate_position(worldlines, box)
                save_position(position_output_file, position, i_block, i_pass)
