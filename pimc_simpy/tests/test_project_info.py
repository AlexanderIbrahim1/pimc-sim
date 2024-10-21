from io import BytesIO
from pathlib import Path

import pytest

from pimc_simpy.manage.project_info import ProjectInfo
from pimc_simpy.manage.project_info import parse_project_info_stream


@pytest.fixture
def toml_contents_with_rel_paths() -> str:
    return "\n".join(
        [
            'abs_home = "/home/a68ibrah/projects/def-pnroy/a68ibrah"',
            'rel_external_dirpath = "pimc_simulations/pimc-sim"',
            'rel_executable_dirpath = "pimc_simulations/pimc-sim/build/highperf/source"',
            'rel_subproject_dirpath = "pimc_simulations/simulations/twothreefour_body/mcmc_param_search/p64_coarse"',
            'subproject_name = "p64_coarse"',
        ]
    )


@pytest.fixture
def toml_contents_with_abs_paths() -> str:
    return "\n".join(
        [
            'abs_external_dirpath = "/home/a68ibrah/projects/def-pnroy/a68ibrah/pimc_simulations/pimc-sim"',
            'abs_executable_dirpath = "/home/a68ibrah/projects/def-pnroy/a68ibrah/pimc_simulations/pimc-sim/build/highperf/source"',
            'abs_subproject_dirpath = "/home/a68ibrah/projects/def-pnroy/a68ibrah/pimc_simulations/simulations/twothreefour_body/mcmc_param_search/p64_coarse"',
            'subproject_name = "p64_coarse"',
        ]
    )


class Test_parse_project_info:
    def test_with_both_paths(self, toml_contents_with_abs_paths: str, toml_contents_with_rel_paths: str) -> None:
        toml_stream_rel_paths = BytesIO(toml_contents_with_rel_paths.encode("utf8"))
        toml_stream_abs_paths = BytesIO(toml_contents_with_abs_paths.encode("utf8"))

        info_from_rel_paths = parse_project_info_stream(toml_stream_rel_paths)
        info_from_abs_paths = parse_project_info_stream(toml_stream_abs_paths)

        # fmt: off
        abs_home = Path("/home/a68ibrah/projects/def-pnroy/a68ibrah")
        expected_abs_external_dirpath = abs_home / "pimc_simulations" / "pimc-sim"
        expected_abs_executable_filepath = abs_home / "pimc_simulations" / "pimc-sim" / "build" / "highperf" / "source"
        expected_abs_subproject_dirpath = abs_home / "pimc_simulations" / "simulations" / "twothreefour_body" / "mcmc_param_search" / "p64_coarse"
        expected_subproject_name = "p64_coarse"
        # fmt: on

        assert info_from_rel_paths.abs_external_dirpath == expected_abs_external_dirpath
        assert info_from_rel_paths.abs_executable_dirpath == expected_abs_executable_filepath
        assert info_from_rel_paths.abs_subproject_dirpath == expected_abs_subproject_dirpath
        assert info_from_rel_paths.subproject_name == expected_subproject_name
        assert info_from_abs_paths.abs_external_dirpath == expected_abs_external_dirpath
        assert info_from_abs_paths.abs_executable_dirpath == expected_abs_executable_filepath
        assert info_from_abs_paths.abs_subproject_dirpath == expected_abs_subproject_dirpath
        assert info_from_abs_paths.subproject_name == expected_subproject_name
