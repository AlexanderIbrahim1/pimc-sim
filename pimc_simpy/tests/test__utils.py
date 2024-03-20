from io import StringIO

from pimc_simpy._utils import skip_lines_that_start_with


class Test_skip_lines_that_start_with:
    def test_basic(self) -> None:
        stream = StringIO(
            "\n".join(
                [
                    "# this starts with a hashtag            ",
                    "# also a hashtag                        ",
                    "# and another                           ",
                    "This line does not start with a hashtag.",
                ]
            )
        )
        skip_lines_that_start_with(stream, "#")
        assert stream.readline().strip() == "This line does not start with a hashtag."
