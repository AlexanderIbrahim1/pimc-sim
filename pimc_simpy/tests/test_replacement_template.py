from pathlib import Path

import pytest

from pimc_simpy.manage.replacement_template import ReplacementTemplate


class TestReplacementTemplate:
    def test_basic(self) -> None:
        contents = "value0 = [[first_word]]\nvalue1 = world"
        template = ReplacementTemplate(contents)

        actual = template.replace("[[first_word]]", "hello")
        expected = "value0 = hello\nvalue1 = world"

        assert actual == expected

    def test_multiple_replacement(self) -> None:
        contents = "value0 = [[first_word]]\nvalue1 = [[second_word]]\nvalue2 = 123"
        template = ReplacementTemplate(contents)

        replacement_dict = {"[[first_word]]": "hello", "[[second_word]]": "world"}

        actual = template.replace(replacement_dict)
        expected = "value0 = hello\nvalue1 = world\nvalue2 = 123"

        assert actual == expected

    def test_path_surrounded_by_double_quotes(self) -> None:
        contents = "value0 = [[first_word]]\nvalue1 = world"
        template = ReplacementTemplate(contents)

        replacement = Path("this", "is", "a", "path")
        replacement_as_string = str(replacement)

        actual = template.replace("[[first_word]]", replacement)
        expected = f'value0 = "{replacement_as_string}"\nvalue1 = world'

        assert actual == expected
