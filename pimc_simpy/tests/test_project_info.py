from io import BytesIO
from pathlib import Path

import pytest

from pimc_simpy.manage.project_info import ProjectInfo
from pimc_simpy.manage.project_info import parse_project_info


@pytest.fixture
def toml_contents_with_abs_home() -> str:
    return "\n".join(
        [
            'abs_home = "/home/a68ibrah/projects/def-pnroy/a68ibrah"',
            'rel_external_dirpath = "pimc_simulations/pimc-sim"',
            'rel_executable_filepath = "pimc_simulations/pimc-sim/build/highperf/source/pimc-sim"',
            'rel_subproject_dirpath = "pimc_simulations/simulations/twothreefour_body/mcmc_param_search/p64_coarse"',
            'subproject_name = "p64_coarse"',
        ]
    )


class Test_parse_project_info:
    def test_basic(self, toml_contents_with_abs_home: str) -> None:
        toml_stream = BytesIO(toml_contents_with_abs_home.encode("utf8"))

        info = parse_project_info(toml_stream)

        # fmt: off
        abs_home = Path("/home/a68ibrah/projects/def-pnroy/a68ibrah")
        expected_abs_external_dirpath = abs_home / "pimc_simulations" / "pimc-sim"
        expected_abs_executable_filepath = abs_home / "pimc_simulations" / "pimc-sim" / "build" / "highperf" / "source" / "pimc-sim"
        expected_abs_subproject_dirpath = abs_home / "pimc_simulations" / "simulations" / "twothreefour_body" / "mcmc_param_search" / "p64_coarse"
        expected_subproject_name = "p64_coarse"
        # fmt: on

        assert info.abs_external_dirpath == expected_abs_external_dirpath
        assert info.abs_executable_filepath == expected_abs_executable_filepath
        assert info.abs_subproject_dirpath == expected_abs_subproject_dirpath
        assert info.subproject_name == expected_subproject_name
